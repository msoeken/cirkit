/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "rcbdd_synthesis.hpp"
#include "synthesis_utils_p.hpp"

#include <fstream>

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/pattern_to_circuit.hpp>
#include <reversible/io/print_circuit.hpp>
#include <classical/optimization/optimization.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#include <cuddInt.h>

namespace cirkit
{

enum Direction {
  ChangeLeft = 0,
  ChangeRight
};

int Cudd_bddPickOneCubeForRCBDD( DdManager * ddm, DdNode * node, char * repr)
{
  DdNode *N, *T, *E;
  DdNode *one, *bzero;
  char   dir;
  int    i;

  if ( repr == nullptr || node == nullptr ) return 0;

  /* The constant 0 function has no on-set cubes. */
  one = DD_ONE(ddm);
  bzero = Cudd_Not(one);
  if (node == bzero) return 0;

  for ( i = 0; i < ddm->size; i++ ) repr[i] = 2;

  for (;;)
  {
    if ( node == one ) break;

    N = Cudd_Regular(node);

    T = cuddT(N); E = cuddE(N);
    if ( Cudd_IsComplement( node ) )
    {
	    T = Cudd_Not(T); E = Cudd_Not(E);
    }
    if ( T == bzero )
    {
	    repr[N->index] = 0;
	    node = E;
    }
    else if ( E == bzero )
    {
	    repr[N->index] = 1;
	    node = T;
    }
    else
    {
      if ( N->index % 3 == 1 )
      {
        repr[N->index] = repr[N->index - 1];
        node = repr[N->index] ? T : E;
      }
      else
      {
        dir = (char) ((Cudd_Random( ddm ) & 0x2000) >> 13);
        repr[N->index] = dir;
        node = dir ? T : E;
      }
    }
  }
  return 1;
}

int pick_one_cube_for_rcbdd( BDD node, char * repr )
{
  return Cudd_bddPickOneCubeForRCBDD( node.manager(), node.getNode(), repr );
}

struct rcbdd_synthesis_manager
{
  rcbdd_synthesis_manager( const rcbdd& _cf, circuit& _circ )
    : cf( _cf ),
      circ( _circ ),
      insert_position( 0u )
  {
    f = _cf.chi();

    circ.set_lines( _cf.num_vars() );

    std::vector<std::string> inputs( _cf.num_vars(), _cf.constant_value() ? "1" : "0" );
    boost::copy( cf.input_labels(), inputs.end() - _cf.num_inputs() );
    circ.set_inputs( inputs );

    std::vector<std::string> outputs( _cf.num_vars(), "-" );
    boost::copy( cf.output_labels(), outputs.begin() );
    circ.set_outputs( outputs );

    std::vector<constant> constants( _cf.num_vars(), constant() );
    std::fill( constants.begin(), constants.end() - _cf.num_inputs(), _cf.constant_value() );
    circ.set_constants( constants );

    std::vector<bool> garbage( _cf.num_vars(), true );
    std::fill( garbage.begin(), garbage.begin() + _cf.num_outputs(), false );
    circ.set_garbage( garbage );

    node_count += f.nodeCount();
  }

  void set_var(unsigned v)
  {
    _var = v;

    left_f = cf.manager().bddZero();
    right_f = cf.manager().bddZero();
  }

  void compute_cofactors()
  {
    n  = cf.cofactor(f, _var, false, false);
    pp = cf.cofactor(f, _var, true,  false);
    np = cf.cofactor(f, _var, false, true);
    p  = cf.cofactor(f, _var, true,  true);

    nx  = cf.remove_ys(n);
    ppx = cf.remove_ys(pp);
    npx = cf.remove_ys(np);
    px  = cf.remove_ys(p);

    ny  = cf.remove_xs(n);
    ppy = cf.remove_xs(pp);
    npy = cf.remove_xs(np);
    py  = cf.remove_xs(p);
  }

  void apply_gates(const BDD& lf, const BDD& rf)
  {
    left_f ^= lf;
    right_f ^= rf;

    BDD gate_left = cf.create_from_gate(_var, lf);
    BDD gate_right = cf.create_from_gate(_var, cf.move_ys_to_xs(rf));
    f = cf.compose(cf.compose(gate_left, f), gate_right);
    node_count += f.nodeCount();
  }

