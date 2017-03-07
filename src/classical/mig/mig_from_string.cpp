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

#include "mig_from_string.hpp"

#include <map>
#include <stack>

#include <boost/algorithm/string/replace.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/range_utils.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using bracket_map_t = std::map<unsigned, unsigned>;
using input_map_t = std::map<char, mig_function>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

bracket_map_t find_bracket_pairs( const std::string& s, char open_bracket, char closed_bracket )
{
  std::stack<unsigned> open;
  bracket_map_t pairs;

  for ( const auto& c : index( s ) )
  {
    if ( c.value == open_bracket )
    {
      open.push( c.index );
    }
    else if ( c.value == closed_bracket )
    {
      pairs.insert( {open.top(), c.index} );
      open.pop();
    }
  }

  assert( open.empty() );

  return pairs;
}

mig_function function_from_string( mig_graph& mig, const std::string& expr, unsigned offset,
                                   const bracket_map_t& brackets, input_map_t& inputs )
{
  assert( expr[offset] != '!' );

  if ( expr[offset] == '<' )
  {
    mig_function fs[3];

    auto child_pos = offset + 1u;
    auto to = 0u;
    auto inv = false;

    for ( auto i = 0u; i < 3u; ++i )
    {
      child_pos += to;
      inv = ( expr[child_pos] == '!' );
      if ( inv ) { ++child_pos; }

      to = ( expr[child_pos] == '<' ) ? brackets.at( child_pos ) - child_pos + 1u : 1u;

      fs[i] = function_from_string( mig, expr, child_pos, brackets, inputs );
      if ( inv ) { fs[i] = !fs[i]; }
    }

    return mig_create_maj( mig, fs[0], fs[1], fs[2] );
  }
  else if ( expr[offset] == '0' || expr[offset] == '1' )
  {
    return mig_get_constant( mig, expr[offset] == '1' );
  }
  else
  {
    const auto it = inputs.find( expr[offset] );
    if ( it == inputs.end() )
    {
      auto f = mig_create_pi( mig, expr.substr( offset, 1u ) );
      inputs.insert( {expr[offset], f} );
      return f;
    }
    else
    {
      return it->second;
    }
  }
}

std::string mig_to_string_rec( const mig_graph& mig, const mig_graph_info& info, const mig_function& f )
{
  std::string expr;

  if ( f.complemented )
  {
    expr += "!";
  }

  if ( boost::out_degree( f.node, mig ) == 0u )
  {
    if ( f.node == info.constant )
    {
      expr += "0";
    }
    else
    {
      expr += ( 'a' + std::distance( info.inputs.begin(), boost::find( info.inputs, f.node ) ) );
    }
  }
  else
  {
    auto complemented = boost::get( boost::edge_complement, mig );

    auto it = boost::out_edges( f.node, mig ).first;
    auto expra = mig_to_string_rec( mig, info, {boost::target( *it, mig ), complemented[*it]} ); ++it;
    auto exprb = mig_to_string_rec( mig, info, {boost::target( *it, mig ), complemented[*it]} ); ++it;
    auto exprc = mig_to_string_rec( mig, info, {boost::target( *it, mig ), complemented[*it]} );
    expr += "<" + expra + exprb + exprc + ">";
  }

  return expr;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_graph mig_from_string( const std::string& expr,
                           const properties::ptr& settings,
                           const properties::ptr& statistics )
{
  /* settings */
  const auto model_name   = get( settings, "model_name",   expr );
  const auto output_name  = get( settings, "output_name",  std::string( "f" ) );

  mig_graph mig;
  mig_initialize( mig, model_name );

  mig_create_po( mig, mig_from_string( mig, expr, settings, statistics ), output_name );

  return mig;
}

mig_function mig_from_string( mig_graph& mig, const std::string& expr,
                              const properties::ptr& settings,
                              const properties::ptr& statistics )
{
  /* settings */
  auto variable_map = get( settings, "variable_map", input_map_t() );

  auto nexpr = boost::replace_all_copy( expr, "!!", "" );

  const auto brackets = find_bracket_pairs( nexpr, '<', '>' );

  mig_function f;

  if ( nexpr[0] == '!' )
  {
    f = !function_from_string( mig, nexpr, 1u, brackets, variable_map );
  }
  else
  {
    f = function_from_string( mig, nexpr, 0u, brackets, variable_map );
  }

  return f;
}

std::string mig_to_string( const mig_graph& mig, const mig_function& f,
                           const properties::ptr& settings,
                           const properties::ptr& statistics )
{
  const auto& info = mig_info( mig );

  return mig_to_string_rec( mig, info, f );
}

expression_t::ptr mig_to_expression( const mig_graph& mig, const mig_function& f )
{
  auto expr = std::make_shared<expression_t>();

  if ( f.node == 0 )
  {
    expr->type = expression_t::_const;
    expr->value = f.complemented ? 1u : 0u;
  }
  else if ( f.complemented )
  {
    expr->type = expression_t::_inv;
    expr->children.push_back( mig_to_expression( mig, !f ) );
  }
  else if ( out_degree( f.node, mig ) > 0u )
  {
    expr->type = expression_t::_maj;
    for ( const auto& c : get_children( mig, f.node ) )
    {
      expr->children.push_back( mig_to_expression( mig, c ) );
    }
  }
  else
  {
    const auto& info = mig_info( mig );
    expr->type = expression_t::_var;
    expr->value = std::distance( info.inputs.begin(), boost::find( info.inputs, f.node ) );
  }

  return expr;
}

mig_function mig_from_expression( mig_graph& mig, std::vector<mig_function>& pis, const expression_t::ptr& expr )
{
  switch ( expr->type )
  {
  case expression_t::_const:
    return mig_get_constant( mig, expr->value == 1u );

  case expression_t::_var:
    {
      const auto idx = expr->value;
      if ( idx >= pis.size() )
      {
        for ( auto i = pis.size(); i <= idx; ++i )
        {
          pis.push_back( mig_create_pi( mig, boost::str( boost::format( "x%d" ) % i )  ) );
        }
      }
      return pis[idx];
    }

  case expression_t::_inv:
    return !mig_from_expression( mig, pis, expr->children.front() );

  case expression_t::_and:
    {
      auto it = expr->children.begin();
      const auto a = mig_from_expression( mig, pis, *it++ );
      const auto b = mig_from_expression( mig, pis, *it );

      return mig_create_and( mig, a, b );
    }

  case expression_t::_or:
    {
      auto it = expr->children.begin();
      const auto a = mig_from_expression( mig, pis, *it++ );
      const auto b = mig_from_expression( mig, pis, *it );

      return mig_create_or( mig, a, b );
    }

  case expression_t::_maj:
    {
      auto it = expr->children.begin();
      const auto a = mig_from_expression( mig, pis, *it++ );
      const auto b = mig_from_expression( mig, pis, *it++ );
      const auto c = mig_from_expression( mig, pis, *it );

      return mig_create_maj( mig, a, b, c );
    }

  case expression_t::_xor:
    {
      auto it = expr->children.begin();
      const auto a = mig_from_expression( mig, pis, *it++ );
      const auto b = mig_from_expression( mig, pis, *it );

      return mig_create_xor( mig, a, b );
    }

  default:
    assert( false );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
