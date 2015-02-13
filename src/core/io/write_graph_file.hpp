/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

/**
 * @file write_graph_file.hpp
 *
 * @brief Writes a simple vertex/edge graph file
 *
 * @author Mathias Soeken
 * @since 2.0
 */

#ifndef WRITE_GRAPH_FILE
#define WRITE_GRAPH_FILE

#include <fstream>
#include <functional>
#include <iostream>

#include <boost/range/iterator_range.hpp>

namespace cirkit
{

template<typename Graph>
void write_graph_file( const Graph& g, const std::string& filename, bool verbose = false )
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_t;

  if ( verbose )
  {
    std::cout << "[I] write graph file to " << filename << std::endl;
  }

  std::ofstream os( filename.c_str(), std::ofstream::out );

  os << boost::num_vertices( g ) << std::endl;
  for ( const auto& v : boost::make_iterator_range( boost::vertices( g ) ) )
  {
    os << boost::out_degree( v, g );
    for ( const auto& w : boost::make_iterator_range( boost::adjacent_vertices( v, g ) ) )
    {
      os << " " << w;
    }
    os << std::endl;
  }

  os.close();
}

template<typename Graph, typename VertexLabelFunc, typename EdgeLabelFunc>
void write_labeled_graph_file( const Graph& g,
                               VertexLabelFunc&& vertex_label,
                               EdgeLabelFunc&& edge_label,
                               const std::string& filename, bool verbose = false )
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_t;

  if ( verbose )
  {
    std::cout << "[I] write graph file to " << filename << std::endl;
  }

  std::ofstream os( filename.c_str(), std::ofstream::out );

  os << boost::num_vertices( g ) << std::endl;
  for ( auto v = 0u; v < boost::num_vertices( g ); ++v )
  {
    os << vertex_label( v ) << " " << boost::out_degree( v, g );
    for ( const auto& e : boost::make_iterator_range( boost::out_edges( v, g ) ) )
    {
      os << " " << boost::target( e, g ) << " " << edge_label( e );
    }
    os << std::endl;
  }

  os.close();
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
