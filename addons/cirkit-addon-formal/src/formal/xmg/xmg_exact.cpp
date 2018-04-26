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

#include "xmg_exact.hpp"

#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/spec_representation.hpp>

#ifdef ADDON_FORMAL
#include <formal/utils/z3_utils.hpp>
#include <z3++.h>
#endif

using namespace boost::assign;

using boost::format;
using boost::str;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

struct xmg_exact_instance
{
  struct wire
  {
    wire( xmg_exact_instance* inst, unsigned gate_id, unsigned input_id )
      : inst( inst ),
        sel( create_sel( inst, gate_id, input_id ) ),
        neg( create_neg( inst, gate_id, input_id ) )
    {
    }

    int sel_value( const z3::model& m ) const
    {
      return to_bitset( m.eval( sel ) ).to_ulong();
    }

    bool neg_value( const z3::model& m ) const
    {
      return expr_to_bool( m.eval( neg ) );
    }

    xmg_exact_instance* inst;
    z3::expr sel;
    z3::expr neg;

  private:
    z3::expr create_sel( xmg_exact_instance* inst, unsigned gate_id, unsigned input_id ) const
    {
      const auto name = str( format( "sel%d_%d" ) % input_id % gate_id );
      auto e = inst->ctx.bv_const( name.c_str(), inst->bw );

      /* assertion for sel[level] <= n + k */
      inst->solver.add( z3::ule( e, inst->ctx.bv_val( inst->num_vars + gate_id, inst->bw ) ) );

      return e;
    }

    z3::expr create_neg( xmg_exact_instance* inst, unsigned gate_id, unsigned input_id ) const
    {
      return inst->ctx.bool_const( str( format( "neg%d_%d" ) % input_id % gate_id ).c_str() );
    }
  };

  struct gate
  {
    gate( xmg_exact_instance* inst, unsigned gate_id )
      : inst( inst),
        gate_id( gate_id ),
        inputs({wire( inst, gate_id, 0u ), wire( inst, gate_id, 1u ), wire( inst, gate_id, 2u )}),
        type_expr( inst->ctx.bool_const( str( format( "type%d" ) % gate_id ).c_str() ) )
    {
      /* type = 0 : MAJ, type = 1 : XOR */
      inst->solver.add( implies( is_xor(), !inputs[2].neg ) );
      inst->solver.add( implies( is_xor(), inst->equals( inputs[2].sel, inst->ctx.bv_val( inst->num_vars + gate_id, inst->bw ) ) ) );
    }

    void symmetry_breaking_commutativity() const
    {
      inst->solver.add( z3::ult( inputs[0].sel, inputs[1].sel ) );
      inst->solver.add( implies( is_maj(), z3::ult( inputs[1].sel, inputs[2].sel ) ) );
    }

    void symmetry_breaking_inverters_xor() const
    {
      inst->solver.add( implies( is_xor(), !inputs[0].neg && !inputs[1].neg ) );
    }

    void symmetry_breaking_inverters_maj() const
    {
      auto min_neg = !( ( inputs[0].neg && inputs[1].neg ) || ( inputs[0].neg && inputs[2].neg ) || ( inputs[1].neg && inputs[2].neg ) );
      inst->solver.add( implies( is_maj(), min_neg ) );
    }

    void symmetry_breaking_colexicographic( const gate& prev )
    {
    }

    void symmetry_breaking_different( const gate& other )
    {
      inst->solver.add( implies( is_maj() && other.is_maj(),
                                 inputs[0].sel != other.inputs[0].sel ||
                                 inputs[1].sel != other.inputs[1].sel ||
                                 inputs[2].sel != other.inputs[2].sel ||
                                 inputs[0].neg != other.inputs[0].neg ||
                                 inputs[1].neg != other.inputs[1].neg ||
                                 inputs[2].neg != other.inputs[2].neg ) );
      inst->solver.add( implies( is_xor() && other.is_xor(),
                                 inputs[0].sel != other.inputs[0].sel ||
                                 inputs[1].sel != other.inputs[1].sel ||
                                 inputs[0].neg != other.inputs[0].neg ||
                                 inputs[1].neg != other.inputs[1].neg ) );
    }

    void symmetry_breaking_constant_input() const
    {
      inst->solver.add( !( is_xor() && inst->equals( inputs[0].sel, 0u ) ) );
    }

    void symmetry_breaking_associativity( const gate& other ) const
    {
    }

