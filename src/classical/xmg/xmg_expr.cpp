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

#include "xmg_expr.hpp"

#include <boost/format.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

expression_t::ptr xmg_to_expression( const xmg_graph& xmg, const xmg_function& f )
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
    expr->children.push_back( xmg_to_expression( xmg, !f ) );
  }
  else if ( xmg.is_maj( f.node ) )
  {
    expr->type = expression_t::_maj;
    for ( const auto& c : xmg.children( f.node ) )
    {
      expr->children.push_back( xmg_to_expression( xmg, c ) );
    }
  }
  else if ( xmg.is_xor( f.node ) )
  {
    expr->type = expression_t::_xor;
    for ( const auto& c : xmg.children( f.node ) )
    {
      expr->children.push_back( xmg_to_expression( xmg, c ) );
    }
  }
  else
  {
    expr->type = expression_t::_var;
    expr->value = xmg.input_index( f.node );
  }

  return expr;
}

xmg_function xmg_from_expression( xmg_graph& xmg, std::vector<xmg_function>& pis, const expression_t::ptr& expr )
{
  switch ( expr->type )
  {
  case expression_t::_const:
    return xmg.get_constant( expr->value == 1u );

  case expression_t::_var:
    {
      const auto idx = expr->value;
      if ( idx >= pis.size() )
      {
        for ( auto i = pis.size(); i <= idx; ++i )
        {
          pis.push_back( xmg.create_pi( boost::str( boost::format( "x%d" ) % i )  ) );
        }
      }
      return pis[idx];
    }

  case expression_t::_inv:
    return !xmg_from_expression( xmg, pis, expr->children.front() );

  case expression_t::_and:
    {
      auto it = expr->children.begin();
      const auto a = xmg_from_expression( xmg, pis, *it++ );
      const auto b = xmg_from_expression( xmg, pis, *it );

      return xmg.create_and( a, b );
    }

  case expression_t::_or:
    {
      auto it = expr->children.begin();
      const auto a = xmg_from_expression( xmg, pis, *it++ );
      const auto b = xmg_from_expression( xmg, pis, *it );

      return xmg.create_or( a, b );
    }

  case expression_t::_maj:
    {
      auto it = expr->children.begin();
      const auto a = xmg_from_expression( xmg, pis, *it++ );
      const auto b = xmg_from_expression( xmg, pis, *it++ );
      const auto c = xmg_from_expression( xmg, pis, *it );

      return xmg.create_maj( a, b, c );
    }

  case expression_t::_xor:
    {
      auto it = expr->children.begin();
      const auto a = xmg_from_expression( xmg, pis, *it++ );
      const auto b = xmg_from_expression( xmg, pis, *it );

      return xmg.create_xor( a, b );
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