  void only_left_gate_shortcut()
  {
    BDD chi_prime = f;

    // Copy y_var to x_var
    chi_prime = chi_prime.ExistAbstract(cf.x(_var)) & cf.x(_var).Xnor(cf.y(_var));

    // Check whether chi' is reversible
    if (mpz_class(chi_prime.CountMinterm(2u * cf.num_vars())) == pow2(cf.num_vars())
        && cf.remove_ys(chi_prime) == cf.manager().bddOne()
        && cf.remove_xs(chi_prime) == cf.manager().bddOne())
    {
      BDD lf = cf.remove_ys(!f & chi_prime).ExistAbstract(cf.x(_var));
      apply_gates(lf, cf.manager().bddZero());
    }
  }

  void resolve_one_cycles()
  {
    // Via F_n
    compute_cofactors();

    //BDD lf = (!nx & ppx) ^ (!px & npx);
    BDD lf = ppx & npx;

    apply_gates(lf, cf.manager().bddZero());

    compute_cofactors();

    BDD rf = ppy & npy;

    apply_gates(cf.manager().bddZero(), rf);

    // // Via F_p
    // compute_cofactors();

    // lf = !px & npx;
    // rf = !py & ppy;

    // apply_gates(lf, rf);
  }

  void resolve_two_cycles()
  {
    // Via F_p
    compute_cofactors();

    BDD _f = ppy & p & npx;
    BDD lf = cf.remove_ys(_f);
    BDD rf = cf.remove_xs(_f);

    apply_gates(lf, rf);

    // Via F_n
    compute_cofactors();

    _f = npy & n & ppx;
    lf = cf.remove_ys(_f);
    rf = cf.remove_xs(_f);

    apply_gates(lf, rf);
  }


  void cycle_step()
  {
    compute_cofactors();

    std::vector<BDD> variables;
    boost::push_back(variables, cf.xs());
    boost::push_back(variables, cf.ys());

    // Pick an arbitrary cube from pp
    BDD cube;

    if ( pp != cf.manager().bddZero() )
    {
      cube = pp.PickOneMinterm(variables);
    }
    else
    {
      cube = ( !cf.remove_xs( f ) & !cf.remove_ys( f ) & cf.x( _var) & !cf.y( _var ) ).PickOneMinterm( variables );
      f |= cube;
    }
    char change = ChangeLeft;

    BDD lf = cf.manager().bddZero();
    BDD rf = cf.manager().bddZero();
    BDD cube_part;
    char *scube = new char[3u * cf.num_vars()];

    ++access;

    do
    {
      if (change == ChangeLeft)
      {
        // Update left gate
        cube_part = cf.remove_ys(cube).ExistAbstract(cf.x(_var));
        lf |= cube_part;
        //change = CHANGE_RIGHT;
        cube = !cf.x(_var) & cube_part & f;
      }
      else
      {
        cube_part = cf.remove_xs(cube).ExistAbstract(cf.y(_var));
        rf |= cube_part;
        //change = CHANGE_LEFT;
        cube = cf.y(_var) & cube_part & f;
      }

      /* Cube is not part of the function? */
      if (cube == cf.manager().bddZero())
      {
        BDD unused_outputs = !cf.remove_xs(f);
        BDD unused_inputs = !cf.remove_ys(f);

        BDD icube, ocube;

        if (change == ChangeLeft)
        {
          icube = !cf.x(_var) & cube_part;

          ocube = cf.manager().bddOne();
          if ((unused_outputs & cf.y(_var)) != cf.manager().bddZero())
          {
            (unused_outputs & cf.y(_var)).PickOneCube(scube);
          }
          else
          {
            unused_outputs.PickOneCube(scube);
          }
          for (unsigned i = 0u; i < cf.num_vars(); ++i)
          {
            ocube &= (scube[3u * i + 1u] == 1) ? cf.y(i) : !cf.y(i);
          }
        }
        else
        {
          ocube = cf.y(_var) & cube_part;

          icube = cf.manager().bddOne();
          if ((unused_inputs & !cf.x(_var)) != cf.manager().bddZero())
          {
            (unused_inputs & !cf.x(_var)).PickOneCube(scube);
          }
          else
          {
            unused_inputs.PickOneCube(scube);
          }
          for (unsigned i = 0u; i < cf.num_vars(); ++i)
          {
            icube &= (scube[3u * i] == 1) ? cf.x(i) : !cf.x(i);
          }
        }

        cube = icube & ocube;
        f |= cube;

        compute_cofactors();
      }

      change = ((char)1u - change);
      ++access;
    } while ( ( cube & ( pp | np ) ) == cf.manager().bddZero() );

    /* Cleanup */
    delete[] scube;

    apply_gates(lf, rf);
  }