    void block_for_support( const boost::dynamic_bitset<>& support )
    {
      foreach_bit( ~support, [this, &support]( unsigned pos ) {
          inst->solver.add( !( inst->equals( inputs[0].sel, pos + 1u ) ) );
          inst->solver.add( !( inst->equals( inputs[1].sel, pos + 1u ) ) );
          inst->solver.add( implies( is_maj(), !( inst->equals( inputs[2].sel, pos + 1u ) ) ) );
        } );
    }

    const wire& operator[]( unsigned index ) const
    {
      return inputs[index];
    }

    inline const z3::expr& type() const
    {
      return type_expr;
    }

    inline z3::expr is_maj() const
    {
      return !type_expr;
    }

    inline z3::expr is_xor() const
    {
      return type_expr;
    }

    bool type_value( const z3::model& m ) const
    {
      return expr_to_bool( m.eval( type_expr ) );
    }

  private:
    xmg_exact_instance* inst;
    unsigned gate_id;

  public:
    std::vector<wire> inputs;
    z3::expr type_expr;
  };

  /**
   * @param timeout given in seconds
   */
  xmg_exact_instance( unsigned num_vars, bool expl, boost::optional<unsigned> timeout = boost::none ) :
    num_vars( num_vars ),
    solver( make_solver( expl ) )
  {
    auto upper_bound = 7u;
    if ( num_vars > 4u )
    {
      upper_bound += 10 * ( ( 1u << ( num_vars - 4u ) ) - 1 );
    }
    bw = (unsigned)ceil( log( 2u + num_vars + upper_bound ) / log( 2u ) );

    if ( (bool)timeout )
    {
      z3::params p( ctx );
      p.set( ":timeout", *timeout * 1000u );
      solver.set( p );
    }
  }

  z3::solver make_solver( bool expl )
  {
    if ( expl )
    {
      return z3::solver( ctx );
      //z3::params p1( ctx ); p1.set( "random-seed", (unsigned)0xCAFE );
      //return with( z3::tactic( ctx, "smt" ), p1 ).mk_solver();

      // try
      // {
      //   z3::params p1( ctx ); p1.set( "random-seed", 10u );
      //   z3::params p2( ctx ); p2.set( "random-seed", 100u );
      //   z3::params p3( ctx ); p3.set( "random-seed", 1000u );
      //   Z3_tactic tactics[] = {with( z3::tactic( ctx, "smt" ), p1 ), with( z3::tactic( ctx, "smt" ), p2 ), with( z3::tactic( ctx, "smt" ), p3 )};
      //   return z3::tactic( ctx, Z3_tactic_par_or( ctx, 3, tactics ) ).mk_solver();
      // }
      // catch ( z3::exception e )
      // {
      //   std::cerr << e << std::endl;
      //   assert( false );
      // }
      // //Z3_tactic Z3_API Z3_tactic_par_or(__in Z3_context c, __in unsigned num, __in_ecount(num) Z3_tactic const ts[]);

      // //return z3::solver( ctx );
      //return ( z3::tactic( ctx, "aig" ) & z3::tactic( ctx, "bit-blast" ) &  z3::tactic( ctx, "solve-eqs" ) & z3::tactic( ctx, "sat" ) ).mk_solver();
    }
    else
    {
      return ( z3::tactic( ctx, "qe" ) & z3::tactic( ctx, "smt" ) ).mk_solver();
    }
  }

  void add_level( const boost::dynamic_bitset<>& symmetry_breaking )
  {
    unsigned level = gates.size();

    gates += gate( this, level );

    /* symmetry breaking */
    if ( symmetry_breaking[0u] )
    {
      gates[level].symmetry_breaking_commutativity();
    }

    if ( symmetry_breaking[1u] )
    {
      gates[level].symmetry_breaking_inverters_maj();
      gates[level].symmetry_breaking_inverters_xor();
    }

    if ( symmetry_breaking[2u] )
    {
      for ( auto i = 0u; i < level; ++i )
      {
        gates[level].symmetry_breaking_different( gates[i] );
      }
    }

    if ( symmetry_breaking[3u] )
    {
      for ( auto i = 0u; i < level; ++i )
      {
        gates[level].symmetry_breaking_associativity( gates[i] );
      }
    }

    gates[level].symmetry_breaking_constant_input();

    if ( symmetry_breaking[4u] )
    {
      if ( level > 0u )
      {
        gates[level].symmetry_breaking_colexicographic( gates[level - 1u] );
      }
    }

    if ( symmetry_breaking[5u] )
    {
      gates[level].block_for_support( support );
    }

    if ( symmetry_breaking[6u] )
    {
      for ( const auto& p : symmetries )
      {
        for ( auto c = 0u; c < 3u; ++c )
        {
          if ( level == 0u && c == 0u ) { continue; }

          z3::expr A = equals( gates[level].inputs[c].sel, p.first + 1 );
          if ( c == 2u ) { A = A && gates[level].is_maj(); } /* XOR */

          z3::expr B = ctx.bool_val( true );

          for ( auto j = 0u; j < level; ++j )
          {
            for ( auto d = 0u; d < 2u; ++d )
            {
              A = A && !equals( gates[j].inputs[d].sel, p.first + 1 );
              B = B && !equals( gates[j].inputs[d].sel, p.second + 1 );
            }
            A = A && implies( gates[j].is_maj(), !equals( gates[j].inputs[2u].sel, p.first + 1 ) );
            B = B && implies( gates[j].is_maj(), !equals( gates[j].inputs[2u].sel, p.second + 1 ) );
          }
          for ( auto d = 0u; d < c; ++d )
          {
            assert( d != 2u );
            A = A && !equals( gates[level].inputs[d].sel, p.first + 1 );
            B = B && !equals( gates[level].inputs[d].sel, p.second + 1 );
          }

          solver.add( implies( A, B ) );
        }
      }
    }
  }

