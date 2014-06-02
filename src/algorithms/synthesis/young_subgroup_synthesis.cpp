/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "young_subgroup_synthesis.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

#include <boost/assign/std/vector.hpp>

#include <core/circuit.hpp>
#include <core/truth_table.hpp>
#include <core/functions/add_circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/clear_circuit.hpp>
#include <core/functions/copy_metadata.hpp>
#include <core/functions/fully_specified.hpp>
#include <core/io/write_pla.hpp>
#include <core/utils/timer.hpp>

#include <classical/optimization/esop_minimization.hpp>

#include <cuddObj.hh>

using namespace boost::assign;

namespace revkit
{

typedef std::vector<std::vector<int> > truth_table_column_t;

class young_subgroup_synthesis_manager
{
public:
  young_subgroup_synthesis_manager( circuit& circ ) : circ( circ )
  {
    /* initialize BDD variables */
    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      cudd.bddVar();
    }
  }

  void add_gate_for_cube( circuit& gatecirc, unsigned target, const cube_t& cube )
  {
    gate::control_container controls;

    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      if ( !cube.second[i] ) continue;
      controls += make_var( i, cube.first[i] );
    }

    append_toffoli( gatecirc, controls, target );
  }

  void synthesis_gate( BDD control_function, unsigned target, unsigned& start )
  {
    circuit gatecirc( circ.lines() );

    properties::ptr settings( new properties() );
    settings->set( "on_cube", cube_function_t( [this, &gatecirc, &target]( const cube_t& c ) { add_gate_for_cube( gatecirc, target, c ); } ) );
    settings->set( "verify", true );
    esop_minimization( control_function.manager(), control_function.getNode(), settings );

    if ( gatecirc.num_gates() )
    {
      insert_circuit( circ, start, gatecirc );
      start += gatecirc.num_gates();
    }
  }

  void basic_first_step( const binary_truth_table& spec,
                         truth_table_column_t& v_f_in, truth_table_column_t& v_f_out,
                         truth_table_column_t& v_b_in, truth_table_column_t& v_b_out)
  {
    unsigned bw = spec.num_inputs();
    for (binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it)
    {
      std::vector<int> in_cube, out_cube;
      binary_truth_table::in_const_iterator c = it->first.first;
      binary_truth_table::out_const_iterator ci = it->second.first;
      for (unsigned i = 0; i < bw; ++i)
      {
        in_cube.push_back(**(c + i));
        out_cube.push_back(**(ci + i));
      }
      v_f_in += in_cube;
      v_b_in += out_cube;
      out_cube[0] = -1;
      in_cube[0] = -1;
      v_f_out += in_cube;
      v_b_out += out_cube;
    }
  }

  int find(const truth_table_column_t& vect, const truth_table_column_t::value_type& v)
  {
    unsigned i = 0u;
    while (i < vect.size() && !(std::equal(vect[i].begin(), vect[i].end(), v.begin())))
    {
      i++;
    }
    return i;
  }

  BDD get_control_function( const truth_table_column_t& in,
                            const truth_table_column_t& out,
                            unsigned pos )
  {
    BDD bdd = cudd.bddZero();

    for ( unsigned i = 0u; i < in.size(); ++i )
    {
      if ( in[i][pos] == 0u && out[i][pos] == 1u )
      {
        BDD cube = cudd.bddOne();

        for ( unsigned j = 0; j < circ.lines(); ++j )
        {
          if ( j == pos ) continue;
          cube &= in[i][j] == 1 ? cudd.bddVar( j ) : !cudd.bddVar( j );
        }

        bdd |= cube;
      }
    }

    return bdd;
  }

  void add_gates( const truth_table_column_t& vf_in, const truth_table_column_t& vf_out,
                  const truth_table_column_t& vb_in, const truth_table_column_t& vb_out,
                  unsigned pos,
                  unsigned& start)
  {
    BDD bdd_front = get_control_function( vf_in, vf_out, pos );
    BDD bdd_back  = get_control_function( vb_in, vb_out, pos );

    if ( verbose )
    {
      std::cout << "BDD for front:" << std::endl;
      bdd_front.PrintMinterm();
    }
    synthesis_gate(bdd_front, pos, start);
    unsigned back = start;

    if ( verbose )
    {
      std::cout << "BDD for back:" << std::endl;
      bdd_back.PrintMinterm();
    }
    synthesis_gate(bdd_back, pos, back);
  }

  void add_gates( const truth_table_column_t& vf, const truth_table_column_t& vb,
                  unsigned pos, unsigned& start )
  {
    BDD bdd = get_control_function( vf, vb, pos );

    if ( verbose )
    {
      std::cout << "BDD for middle:" << std::endl;
      bdd.PrintMinterm();
    }
    synthesis_gate( bdd, pos, start);
  }

  void next_step( truth_table_column_t& v_f_in, truth_table_column_t& v_f_out,
                  truth_table_column_t& v_b_in, truth_table_column_t& v_b_out,
                  unsigned pos)
  {
    v_f_in = v_f_out;
    v_b_in = v_b_out;
    for (unsigned i = 0; i < v_f_in.size(); ++i) {
      v_f_out[i][pos] = -1;
      v_b_out[i][pos] = -1;

    }
  }

  void build_shape( truth_table_column_t& v_front, truth_table_column_t& v_back, unsigned pos )
  {
    unsigned j = 0u, nb_cubes = 0u;
    while (j < v_front.size()) {
      unsigned k = j;
      while (k < v_front.size() && v_front[k][pos] != -1)
        k++;
      //  std::cout << "k=" << k  << "j=" << j<< std::endl;
      if (k < v_front.size()) {
        //    std::cout << "index1= "<<v_back.size()<< std::endl;
        std::vector<int> v = v_front[k];
        v_front[k][pos] = 0;
        unsigned index = find(v_front, v);
        v_front[index][pos] = 1;
        v = v_back[index];
        v_back[index][pos] = 1;
        index = find(v_back, v);
        v_back[index][pos] = 0;
        j = index;
        nb_cubes++;
        //    std::cout << "index3= "<< index<<"  pos="<<pos << std::endl;

      } else {
        //    std::cout << "else===" << std::endl;
        break;
      }
    }
  }

  Cudd cudd;
  circuit& circ;
  bool verbose;
};


  bool young_subgroup_synthesis(circuit& circ, const binary_truth_table& spec, properties::ptr settings, properties::ptr statistics)
  {
    /* Settings */
    bool verbose = get( settings, "verbose", false );

    unsigned bw = spec.num_inputs(), i = 0u;

    std::vector<std::vector<int> > v_front, v_back, v_f_in, v_b_in;
    timer < properties_timer > t;

    if (statistics) {
      properties_timer rt(statistics);
      t.start(rt);
    }

    // circuit has to be empty
    clear_circuit(circ);

    // truth table has to be fully specified
    if (!fully_specified(spec)) {
      set_error_message(statistics, "truth table `spec` is not fully specified.");
      return false;
    }

    circ.set_lines(bw);

    // copy metadata
    copy_metadata(spec, circ);

    young_subgroup_synthesis_manager mgr( circ );
    mgr.verbose = verbose;

    // Step 1
    mgr.basic_first_step(spec, v_f_in, v_front, v_b_in, v_back);

    // Step 2
    unsigned start = 0u;
    while (i < spec.num_inputs()) {
      //std::cout << "i=" << i << std::endl;
      if (i < spec.num_inputs() - 1) {
        //  print_vec(v_front, v_back);
        mgr.build_shape(v_front, v_back, i);
        mgr.add_gates(v_f_in, v_front, v_b_in, v_back, i, start);
        mgr.next_step(v_f_in, v_front, v_b_in, v_back, i + 1);

      } else {
        mgr.add_gates(v_f_in, v_b_in, i, start);
        //  print_vec(v_f_in, v_b_in);
      }
      i++;
    }

    return true;
  }

  truth_table_synthesis_func young_subgroup_synthesis_func(properties::ptr settings, properties::ptr statistics)
  {
    truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
      return young_subgroup_synthesis( circ, spec, settings, statistics );
    };
    f.init( settings, statistics );
    return f;
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
