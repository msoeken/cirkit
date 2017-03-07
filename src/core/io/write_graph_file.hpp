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
