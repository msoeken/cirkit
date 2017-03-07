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

#include "exact_mig.hpp"

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
#include <classical/mig/mig_from_string.hpp>
#include <classical/mig/mig_utils.hpp>
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

#ifdef ADDON_FORMAL
struct exact_mig_instance
{
  struct wire
  {
    wire( exact_mig_instance* inst, unsigned gate_id, unsigned input_id )
      : inst( inst ),
        sel( create_sel( inst, gate_id, input_id ) ),
        neg( create_neg( inst, gate_id, input_id ) )
    {
    }

    int sel_value( const z3::model& m ) const
    {
      int v;
      if ( inst->enc_bv )
      {
        v = to_bitset( m.eval( sel ) ).to_ulong();
      }
      else
      {
        Z3_get_numeral_int( inst->ctx, m.eval( sel ), &v );
      }
      return v;
    }

    bool neg_value( const z3::model& m ) const
    {
      return expr_to_bool( m.eval( neg ) );
    }

    exact_mig_instance* inst;
    z3::expr sel;
    z3::expr neg;

  private:
    z3::expr create_sel( exact_mig_instance* inst, unsigned gate_id, unsigned input_id ) const
    {
      const auto name = str( format( "sel%d_%d" ) % input_id % gate_id );
      if ( inst->enc_bv )
      {
        auto e = inst->ctx.bv_const( name.c_str(), inst->bw );

        /* assertion for sel[level] <= n + k */
        inst->solver.add( z3::ule( e, inst->ctx.bv_val( inst->num_vars + gate_id, inst->bw ) ) );

        return e;
      }
      else
      {
        auto e = inst->ctx.int_const( name.c_str() );

        /* assertion for sel[level] <= n + k */
        inst->solver.add( e >= 0 );
        inst->solver.add( e <= static_cast<int>( inst->num_vars + gate_id ) );

        return e;
      }
    }

    z3::expr create_neg( exact_mig_instance* inst, unsigned gate_id, unsigned input_id ) const
    {
      return inst->ctx.bool_const( str( format( "neg%d_%d" ) % input_id % gate_id ).c_str() );
    }
  };

  struct gate
  {
    gate( exact_mig_instance* inst, unsigned gate_id, bool with_xor )
      : inst( inst),
        gate_id( gate_id ),
        inputs({wire( inst, gate_id, 0u ), wire( inst, gate_id, 1u ), wire( inst, gate_id, 2u )}),
        with_xor( with_xor )
    {
      if ( with_xor )
      {
        /* type = 0 : MAJ, type = 1 : XOR */
        type_expr = std::make_shared<z3::expr>( inst->ctx.bool_const( str( format( "type%d" ) % gate_id ).c_str() ) );

        inst->solver.add( implies( type(), !inputs[2].neg ) );
        inst->solver.add( implies( type(), inst->equals( inputs[2].sel, inst->ctx.bv_val( inst->num_vars + gate_id, inst->bw ) ) ) );
      }
    }

    void symmetry_breaking_commutativity() const
    {
      if ( inst->enc_bv )
      {
        inst->solver.add( z3::ult( inputs[0].sel, inputs[1].sel ) );

        if ( with_xor )
        {
          inst->solver.add( implies( !type(), z3::ult( inputs[1].sel, inputs[2].sel ) ) );
        }
        else
        {
          inst->solver.add( z3::ult( inputs[1].sel, inputs[2].sel ) );
        }
      }
      else
      {
        inst->solver.add( inputs[0].sel < inputs[1].sel );

        if ( with_xor )
        {
          inst->solver.add( implies( !type(), inputs[1].sel < inputs[2].sel ) );
        }
        else
        {
          inst->solver.add( inputs[1].sel < inputs[2].sel );
        }
      }
    }

    void symmetry_breaking_inverters_xor() const
    {
      inst->solver.add( implies( type(), !inputs[0].neg && !inputs[1].neg ) );
    }

    void symmetry_breaking_inverters_maj() const
    {
      auto min_neg = !( ( inputs[0].neg && inputs[1].neg ) || ( inputs[0].neg && inputs[2].neg ) || ( inputs[1].neg && inputs[2].neg ) );
      if ( with_xor )
      {
        inst->solver.add( implies( !type(), min_neg ) );
      }
      else
      {
        inst->solver.add( min_neg );
        //inst->solver.add( !inputs[0].neg || !inputs[1].neg || !inputs[2].neg );
      }
    }