  void resolve_k_cycles()
  {
    while (cf.cofactor(f, _var, true, false) != cf.manager().bddZero() || cf.cofactor(f, _var, false, true) != cf.manager().bddZero())
    {
      compute_cofactors();

      /* special case #P' is 0 and #N' is not zero, then swap */
      if ( pp == cf.manager().bddZero() && np != cf.manager().bddZero() )
      {
        apply_gates( cf.manager().bddOne(), cf.manager().bddOne() );
      }

      if ( false /*verbose*/ )
      {
        std::cout << "#N: " << cf.cofactor( f, _var, false, false ).CountMinterm( 2u * cf.num_vars() ) << " "
                  << "#P': " << cf.cofactor( f, _var, true, false ).CountMinterm( 2u * cf.num_vars() ) << " "
                  << "#N': " << cf.cofactor( f, _var, false, true ).CountMinterm( 2u * cf.num_vars() ) << " "
                  << "#P: " << cf.cofactor( f, _var, true, true ).CountMinterm( 2u * cf.num_vars() ) << std::endl;
      }

      cycle_step();
    }
  }

  typedef std::tuple<
    bool,                                                   // p1[_var] value
    std::function<BDD(rcbdd_synthesis_manager*)>,           // compute fc
    unsigned,                                               // scube offset
    std::function<BDD(rcbdd_synthesis_manager*, BDD, BDD)>, // update f
    std::function<void(circuit&, const circuit&)>           // update circ
    > resolve_cycles_configuration_t;

  /* DISABLE FOR NOW
  resolve_cycles_configuration_t resolve_x = std::make_tuple(
    true,
    []( rcbdd_synthesis_manager* manager ) {
      BDD fc = manager->ppx & manager->cf.move_xs_to_tmp( manager->npx );
      for ( unsigned j = 0u; j < manager->_var; ++j )
      {
        fc &= ( manager->cf.x( j ).Xnor( manager->cf.z( j ) ) );
      }
      return fc;
    },
    0u,
    []( rcbdd_synthesis_manager* manager, BDD gcirc, BDD f ) { return manager->cf.compose( gcirc, f ); },
    []( circuit& circ, const circuit& c ) { append_circuit( circ, c ); } );

  resolve_cycles_configuration_t resolve_y = std::make_tuple(
    false,
    []( rcbdd_synthesis_manager* manager ) {
      BDD fc = manager->ppy & manager->cf.move_ys_to_tmp( manager->npy );
      for ( unsigned j = 0u; j < manager->_var; ++j )
      {
        fc &= ( manager->cf.y( j ).Xnor( manager->cf.z( j ) ) );
      }
      return fc;
    },
    1u,
    []( rcbdd_synthesis_manager* manager, BDD gcirc, BDD f ) { return manager->cf.compose( f, gcirc ); },
    []( circuit& circ, const circuit& c ) { prepend_circuit( circ, c ); } );
  */