  void constrain( const tt& spec )
  {
    auto N = 1u << num_vars;

    std::vector<std::vector<z3::expr>> out( N );
    std::vector<std::vector<std::vector<z3::expr>>> in( 3u, std::vector<std::vector<z3::expr>>( N ) );

    /* value constraints */
    for ( auto j = 0u; j < N; ++j )
    {
      for ( auto level = 0u; level < gates.size(); ++level )
      {
        out[j] += ctx.bool_const( str( format( "out_%d_%d" ) % j % level ).c_str() );
        in[0u][j] += ctx.bool_const( str( format( "in1_%d_%d" ) % j % level ).c_str() );
        in[1u][j] += ctx.bool_const( str( format( "in2_%d_%d" ) % j % level ).c_str() );
        in[2u][j] += ctx.bool_const( str( format( "in3_%d_%d" ) % j % level ).c_str() );

        /* assertion for out[j][level] = M(in1[j][level],in2[j][level],in3[j][level] */
        if ( with_xor )
        {
          solver.add( out[j][level] == ( implies( !gates[level].type(), ( in[0u][j][level] && in[1u][j][level] ) || ( in[0u][j][level] && in[2u][j][level] ) || ( in[1u][j][level] && in[2u][j][level] ) )
                                         && implies( gates[level].type(), logic_xor( in[0u][j][level], in[1u][j][level] ) ) ) );
        }
        else
        {
          solver.add( out[j][level] == ( ( in[0u][j][level] && in[1u][j][level] ) || ( in[0u][j][level] && in[2u][j][level] ) || ( in[1u][j][level] && in[2u][j][level] ) ) );
        }

        /* assertions for in[x][j][level] = neg[level] ^ ite( sel[level], ... ) */
        boost::dynamic_bitset<> val( num_vars, j );
        for ( auto x = 0u; x < 3u; ++x )
        {
          solver.add( implies( equals( gates[level][x].sel, 0 ),
                               in[x][j][level] == logic_xor( gates[level][x].neg, ctx.bool_val( false ) ) ) );

          for ( auto l = 0u; l < num_vars; ++l )
          {
            solver.add( implies( equals( gates[level][x].sel, l + 1 ),
                                 in[x][j][level] == logic_xor( gates[level][x].neg, ctx.bool_val( val[l] ) ) ) );
          }
          for ( auto l = 0u; l < level; ++l )
          {
            solver.add( implies( equals( gates[level][x].sel, l + 1 + num_vars ),
                                 in[x][j][level] == logic_xor( gates[level][x].neg, out[j][l] ) ) );
          }
        }
      }

      solver.add( out[j].back() == ctx.bool_val( spec[j] ) );
    }
  }