    void symmetry_breaking_colexicographic( const gate& prev )
    {
      if ( with_xor )
      {
        return;
        const auto imp = implies( is_maj(), !inst->equals( inputs[2].sel, inst->num_vars + 1u + prev.gate_id ) ) ||
                         implies( is_xor(), !inst->equals( inputs[1].sel, inst->num_vars + 1u + prev.gate_id ) );
        const auto c1e1 = inputs[0].sel == prev.inputs[0].sel;
        const auto c2e2 = inputs[1].sel == prev.inputs[1].sel;

        if ( inst->enc_bv )
        {
          const auto c1 = z3::ult( prev.inputs[0].sel, inputs[0].sel );
          const auto c2 = c1e1 && z3::ult( prev.inputs[1].sel, inputs[1].sel );
          const auto c3 = is_maj() && prev.is_maj() && c1e1 && c2e2 && z3::ule( prev.inputs[2].sel, inputs[2].sel );

          inst->solver.add( implies( imp, c1 || c2 || c3 ) );
        }
        else
        {
          const auto c1 = prev.inputs[0].sel < inputs[0].sel;
          const auto c2 = c1e1 && prev.inputs[1].sel < inputs[1].sel;
          const auto c3 = is_maj() && prev.is_maj() && c1e1 && c2e2 && prev.inputs[2].sel <= inputs[2].sel;

          inst->solver.add( implies( imp, c1 || c2 || c3 ) );
        }
      }
      else
      {
        const auto imp = !inst->equals( inputs[2].sel, inst->num_vars + 1u + prev.gate_id );
        const auto c1e1 = inputs[0].sel == prev.inputs[0].sel;
        const auto c2e2 = inputs[1].sel == prev.inputs[1].sel;

        if ( inst->enc_bv )
        {
          const auto c1 = z3::ult( prev.inputs[0].sel, inputs[0].sel );
          const auto c2 = c1e1 && z3::ult( prev.inputs[1].sel, inputs[1].sel );
          const auto c3 = c1e1 && c2e2 && z3::ule( prev.inputs[2].sel, inputs[2].sel );

          inst->solver.add( implies( imp, c1 || c2 || c3 ) );
        }
        else
        {
          const auto c1 = prev.inputs[0].sel < inputs[0].sel;
          const auto c2 = c1e1 && prev.inputs[1].sel < inputs[1].sel;
          const auto c3 = c1e1 && c2e2 && prev.inputs[2].sel <= inputs[2].sel;

          inst->solver.add( implies( imp, c1 || c2 || c3 ) );
        }
      }
    }

    void symmetry_breaking_different( const gate& other )
    {
      if ( with_xor )
      {
        inst->solver.add( implies( !type() && !other.type(),
                                   inputs[0].sel != other.inputs[0].sel ||
                                   inputs[1].sel != other.inputs[1].sel ||
                                   inputs[2].sel != other.inputs[2].sel ||
                                   inputs[0].neg != other.inputs[0].neg ||
                                   inputs[1].neg != other.inputs[1].neg ||
                                   inputs[2].neg != other.inputs[2].neg ) );
        inst->solver.add( implies( type() && other.type(),
                                   inputs[0].sel != other.inputs[0].sel ||
                                   inputs[1].sel != other.inputs[1].sel ||
                                   inputs[0].neg != other.inputs[0].neg ||
                                   inputs[1].neg != other.inputs[1].neg ) );
      }
      else
      {
        inst->solver.add( inputs[0].sel != other.inputs[0].sel ||
                          inputs[1].sel != other.inputs[1].sel ||
                          inputs[2].sel != other.inputs[2].sel ||
                          inputs[0].neg != other.inputs[0].neg ||
                          inputs[1].neg != other.inputs[1].neg ||
                          inputs[2].neg != other.inputs[2].neg );
      }
    }

    void symmetry_breaking_constant_input() const
    {
      inst->solver.add( !( type() && inst->equals( inputs[0].sel, 0u ) ) );
    }