  void resolve_cycles_with_transpositions( const resolve_cycles_configuration_t& configuration )
  {
    if ( false /*verbose*/ )
    {
      std::cout << boost::format( "[i] resolve cycles for var %d" ) % _var << std::endl;
    }

    while ( cf.cofactor( f, _var, true, false ) != cf.manager().bddZero() ||
            cf.cofactor( f, _var, false, true ) != cf.manager().bddZero() )
    {
      compute_cofactors();

      /* Extract two cubes and save them into bitset */
      boost::dynamic_bitset<> p1( cf.num_vars() ), p2( cf.num_vars() );

      p1[_var] =  std::get<0>( configuration );
      p2[_var] = !std::get<0>( configuration );

      char *scube = new char[3u * cf.num_vars()];

      BDD fc = std::get<1>( configuration )( this );

      if ( !smart_pickcube )
      {
        fc.PickOneCube( scube );
      }
      else
      {
        pick_one_cube_for_rcbdd( fc, scube );
      }

      for ( unsigned i = 0u; i < cf.num_vars(); ++i )
      {
        if ( i == _var ) continue;
        p1[i] = scube[3u * i + std::get<2>( configuration )];
        p2[i] = scube[3u * i + 2u];
      }

      delete[] scube;

      /* Create circuit for p1 and p2 */
      if ( verbose )
      {
        //std::cout << "[i] realize transposition (" << p1 << " " << p2 << ")" << std::endl;
      }
      circuit c( cf.num_vars() );
      pattern_to_circuit( c, p1, p2 );
      if ( verbose )
      {
        //std::cout << "[i] result: " << std::endl << c;
      }

      /* Apply circuit to f */
      BDD gcirc = cf.create_from_circuit( c );
      f = std::get<3>( configuration )( this, gcirc, f );
      std::get<4>( configuration )( circ, c );
    }
  }

  void add_toffoli_gate( const cube_t& cube, unsigned offset )
  {
    gate::control_container controls;
    for ( unsigned i = 0u; i < cf.num_vars(); ++i )
    {
      if ( cube.second[3u * i + offset] )
      {
        controls += make_var( i, cube.first[3u * i + offset] );
      }
    }

    insert_toffoli( circ, insert_position, controls, _var );
    insert_position++;
  }

  void create_toffoli_gates_with_exorcism(const BDD& gate, unsigned var, unsigned offset, bool add_gates_to_circuit = true)
  {
    if (gate == cf.manager().bddZero()) return;

    if ( genesop )
    {
      std::ofstream esopout;
      esopout.open( boost::str( boost::format( "/tmp/%s_%d_%d.pla" ) % name % var % offset ) );
      esopout << ".i " << cf.num_vars() << std::endl;
      esopout << ".o " << 1 << std::endl;

      int *cube;
      CUDD_VALUE_TYPE value;
      DdGen *gen;

      Cudd_ForeachCube(gate.manager(), gate.getNode(), gen, cube, value)
      {
        char v;

        for (unsigned i = 0u; i < cf.num_vars(); ++i)
        {
          v = cube[3u * i + offset];
          if (v != 2)
          {
            esopout << ((v == 0) ? "0" : "1");
          }
          else
          {
            esopout << "-";
          }
        }

        esopout << " 1" << std::endl;
      }

      esopout << ".e" << std::endl;
      esopout.close();
    }

    if ( create_gates )
    {
      if ( add_gates_to_circuit )
      {
        esopmin.settings()->set( "on_cube", cube_function_t( [this, &offset]( const cube_t& c ) { add_toffoli_gate( c, offset ); } ) );
      }
      else
      {
        esopmin.settings()->set( "on_cube", cube_function_t( [this, &offset]( const cube_t& c ) {} ) );
      }
      esopmin.settings()->set( "verify", false );
      esopmin( gate.manager(), gate.getNode() );

      if ( add_gates_to_circuit && offset == 1u )
      {
        insert_position -= esopmin.statistics()->get<unsigned>( "cube_count" );
      }

      total_toffoli_gates += esopmin.statistics()->get<unsigned>( "cube_count" );
      total_control_lines += esopmin.statistics()->get<unsigned>( "literal_count" );
    }

    /*
    system("exorcism /tmp/test.esop");

    // Get number of gates
    total_toffoli_gates += boost::lexical_cast<unsigned long long>(execute_and_return_output("cat /tmp/test.esop | grep -v -e \"^[#\\.]\" | wc -l"));
    total_control_lines += boost::lexical_cast<unsigned long long>(execute_and_return_output("cat /tmp/test.esop | grep -v -e \"^[#\\.]\" | awk '{print $1}' | tr '\\n' ' ' | sed -e \"s/[^01]//g\" | wc -c"));

    // Get gates
    using boost::format;
    using boost::str;

    std::ifstream is;
    is.open("/tmp/test.esop");

    std::string line;
    while (std::getline(is, line)) {
      boost::trim(line);
      if (line.empty()) continue;

      if (line[0] == '0' || line[0] == '1' || line[0] == '-') {
        std::string negate, vars;
        unsigned c = 0u;
        for (unsigned i = 0u; i < mgr.num_vars(); ++i) {
          if (line[i] == '0') {
            negate += str(format("t1 x%d\n") % i);
          }
          if (line[i] == '0' || line[i] == '1') {
            vars += str(format(" x%d") % i);
            ++c;
          }
        }
        std::string _gate = str(format("%st%d%s x%d\n%s") % negate % (c + 1u) % vars % _var % negate);
        if (offset == 0u) {
          real_l += _gate;
        } else {
          real_r = _gate + real_r;
        }
      }
    }
    is.close();
    */
  }

