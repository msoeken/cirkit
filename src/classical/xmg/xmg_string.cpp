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

#include "xmg_string.hpp"

#include <algorithm>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <classical/utils/expression_parser.hpp>
#include <classical/xmg/xmg_expr.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string xmg_to_string_rec( const xmg_graph& xmg, const xmg_function& f )
{
  std::string expr;

  if ( f.complemented )
  {
    expr += "!";
  }

  if ( xmg.is_input( f.node ) )
  {
    if ( f.node == 0u )
    {
      expr += "0";
    }
    else
    {
      expr += ( 'a' + ( boost::find_if( xmg.inputs(), [&f]( const xmg_graph::input_vec_t::value_type& p ) { return p.first == f.node; } ) - xmg.inputs().begin() ) );
    }
  }
  else if ( xmg.is_xor( f.node ) )
  {
    const auto children = xmg.children( f.node );
    expr += boost::str( boost::format( "[%s%s]" ) % xmg_to_string_rec( xmg, children[0] ) % xmg_to_string_rec( xmg, children[1] ) );
  }
  else if ( xmg.is_maj( f.node ) )
  {
    const auto children = xmg.children( f.node );
    expr += boost::str( boost::format( "<%s%s%s>" ) % xmg_to_string_rec( xmg, children[0] ) % xmg_to_string_rec( xmg, children[1] ) % xmg_to_string_rec( xmg, children[2] ) );
  }

  return expr;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::string xmg_to_string( const xmg_graph& xmg, const xmg_function& f )
{
  return xmg_to_string_rec( xmg, f );
}

xmg_function xmg_from_string( xmg_graph& xmg, const std::string& expr, const properties::ptr& settings )
{
  /* settings */
  auto primary_inputs = get( settings, "primary_inputs", std::vector<xmg_function>() );

  return xmg_from_expression( xmg, primary_inputs, parse_expression( expr ) );
}

xmg_graph xmg_from_string( const std::string& expr, const properties::ptr& settings )
{
  /* settings */
  const auto model_name  = get( settings, "model_name",  expr );
  const auto output_name = get( settings, "output_name", std::string( "f" ) );

  xmg_graph xmg( model_name );

  std::vector<xmg_function> pis;
  xmg.create_po( xmg_from_string( xmg, expr, settings ), output_name );

  return xmg;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