  xmg_graph extract_xmg( const std::string& model_name, const std::string& output_name, bool invert, bool very_verbose )
  {
    xmg_graph xmg( model_name );

    std::vector<xmg_function> inputs, nodes;
    for ( auto i = 0u; i < num_vars; ++i )
    {
      inputs += xmg.create_pi( str( format( "x%d" ) % i ) );
    }

    const auto m = solver.get_model();

    for ( auto i = 0u; i < gates.size(); ++i )
    {
      const auto type = gates[i].type_value( m );

      xmg_function children[3];

      for ( auto x = 0u; x < ( type ? 2u : 3u ); ++x )
      {
        auto sel_val = gates[i][x].sel_value( m );

        if ( sel_val == 0 )
        {
          children[x] = xmg.get_constant( false );
        }
        else if ( sel_val > 0 && sel_val <= static_cast<int>( num_vars ) )
        {
          children[x] = inputs[sel_val - 1u];
        }
        else
        {
          children[x] = nodes[sel_val - num_vars - 1u];
        }

        if ( gates[i][x].neg_value( m ) )
        {
          children[x] = !children[x];
        }
      }

      if ( !type )
      {
        nodes += xmg.create_maj( children[0], children[1], children[2] );
      }
      else
      {
        nodes += xmg.create_xor( children[0], children[1] );
      }

      if ( very_verbose )
      {
        std::cout << format( "added %s node (%d,%d) for i = %d" ) % ( type ? "XOR" : "MAJ" ) % nodes.back().node % nodes.back().complemented % i << std::endl;
        for ( auto x = 0u; x < ( type ? 2u : 3u ); ++x )
        {
          std::cout << format( "  - child %d = (%d,%d)" ) % x % children[x].node % children[x].complemented << std::endl;
        }
      }
    }

    auto f = nodes.back() ^ invert;
    xmg.create_po( f, output_name );

    return xmg;
  }

  void block_solution()
  {
    const auto m = solver.get_model();
    auto clause = ctx.bool_val( false );

    for ( auto i = 0u; i < gates.size(); ++i )
    {
      for ( auto x = 0u; x < 3u; ++x )
      {
        clause = clause || ( gates[i][x].sel != m.eval( gates[i][x].sel ) ) || ( gates[i][x].neg != m.eval( gates[i][x].neg ) );
      }

      if ( with_xor )
      {
        clause = clause || ( gates[i].type() != m.eval( gates[i].type() ) );
      }
    }

    solver.add( clause );
  }

  /* some helper methods */
  z3::expr equals( const z3::expr& expr, unsigned value )
  {
    return ( expr == ctx.bv_val( value, bw ) );
  }

  z3::expr less_equals( const z3::expr& expr, unsigned value )
  {
    return ( z3::ule( expr, ctx.bv_val( value, bw ) ) );
  }

  unsigned num_vars;
  bool with_xor;
  z3::context ctx;
  z3::solver solver;

  std::vector<gate> gates;

  unsigned bw;