    void symmetry_breaking_associativity( const gate& other ) const
    {
      if ( with_xor ) { return; }

      for ( auto alpha = 0u; alpha < 3u; ++alpha )
      {
        for ( auto beta = 0u; beta < 3u; ++beta )
        {
          if ( alpha == beta ) continue;
          for ( auto gamma = 0u; gamma < 3u; ++gamma )
          {
            const auto is_child = inst->equals( inputs[beta].sel, inst->num_vars + 1u + other.gate_id );
            const auto has_same_child = ( inputs[alpha].sel == other.inputs[gamma].sel );
            const auto has_same_pol = ( inputs[alpha].neg == other.inputs[gamma].neg );
            const auto has_same = has_same_child && has_same_pol;

            const auto p_child = inputs[3 - alpha - beta].sel;
            const auto c_child_id = gamma == 2u ? 1u : 2u;
            const auto c_child = other.inputs[c_child_id].sel;

            if ( inst->enc_bv )
            {
              inst->solver.add( implies( is_child && has_same, z3::ule( c_child, p_child ) ) );
            }
            else
            {
              inst->solver.add( implies( is_child && has_same, c_child <= p_child ) );
            }
            //inst->solver.add( implies( is_child && has_same_child/* && ( p_child != c_child ) && ( p_child != other.inputs[3 - gamma - c_child_id].sel )*/, !has_same_pol ) );

            //inst->solver.add( implies( is_child && has_same_child, p_child != c_child && p_child != other.inputs[3 - gamma - c_child_id].sel ) );
          }
        }
      }

      // for ( auto i = 0u; i < 3u; ++i )
      // {
      //   for ( auto j = 0u; j < 3u; ++j )
      //   {
      //     for ( auto k = 0ul; k < 3u; ++k )
      //     {
      //       if ( i == k ) { continue; }

      //       const auto is_child = inst->equals( inputs[i].sel, inst->num_vars + 1u + other.gate_id );
      //       const auto has_same = ( inputs[i].sel == other.inputs[j].sel ) && ( inputs[i].neg == other.inputs[j].neg );

      //       if ( inst->enc_bv )
      //       {
      //         inst->solver.add( implies( is_child && has_same, z3::ule( inputs[3 - i - k].sel, other.inputs[ j == 0 ? 1 : 0 ].sel ) ) );
      //       }
      //       else
      //       {
      //         inst->solver.add( implies( is_child && has_same, inputs[3 - i - k].sel <= other.inputs[ j == 0 ? 1 : 0 ].sel ) );
      //       }
      //     }
      //   }
    }

    void block_for_support( const boost::dynamic_bitset<>& support )
    {
      foreach_bit( ~support, [this, &support]( unsigned pos ) {
          inst->solver.add( !( inst->equals( inputs[0].sel, pos + 1u ) ) );
          inst->solver.add( !( inst->equals( inputs[1].sel, pos + 1u ) ) );
          if ( with_xor )
          {
            inst->solver.add( implies( !type(), !( inst->equals( inputs[2].sel, pos + 1u ) ) ) );
          }
        } );
    }

    //void block_for_symmetries( const std::vector<std::pair<unsigned, unsigned>>& symmetries )
    //{
    //}

    const wire& operator[]( unsigned index ) const
    {
      return inputs[index];
    }

    inline const z3::expr& type() const
    {
      return *type_expr;
    }

    inline z3::expr is_maj() const
    {
      return !*type_expr;
    }

    inline z3::expr is_xor() const
    {
      return *type_expr;
    }

    bool type_value( const z3::model& m ) const
    {
      return expr_to_bool( m.eval( type() ) );
    }

  private:
    exact_mig_instance* inst;
    unsigned gate_id;

  public:
    std::vector<wire> inputs;
    bool with_xor;
    std::shared_ptr<z3::expr> type_expr;
  };

  /**
   * @param timeout given in seconds
   */
  exact_mig_instance( unsigned num_vars, bool with_xor, bool enc_bv, bool expl, boost::optional<unsigned> timeout = boost::none ) :
    num_vars( num_vars ),
    with_xor( with_xor ),
    enc_bv( enc_bv ),
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

    gates += gate( this, level, with_xor );

    /* symmetry breaking */
    if ( symmetry_breaking[0u] )
    {
      gates[level].symmetry_breaking_commutativity();
    }

