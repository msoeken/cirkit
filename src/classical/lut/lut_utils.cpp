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

#include "lut_utils.hpp"

#include <classical/lut/lut_coi.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

int compute_depth_rec( const lut_graph& graph, const lut_vertex_t& node, std::vector<int>& depths )
{
  if ( depths[node] >= 0 ) return depths[node];

  if ( graph.is_input(node) )
  {
    return depths[node] = 0;
  }
  else
  {
    int depth = 0;
    for ( const auto& c : graph.children( node ) )
    {
      depth = std::max( compute_depth_rec( graph, c, depths ), depth );
    }
    return depths[node] = ( depth + 1 );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned lut_compute_depth( const lut_graph& graph )
{
  std::vector<int> depths( graph.size(), -1 );

  int depth = -1;

  for ( const auto& output : graph.outputs() )
  {
    depth = std::max( compute_depth_rec( graph, output.first, depths ), depth );
  }

  return static_cast<unsigned>( depth );
}

void lut_print_stats( const lut_graph& graph, std::ostream& os )
{
  auto name = graph.name();
  if ( name.empty() )
  {
    name = "(unnamed)";
  }

  os << boost::format( "[i] %20s: i/o = %7d / %7d gates = %7d lev = %4d" ) % name % graph.inputs().size() % graph.outputs().size() % graph.num_gates() % lut_compute_depth( graph );

  os << std::endl;
}

std::vector<unsigned> lut_compute_levels( const lut_graph& graph )
{
  std::vector<unsigned> levels( graph.size() );
  for ( const auto& n : graph.topological_nodes() )
  {
    if ( graph.is_input( n ) )
    {
      levels[n] = 0u;
    }
    else
    {
      auto level = 0u;
      for ( const auto& c : graph.children( n ) )
      {
        level = std::max( level, levels[c] );
      }
      levels[n] = level + 1u;
    }
  }
  return levels;
}

std::vector<boost::dynamic_bitset<>> lut_compute_sections( const lut_graph& graph )
{
  using namespace boost::assign;

  std::vector< boost::dynamic_bitset<> > sections;
  for ( const auto& o : graph.outputs() )
  {
    sections += lut_compute_coi_as_bitset( graph, o.first );
  }
  sections = transpose( sections );

  std::vector< boost::dynamic_bitset<> > node_to_section;
  node_to_section.resize( graph.nodes().size() );
  for ( const auto& s : index( sections ) )
  {
    node_to_section[ s.index + 2u ] = s.value;
  }
  return node_to_section;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
