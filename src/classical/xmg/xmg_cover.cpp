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

#include "xmg_cover.hpp"

#include <fstream>
#include <iostream>

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>

#include <classical/xmg/xmg_bitmarks.hpp>

namespace cirkit
{

/******************************************************************************
 * xmg_cover                                                                  *
 ******************************************************************************/

xmg_cover::xmg_cover( unsigned cut_size, const xmg_graph& xmg )
  : _cut_size( cut_size ),
    offset( xmg.size(), 0u ),
    leafs( 1u )
{
}

void xmg_cover::add_cut( xmg_node n, const xmg_cuts_paged::cut& cut )
{
  assert( offset[n] == 0u );

  offset[n] = leafs.size();
  leafs.push_back( cut.size() );

  for ( auto l : cut )
  {
    leafs.push_back( l );
  }

  ++count;
}

bool xmg_cover::has_cut( xmg_node n ) const
{
  return offset[n] != 0u;
}

xmg_cover::index_range xmg_cover::cut( xmg_node n ) const
{
  return boost::make_iterator_range( leafs.begin() + offset[n] + 1u,
                                     leafs.begin() + offset[n] + 1u + leafs[offset[n]] );
}

/******************************************************************************
 * private functions                                                          *
 ******************************************************************************/

void xmg_cover_write_dot_rec( const xmg_graph& xmg, std::ostream& os, xmg_node n, boost::dynamic_bitset<>& visited )
{
  if ( visited[n] || xmg.is_input( n ) )
  {
    return;
  }

  visited.set( n );

  os << boost::format( "%d[shape=box,fillcolor=%s,style=filled];" ) % n % ( xmg.bitmarks().is_marked( n ) ? "red" : "lightgray" ) << std::endl;

  for ( auto child : xmg.cover().cut( n ) )
  {
    xmg_cover_write_dot_rec( xmg, os, child, visited );
    os << boost::format( "%d->%d;" ) % n % child << std::endl;
  }
}

/******************************************************************************
 * public functions                                                           *
 ******************************************************************************/

void xmg_cover_write_dot( const xmg_graph& xmg, std::ostream& os )
{
  using boost::format;

  os << "digraph G {" << std::endl;

  /* inputs */
  for ( const auto& input : xmg.inputs() )
  {
    os << format( "%d[shape=house,label=\"%s\"%s];" ) % input.first % input.second % ( xmg.bitmarks().is_marked( input.first ) ? ",fillcolor=red,style=filled" : "" ) << std::endl;
  }

  /* outputs and nodes */
  boost::dynamic_bitset<> visited( xmg.size() );
  for ( const auto& output : xmg.outputs() )
  {
    xmg_cover_write_dot_rec( xmg, os, output.first.node, visited );

    os << format( "n%dout[shape=house,label=\"%s\"];" ) % output.first.node % output.second << std::endl;
    os << format( "n%dout->%d[%s];" ) % output.first.node % output.first.node % ( output.first.complemented ? "dashed" : "" ) << std::endl;
  }

  os << "}" << std::endl;
}

void xmg_cover_write_dot( const xmg_graph& xmg, const std::string& filename )
{
  std::ofstream out( filename.c_str(), std::ostream::out );
  xmg_cover_write_dot( xmg, out );
  out.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