  void default_synthesis()
  {
    null_stream ns;
    std::ostream null_out( &ns );
    boost::progress_display show_progress( cf.num_vars(), progress ? std::cout : null_out );

    for (unsigned var = 0; var < cf.num_vars(); ++var)
    {
      ++show_progress;
      set_var(var);

      if ( synthesis_method == ResolveCycles )
      {
        only_left_gate_shortcut();
        resolve_one_cycles();
        resolve_two_cycles();
        resolve_k_cycles();

        if ( false /*verbose*/ )
        {
          std::cout << "Target: " << _var << std::endl << " - left control function:" << std::endl;
          left_f.PrintMinterm();
          std::cout << " - right control function:" << std::endl;
          right_f.PrintMinterm();
        }

        create_toffoli_gates_with_exorcism(left_f, var, 0u);
        create_toffoli_gates_with_exorcism(right_f, var, 1u);
      }
      else if ( synthesis_method == TranspositionsX )
      {
        //resolve_cycles_with_transpositions( resolve_x );
      }
      else if ( synthesis_method == TranspositionsY )
      {
        //resolve_cycles_with_transpositions( resolve_y );
      }
    }
  }

  void heuristic_swap() //Get chi
  {
    std::vector<unsigned> list_lines;
    for (unsigned var = 0; var < cf.num_vars(); ++var)
    {
      list_lines.push_back(var);
    }

    BDD lf_c, lr_c;

    while (!list_lines.empty())
    {
      unsigned min_cost = UINT_MAX;
      unsigned best_line = 0u;

      for (unsigned i = 0u; i < list_lines.size(); ++i)
      {
        BDD oldchi = f; //make a copy of chi
        unsigned old_control_lines = total_control_lines;
        unsigned old_toffoli_gates = total_toffoli_gates;
        if ( false /*verbose*/ )
        {
          std::cout << "[I] - total_toffoli_gates" << total_toffoli_gates << std::endl;
        }
        set_var(list_lines[i]);

        if ( false /*verbose*/ )
        {
          std::cout << "[I] set_var(var): " << _var << std::endl;
        }
        only_left_gate_shortcut();
        resolve_one_cycles();
        resolve_two_cycles();
        resolve_k_cycles();

        if ( false /*verbose*/ )
        {
          std::cout << "Target: " << _var << std::endl << " - left control function:" << std::endl;
          left_f.PrintMinterm();
          std::cout << "[I] - right control function:" << std::endl;
          right_f.PrintMinterm();
        }

        create_toffoli_gates_with_exorcism( left_f, list_lines[i], 0u, false );
        create_toffoli_gates_with_exorcism( right_f, list_lines[i], 1u, false );


        // Determine cost and save in new_cost
        unsigned new_cost = total_toffoli_gates - old_toffoli_gates;

        if ( false /*verbose*/ )
        {
          std::cout << "[I] h1: Lines:    " << cf.num_vars() << std::endl;
          std::cout << "[I] h1: Gates:    " << new_cost << std::endl;
          std::cout << "[I] Controls:     " << total_control_lines << std::endl;
        }

        if (new_cost < min_cost)
        {
          best_line = list_lines[i];
          min_cost = new_cost;
          if ( false /*verbose*/ )
          {
            std::cout << "[I] Min cost: " << min_cost << std::endl;
          }
        }

        f = oldchi;
        total_toffoli_gates = old_toffoli_gates;
        total_control_lines = old_control_lines;
      }

      set_var(best_line);
      // Synthesis with best_line
      only_left_gate_shortcut();
      resolve_one_cycles();
      resolve_two_cycles();
      resolve_k_cycles();

      create_toffoli_gates_with_exorcism(left_f, best_line, 0u);
      create_toffoli_gates_with_exorcism(right_f, best_line, 1u);

      list_lines.erase(std::remove(list_lines.begin(),list_lines.end(),best_line));

      if ( false /*verbose*/ )
      {
        std::cout << "[I] Best Line: " << best_line << std::endl;
      }
    }
  }