    if ( symmetry_breaking[1u] )
    {
      gates[level].symmetry_breaking_inverters_maj();

      if ( with_xor )
      {
        gates[level].symmetry_breaking_inverters_xor();
      }
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

    if ( with_xor )
    {
      gates[level].symmetry_breaking_constant_input();
    }

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
      if ( with_xor )
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
      else
      {
        for ( const auto& p : symmetries )
        {
          for ( auto c = 0u; c < 3u; ++c )
          {
            if ( level == 0u && c == 0u ) { continue; }

            z3::expr A = equals( gates[level].inputs[c].sel, p.first + 1 );

            z3::expr B = ctx.bool_val( true );

            for ( auto j = 0u; j < level; ++j )
            {
              for ( auto d = 0u; d < 3u; ++d )
              {
                A = A && !equals( gates[j].inputs[d].sel, p.first + 1 );
                B = B && !equals( gates[j].inputs[d].sel, p.second + 1 );
              }
            }
            for ( auto d = 0u; d < c; ++d )
            {
              A = A && !equals( gates[level].inputs[d].sel, p.first + 1 );
              B = B && !equals( gates[level].inputs[d].sel, p.second + 1 );
            }

            solver.add( implies( A, B ) );
          }
        }
      }
    }
  }

  void constrain( const mig_graph& mig )
  {
    z3::params p( ctx );
    p.set( "mbqi", true );
    solver.set( p );

    const auto& info = mig_info( mig );
    assert( info.inputs.size() == num_vars );
    assert( info.outputs.size() == 1u );

    /* for later */
    z3::expr_vector all_vars( ctx );

    /* value constraints */
    std::vector<z3::expr> out;
    std::vector<std::vector<z3::expr>> in( 3u );

    std::vector<z3::expr> input;

    for ( auto i = 0u; i < num_vars; ++i )
    {
      input += ctx.bool_const( str( format( "x%d" ) % i ).c_str() );
      all_vars.push_back( input.back() );
    }

    //auto constraint = ctx.bool_val( true );

    for ( auto level = 0u; level < gates.size(); ++level )
    {
      /* assertions for in[x][j][level] = neg[level] ^ ite( sel[level], ... ) */
      for ( auto x = 0u; x < 3u; ++x )
      {
        /* constant */
        auto expr = implies( equals( gates[level][x].sel, 0 ), gates[level][x].neg );

        /* inputs */
        for ( auto l = 0u; l < num_vars; ++l )
        {
          expr = expr && implies( equals( gates[level][x].sel, l + 1 ), logic_xor( gates[level][x].neg, input[l] ) );
        }

        /* gates */
        for ( auto l = 0u; l < level; ++l )
        {
          expr = expr && implies( equals( gates[level][x].sel, l + 1 + num_vars ), logic_xor( gates[level][x].neg, out[l] ) );
        }

        in[x] += expr;
      }

      if ( with_xor )
      {
        out += implies( !gates[level].type(), ( in[0u][level] && in[1u][level] ) || ( in[0u][level] && in[2u][level] ) || ( in[1u][level] && in[2u][level] ) )
          && implies( gates[level].type(), logic_xor( in[0u][level], in[1u][level] ) );
      }
      else
      {
        out += ( in[0u][level] && in[1u][level] ) || ( in[0u][level] && in[2u][level] ) || ( in[1u][level] && in[2u][level] );
      }
    }

    /* constrain mig */
    std::vector<boost::optional<z3::expr>> node_to_expr( num_vertices( mig ) );
    node_to_expr[0u] = ctx.bool_val( false );
    for ( auto i : index( info.inputs ) )
    {
      node_to_expr[i.value] = input[i.index];
    }

    std::vector<mig_node> topsort( num_vertices( mig ) );
    boost::topological_sort( mig, topsort.begin() );

    for ( auto node : topsort )
    {
      if ( out_degree( node, mig ) == 0u ) continue;

      const auto children = get_children( mig, node );

      const z3::expr c1 = logic_xor( *node_to_expr[children[0].node], ctx.bool_val( children[0].complemented ) );
      const z3::expr c2 = logic_xor( *node_to_expr[children[1].node], ctx.bool_val( children[1].complemented ) );
      const z3::expr c3 = logic_xor( *node_to_expr[children[2].node], ctx.bool_val( children[2].complemented ) );

      node_to_expr[node] = ( c1 && c2 ) || ( c1 && c3 ) || ( c2 && c3 );
    }

    /* join constraints */
    const auto o = info.outputs[0u].first;
    const auto out1 = out.back();
    const auto out2 = logic_xor( *node_to_expr[o.node], ctx.bool_val( o.complemented ) );

    /* build everything */
    const auto forall_expr = forall( all_vars, out1 == out2 );
    solver.add( forall_expr );
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

  mig_graph extract_mig( const std::string& model_name, const std::string& output_name, bool invert, bool very_verbose )
  {
    mig_graph mig;
    mig_initialize( mig, model_name );

    std::vector<mig_function> inputs, nodes;
    for ( auto i = 0u; i < num_vars; ++i )
    {
      inputs += mig_create_pi( mig, str( format( "x%d" ) % i ) );
    }

    const auto m = solver.get_model();

    if ( very_verbose )
    {
      std::cout << m << std::endl;
    }

    for ( auto i = 0u; i < gates.size(); ++i )
    {
      mig_function children[3];

      for ( auto x = 0u; x < 3u; ++x )
      {
        auto sel_val = gates[i][x].sel_value( m );

        if ( sel_val == 0 )
        {
          children[x] = mig_get_constant( mig, false );
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

      nodes += mig_create_maj( mig, children[0], children[1], children[2] );

      if ( very_verbose )
      {
        std::cout << format( "added node (%d,%d) for i = %d" ) % nodes.back().node % nodes.back().complemented % i << std::endl;
        for ( auto x = 0u; x < 3u; ++x )
        {
          std::cout << format( "  - child %d = (%d,%d)" ) % x % children[x].node % children[x].complemented << std::endl;
        }
      }
    }

    auto f = nodes.back() ^ invert;
    mig_create_po( mig, f, output_name );

    return mig;
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

  void add_depth_constraints( int max_depth )
  {
    const auto num_gates = gates.size();

    /* depth variables d[x][i], x = 0,1,2,3, i = 0,1,2,...,num_gates - 1 */
    std::vector<std::vector<z3::expr>> dvars( 4u );

    for ( auto i = 0u; i < num_gates; ++i )
    {
      /* create variables, it's safe because of topological order */
      for ( auto x = 0u; x < 4u; ++x )
      {
        if ( enc_bv )
        {
          dvars[x] += ctx.bv_const( str( format( "dvars%d_%d" ) % x % i ).c_str(), bw );
        }
        else
        {
          dvars[x] += ctx.int_const( str( format( "dvars%d_%d" ) % x % i ).c_str() );
        }
      }

      /* depth of output is maximum of input depths + 1 */
      const auto& a = dvars[0u][i];
      const auto& b = dvars[1u][i];
      const auto& c = dvars[2u][i];

      //solver.add( dvars[3u][i] == ite( a > b, ite( a > c, a, c ), ite( b > c, b, c ) ) + 1 );

      solver.add( dvars[3u][i] == ite( greater_than( a, b ),
                                       ite( greater_than( a, c ), a, c ),
                                       ite( greater_than( b, c ), b, c ) ) + 1 );

      /* depth of inputs */
      for ( auto x = 0u; x < 3u; ++x )
      {
        solver.add( implies( less_equals( gates[i][x].sel, num_vars ),
                             equals( dvars[x][i], 0 ) ) );

        for ( auto l = 0u; l < i; ++l )
        {
          solver.add( implies( equals( gates[i][x].sel, l + num_vars + 1 ),
                               dvars[x][i] == dvars[3u][l] ) );
        }
      }
    }

    solver.add( less_equals( dvars[3u][num_gates - 1u], max_depth ) );
  }

  /* some helper methods */
  z3::expr equals( const z3::expr& expr, unsigned value )
  {
    if ( enc_bv )
    {
      return ( expr == ctx.bv_val( value, bw ) );
    }
    else
    {
      return ( expr == static_cast<int>( value ) );
    }
  }

  z3::expr less_equals( const z3::expr& expr, unsigned value )
  {
    if ( enc_bv )
    {
      return ( z3::ule( expr, ctx.bv_val( value, bw ) ) );
    }
    else
    {
      return ( expr <= static_cast<int>( value ) );
    }
  }

  z3::expr less_than( const z3::expr& expr1, const z3::expr& expr2 )
  {
    if ( enc_bv )
    {
      return z3::ult( expr1, expr2 );
    }
    else
    {
      return expr1 < expr2;
    }
  }

  z3::expr greater_than( const z3::expr& expr1, const z3::expr& expr2 )
  {
    if ( enc_bv )
    {
      return z3::ugt( expr1, expr2 );
    }
    else
    {
      return expr1 > expr2;
    }
  }

  unsigned num_vars;
  bool with_xor;
  bool enc_bv;
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

template<typename T>
class exact_mig_manager
{
public:
  exact_mig_manager( const spec_representation& spec, const properties::ptr& settings )
    : spec( spec ),
      normal( spec.is_normal() )
  {
    /* meta data */
    model_name          = get( settings, "model_name",          std::string( "exact" ) );
    output_name         = get( settings, "output_name",         std::string( "f" ) );

    /* control algorithm */
    objective           = get( settings, "objective",           0u );   /* 0: size, 1: size/depth, 2: depth/size */
    start               = get( settings, "start",               1u );
    start_depth         = get( settings, "start_depth",         1u );
    incremental         = get( settings, "incremental",         false );
    all_solutions       = get( settings, "all_solutions",       false );

    /* encoding */
    breaking            = get( settings, "breaking",            std::string( "CIsalty" ) );
    enc_with_bitvectors = get( settings, "enc_with_bitvectors", true );

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

  std::vector<T> run()
  {
    /* check trivial case */
    const auto triv = spec.is_trivial();
    if ( (bool)triv )
    {
      return {create_trivial<T>( triv->first, triv->second )};
    }

    if ( !normal )
    {
      spec.invert();
    }

    if ( incremental )
    {
      return exact_mig_size_incremental();
    }
    else
    {
      switch ( objective )
      {
      case 0u:
        return exact_mig_size_explicit();
      case 1u:
        return exact_mig_size_depth_explicit();
      case 2u:
        return exact_mig_depth_size_explicit();
      default:
        assert( false );
        return std::vector<T>();
      }
    }
  }

  std::vector<T> exact_mig_size_explicit()
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
        store_memory( inst );
        return extract_solutions( inst );
      }
      else if ( result == z3::unknown && !timeout_heuristic )
      {
        last_size = k;
        return std::vector<T>();
      }

      ++k;
    }
  }

  std::vector<T> exact_mig_size_depth_explicit()
  {
    auto k = start;
    auto d = 0; /* d = 0 means still looking for best size (depth-0 migs are found before this function is called) */

    while ( true )
    {
      if ( verbose )
      {
        if ( d )
        {
          std::cout << boost::format( "[i] check for realization with %d gates and depth %d" ) % k % d << std::endl;
        }
        else
        {
          std::cout << boost::format( "[i] check for realization with %d gates" ) % k << std::endl;
        }
      }

      if ( d )
      {
        symmetry_breaking.reset( 3u ); /* don't use associativity symmetry breaking */
      }

      const auto inst = create_instance();

      for ( auto i = 0u; i < k; ++i )
      {
        inst->add_level( symmetry_breaking );
      }

      constrain( inst );

      if ( d ) /* find best depth */
      {
        inst->add_depth_constraints( d );
        const auto result = inst->solver.check();
        if ( result == z3::sat )
        {
          store_memory( inst );
          return extract_solutions( inst );
        }
        else if ( result == z3::unknown && !timeout_heuristic )
        {
          last_size = k;
          return std::vector<T>();
        }

        ++d;
      }
      else
      {
        const auto result = inst->solver.check();
        if ( result == z3::sat )
        {
          d = start_depth;
        }
        else if ( result == z3::unknown && !timeout_heuristic )
        {
          return std::vector<T>();
        }
        else
        {
          ++k;
        }
      }
    }
  }

  std::vector<T> exact_mig_depth_size_explicit()
  {
    symmetry_breaking.reset( 3u );
    auto d = start_depth;

    while ( true )
    {
      const auto max_gates = ( static_cast<int>( pow( 3, d ) ) - 1 ) / 2;

      for ( int k = ( d == start_depth ) ? start : 1; k <= max_gates; ++k )
      {
        if ( verbose )
        {
          std::cout << boost::format( "[i] check for realization with depth %d and %d gates" ) % d % k << std::endl;
        }

        const auto inst = create_instance();

        for ( auto i = 0; i < k; ++i )
        {
          inst->add_level( symmetry_breaking );
        }

        constrain( inst );
        inst->add_depth_constraints( d );

        const auto result = inst->solver.check();
        if ( result == z3::sat )
        {
          store_memory( inst );
          return extract_solutions( inst );
        }
        else if ( result == z3::unknown && !timeout_heuristic )
        {
          return std::vector<T>();
        }
      }

      ++d;
    }
  }

  std::vector<T> exact_mig_size_incremental()
  {
    const auto inst = create_instance();

    for ( unsigned i = 1u; i < start; ++i )
    {
      inst->add_level( symmetry_breaking );
    }

    while ( true )
    {
      inst->add_level( symmetry_breaking );

      if ( verbose )
      {
        std::cout << boost::format( "[i] check for realization with %d gates" ) % inst->gates.size() << std::endl;
      }

      /* solve */
      inst->solver.push();
      constrain( inst );
      const auto result = inst->solver.check();
      if ( result == z3::sat )
      {
        store_memory( inst );
        return extract_solutions( inst );
      }
      else if ( result == z3::unknown && !timeout_heuristic )
      {
        last_size = inst->gates.size();
        return std::vector<T>();
      }
      inst->solver.pop();
    }
  }

private:
  template<typename C, typename std::enable_if<std::is_same<mig_graph, C>::value>::type* = nullptr>
  bool with_xor() const
  {
    return false;
  }

  template<typename C, typename std::enable_if<std::is_same<xmg_graph, C>::value>::type* = nullptr>
  bool with_xor() const
  {
    return true;
  }

  template<typename C, typename std::enable_if<std::is_same<mig_graph, C>::value>::type* = nullptr>
  mig_graph create_trivial( unsigned id, bool complement )
  {
    mig_graph mig;
    mig_initialize( mig, model_name );

    if ( id == 0u )
    {
      mig_create_po( mig, mig_get_constant( mig, complement ), output_name );
    }
    else
    {
      const auto& info = mig_info( mig );

      for ( auto i = 0u; i < id; ++i )
      {
        mig_create_pi( mig, str( format( "x%d" ) % i ) );
      }
      mig_create_po( mig, {info.inputs.back(), complement}, output_name );
    }
    return mig;
  }

  template<typename C, typename std::enable_if<std::is_same<xmg_graph, C>::value>::type* = nullptr>
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

  template<typename C, typename std::enable_if<std::is_same<mig_graph, C>::value>::type* = nullptr>
  mig_graph extract_solution( const std::shared_ptr<exact_mig_instance>& inst ) const
  {
    return inst->extract_mig( model_name, output_name, !normal, very_verbose );
  }

  template<typename C, typename std::enable_if<std::is_same<xmg_graph, C>::value>::type* = nullptr>
  xmg_graph extract_solution( const std::shared_ptr<exact_mig_instance>& inst ) const
  {
    return inst->extract_xmg( model_name, output_name, !normal, very_verbose );
  }

  inline std::shared_ptr<exact_mig_instance> create_instance() const
  {
    auto inst = std::make_shared<exact_mig_instance>( spec.num_vars(), with_xor<T>(), enc_with_bitvectors, spec.is_explicit(), timeout );
    inst->support    = support;
    inst->symmetries = symmetries;
    return inst;
  }

  std::vector<T> extract_solutions( const std::shared_ptr<exact_mig_instance>& inst ) const
  {
    if ( all_solutions )
    {
      std::vector<T> migs;

      do
      {
        migs.push_back( extract_solution<T>( inst ) );
        inst->block_solution();
      } while ( inst->solver.check() == z3::sat );

      return migs;
    }
    else
    {
      return {extract_solution<T>( inst )};
    }
  }

  struct constrain_visitor : public boost::static_visitor<void>
  {
    constrain_visitor( const std::shared_ptr<exact_mig_instance>& inst ) : inst( inst ) {}

    void operator()( const tt& spec ) const
    {
      inst->constrain( spec );
    }

    void operator()( const mig_graph& spec ) const
    {
      inst->constrain( spec );
    }

  private:
    const std::shared_ptr<exact_mig_instance>& inst;
  };

  void constrain( const std::shared_ptr<exact_mig_instance>& inst ) const
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

  void store_memory( const std::shared_ptr<exact_mig_instance>& inst )
  {
    const auto stats = inst->solver.statistics();

    for ( auto i = 0u; i < stats.size(); ++i )
    {
      if ( stats.key( i ) == "memory" )
      {
        memory = stats.double_value( i );
        break;
      }
    }
  }

private:
  spec_representation spec;
  bool normal;
  unsigned start;
  unsigned start_depth;
  std::string model_name;
  std::string output_name;
  unsigned objective;
  bool incremental;
  bool all_solutions;
  std::string breaking;
  bool enc_with_bitvectors;
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
  double   memory    = -1.0; /* memory usage */
};
#endif

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

boost::optional<mig_graph> exact_mig_with_sat( const tt& spec,
                                               const properties::ptr& settings,
                                               const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  assert( !spec.empty() );

#ifdef ADDON_FORMAL
  const auto all_solutions = get( settings, "all_solutions", false );
  exact_mig_manager<mig_graph> mgr( spec_representation( spec ), settings );

  const auto migs = mgr.run();
  if ( all_solutions )
  {
    set( statistics, "all_solutions", migs );
  }
  set( statistics, "last_size", mgr.last_size );
  set( statistics, "memory", mgr.memory );

  if ( migs.empty() )
  {
    return boost::none;
  }
  else
  {
    return migs.front();
  }

#else
  std::cout << "[e] z3 solver requires formal addon enabled" << std::endl;
  return boost::none;
#endif
}

boost::optional<mig_graph> exact_mig_with_sat( const mig_graph& spec,
                                               const properties::ptr& settings,
                                               const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

#ifdef ADDON_FORMAL
  const auto all_solutions = get( settings, "all_solutions", false );

  exact_mig_manager<mig_graph> mgr( spec_representation( spec ), settings );

  const auto migs = mgr.run();
  if ( all_solutions )
  {
    set( statistics, "all_solutions", migs );
  }
  set( statistics, "last_size", mgr.last_size );
  set( statistics, "memory", mgr.memory );

  if ( migs.empty() )
  {
    return boost::none;
  }
  else
  {
    return migs.front();
  }

#else
  std::cout << "[e] z3 solver requires formal addon enabled" << std::endl;
  return boost::none;
#endif
}

boost::optional<xmg_graph> exact_xmg_with_sat( const tt& spec,
                                               const properties::ptr& settings,
                                               const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  assert( !spec.empty() );

#ifdef ADDON_FORMAL
  const auto all_solutions = get( settings, "all_solutions", false );

  exact_mig_manager<xmg_graph> mgr( spec_representation( spec ), settings );

  const auto xmgs = mgr.run();
  if ( all_solutions )
  {
    set( statistics, "all_solutions", xmgs );
  }
  set( statistics, "last_size", mgr.last_size );
  set( statistics, "memory", mgr.memory );

  if ( xmgs.empty() )
  {
    return boost::none;
  }
  else
  {
    return xmgs.front();
  }

#else
  std::cout << "[e] z3 solver requires formal addon enabled" << std::endl;
  return boost::none;
#endif
}

boost::optional<xmg_graph> exact_xmg_with_sat( const mig_graph& spec,
                                               const properties::ptr& settings,
                                               const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

#ifdef ADDON_FORMAL
  const auto all_solutions = get( settings, "all_solutions", false );

  exact_mig_manager<xmg_graph> mgr( spec_representation( spec ), settings );

  const auto xmgs = mgr.run();
  if ( all_solutions )
  {
    set( statistics, "all_solutions", xmgs );
  }
  set( statistics, "last_size", mgr.last_size );
  set( statistics, "memory", mgr.memory );

  if ( xmgs.empty() )
  {
    return boost::none;
  }
  else
  {
    return xmgs.front();
  }

#else
  std::cout << "[e] z3 solver requires formal addon enabled" << std::endl;
  return boost::none;
#endif
}

boost::optional<mig_graph> exact_mig_with_bdds( const tt& spec,
                                                const properties::ptr& settings,
                                                const properties::ptr& statistics )
{
  properties_timer t( statistics );

  assert( false && "not yet implemented" );

  return boost::none;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