  /* spec properties */
  boost::dynamic_bitset<>                    support;
  std::vector<std::pair<unsigned, unsigned>> symmetries;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class xmg_exact_manager
{
public:
  xmg_exact_manager( const spec_representation& spec, const properties::ptr& settings )
    : spec( spec ),
      normal( spec.is_normal() )
  {
    /* meta data */
    model_name          = get( settings, "model_name",          std::string( "exact" ) );
    output_name         = get( settings, "output_name",         std::string( "f" ) );

    /* control algorithm */
    start               = get( settings, "start",               1u );
    all_solutions       = get( settings, "all_solutions",       false );

    /* encoding */
    breaking            = get( settings, "breaking",            std::string( "CIsalty" ) );

    timeout             = get( settings, "timeout",             boost::optional<unsigned>() );
    timeout_heuristic   = get( settings, "timeout_heuristic",   false );
    verbose             = get( settings, "verbose",             false );
    very_verbose        = get( settings, "very_verbose",        false );

    verbose = verbose || very_verbose;
    make_symmetry_breaking_bitset();

    /* pre compute properties for making solving faster */
    support    = spec.support();
    symmetries = spec.symmetric_variables();

    if ( verbose )
    {
      std::cout << "[i] symmetry breaking: " << symmetry_breaking << std::endl;
      std::cout << "[i] support: " << support << std::endl;
      std::cout << "[i] symmetric variables: #" << symmetries.size() << ":";
      for ( const auto& p : symmetries )
      {
        std::cout << " (" << p.first << "," << p.second << ")";
      }
      std::cout << std::endl;
    }
  }

  std::vector<xmg_graph> run()
  {
    /* check trivial case */
    const auto triv = spec.is_trivial();
    if ( (bool)triv )
    {
      return {create_trivial( triv->first, triv->second )};
    }

    if ( !normal )
    {
      spec.invert();
    }

    return exact_mig_size_explicit();
  }

  std::vector<xmg_graph> exact_mig_size_explicit()
  {
    auto k = start;
    while ( true )
    {
      if ( verbose )
      {
        std::cout << boost::format( "[i] check for realization with %d gates" ) % k << std::endl;
      }

      const auto inst = create_instance();

      for ( auto i = 0u; i < k; ++i )
      {
        inst->add_level( symmetry_breaking );
      }

      constrain( inst );

      const auto result = inst->solver.check();
      if ( result == z3::sat )
      {
        return extract_solutions( inst );
      }
      else if ( result == z3::unknown && !timeout_heuristic )
      {
        last_size = k;
        return std::vector<xmg_graph>();
      }

      ++k;
    }
  }

private:
  xmg_graph create_trivial( unsigned id, bool complement )
  {
    xmg_graph xmg( model_name );
    if ( id == 0u )
    {
      xmg.create_po( xmg.get_constant( complement ), output_name );
    }
    else
    {
      for ( auto i = 0u; i < id; ++i )
      {
        xmg.create_pi( str( format( "x%d" ) % i ) );
      }
      xmg.create_po( xmg_function( xmg.inputs().back().first, complement ), output_name );
    }
    return xmg;
  }

  xmg_graph extract_solution( const std::shared_ptr<xmg_exact_instance>& inst ) const
  {
    return inst->extract_xmg( model_name, output_name, !normal, very_verbose );
  }

  inline std::shared_ptr<xmg_exact_instance> create_instance() const
  {
    auto inst = std::make_shared<xmg_exact_instance>( spec.num_vars(), spec.is_explicit(), timeout );
    inst->support    = support;
    inst->symmetries = symmetries;
    return inst;
  }

  std::vector<xmg_graph> extract_solutions( const std::shared_ptr<xmg_exact_instance>& inst ) const
  {
    if ( all_solutions )
    {
      std::vector<xmg_graph> migs;

      do
      {
        migs.push_back( extract_solution( inst ) );
        inst->block_solution();
      } while ( inst->solver.check() == z3::sat );

      return migs;
    }
    else
    {
      return {extract_solution( inst )};
    }
  }

  struct constrain_visitor : public boost::static_visitor<void>
  {
    constrain_visitor( const std::shared_ptr<xmg_exact_instance>& inst ) : inst( inst ) {}

    void operator()( const tt& spec ) const
    {
      inst->constrain( spec );
    }

    void operator()( const mig_graph& spec ) const
    {
      assert( false );
    }

  private:
    const std::shared_ptr<xmg_exact_instance>& inst;
  };

  void constrain( const std::shared_ptr<xmg_exact_instance>& inst ) const
  {
    spec.apply_visitor( constrain_visitor( inst ) );
  }

  void make_symmetry_breaking_bitset()
  {
    symmetry_breaking.resize( 7u );

    for ( auto c : breaking )
    {
      switch ( c )
      {
      case 'C': symmetry_breaking.set( 0u ); break;
      case 'I': symmetry_breaking.set( 1u ); break;
      case 's': symmetry_breaking.set( 2u ); break;
      case 'a': symmetry_breaking.set( 3u ); break;
      case 'l': symmetry_breaking.set( 4u ); break;
      case 't': symmetry_breaking.set( 5u ); break;
      case 'y': symmetry_breaking.set( 6u ); break;
      };
    }
  };

private:
  spec_representation spec;
  bool normal;
  unsigned start;
  std::string model_name;
  std::string output_name;
  bool all_solutions;
  std::string breaking;
  boost::optional<unsigned> timeout;
  bool timeout_heuristic;
  bool verbose;
  bool very_verbose;

  /* pre-computed function properties */
  boost::dynamic_bitset<>                    support;
  std::vector<std::pair<unsigned, unsigned>> symmetries;

  /* vector for symmetry breaking (bits from 0 to ?):
   * 0: commutativity          C
   * 1: inverters              I
   * 2: structural hashing     s
   * 3: associativity          a
   * 4: co-lexicographic order l
   * 5: support                t
   * 6: symmetries             y
   */
  boost::dynamic_bitset<> symmetry_breaking;

public:
  /* some statistics */
  unsigned last_size = 0u; /* the last level that has been tried (helpful when using timeout) */
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

boost::optional<xmg_graph> xmg_exact_with_sat( const tt& spec,
                                               const properties::ptr& settings,
                                               const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  assert( !spec.empty() );

  const auto all_solutions = get( settings, "all_solutions", false );
  xmg_exact_manager mgr( spec_representation( spec ), settings );

  const auto migs = mgr.run();
  if ( all_solutions )
  {
    set( statistics, "all_solutions", migs );
  }
  set( statistics, "last_size", mgr.last_size );

  if ( migs.empty() )
  {
    return boost::none;
  }
  else
  {
    return migs.front();
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