  void heuristic_hamming()
  {
    std::vector<unsigned> list_lines;
    for (unsigned var = 0; var < cf.num_vars(); ++var)
    {
      list_lines.push_back(var);
    }

    BDD lf_c, lr_c;

    while (!list_lines.empty())
    {
      double min_cost = DBL_MAX;
      unsigned best_line = 0u;

      for (unsigned i = 0u; i < list_lines.size(); ++i)
      {
        // Determine costs and save in new_cost
        double new_cost = cf.cofactor(f, list_lines[i], false, true).CountMinterm(2 * cf.num_vars());


        if (new_cost < min_cost)
        {
          best_line = list_lines[i];
          min_cost = new_cost;
          if ( false /*verbose*/ )
          {
            std::cout << "[I] Min cost: " << min_cost << std::endl;
          }
        }
      }

      set_var(best_line);
      // Synthesis with best_line
      only_left_gate_shortcut();
      resolve_one_cycles();
      resolve_two_cycles();
      resolve_k_cycles();

      create_toffoli_gates_with_exorcism(left_f, best_line, 0u);
      create_toffoli_gates_with_exorcism(right_f, best_line, 1u);

      list_lines.erase(std::remove(list_lines.begin(),list_lines.end(),best_line));

      if ( false /*verbose*/ )
      {
        std::cout << "[I] Best Line: " << best_line << std::endl;
      }
    }
  }

  const rcbdd& cf;
  circuit& circ;

  bool verbose;
  bool progress;
  std::string name;
  bool genesop;
  dd_based_esop_optimization_func esopmin;
  bool create_gates;
  bool smart_pickcube;
  SynthesisMethod synthesis_method;

  BDD f;
  BDD left_f, right_f;
  unsigned _var;
  unsigned insert_position;
  BDD n, pp, np, p;
  BDD nx, ppx, npx, px;
  BDD ny,  ppy, npy, py;
  unsigned total_control_lines = 0u, total_toffoli_gates = 0u;
  unsigned long long access = 0ull;
  std::vector<int> node_count;
};

bool rcbdd_synthesis( circuit& circ, const rcbdd& cf, properties::ptr settings, properties::ptr statistics )
{
  /* Settings */
  auto verbose          = get( settings, "verbose",          false                             );
  auto progress         = get( settings, "progress",         false                             );
  auto name             = get( settings, "name",             std::string( "test" )             );
  auto genesop          = get( settings, "genesop",          false                             );
  auto esopmin          = get( settings, "esopmin",          dd_based_esop_optimization_func() );
  auto create_gates     = get( settings, "create_gates",     true                              );
  /* 0: default, 1: swap, 2: hamming */
  auto mode             = get( settings, "mode",             0u                                );
  auto synthesis_method = get( settings, "synthesis_method", ResolveCycles                     );
  auto smart_pickcube   = get( settings, "smart_pickcube",   true                              );

  /* Timing */
  properties_timer t( statistics );

  rcbdd_synthesis_manager mgr( cf, circ );
  mgr.verbose          = verbose;
  mgr.progress         = progress;
  mgr.name             = name;
  mgr.genesop          = genesop;
  mgr.esopmin          = esopmin;
  mgr.create_gates     = create_gates;
  mgr.synthesis_method = synthesis_method;
  mgr.smart_pickcube   = smart_pickcube;
  switch ( mode )
  {
  case 1u:
    mgr.heuristic_swap();
    break;
  case 2u:
    mgr.heuristic_hamming();
    break;
  default:
    mgr.default_synthesis();
  };

  if ( statistics )
  {
    statistics->set( "access", mgr.access );
    statistics->set( "node_count", mgr.node_count );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
