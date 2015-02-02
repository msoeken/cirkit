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
 * @file read_graph_file.hpp
 *
 * @brief Reads a simple vertex/edge graph file
 *
 * @author  Mathias Soeken
 * @since 2.0
 */

#ifndef READ_GRAPH_FILE
#define READ_GRAPH_FILE

#include <fstream>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/string_utils.hpp>

namespace cirkit
{

template<typename Graph>
void read_graph_file( Graph& g, const std::string& filename, bool verbose = false )
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_t;

  if ( verbose )
  {
    std::cout << "[I] read graph file from " << filename << std::endl;
  }

  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::in );

  std::istream is( &fb );
  std::string line;

  /* read number of vertices to read */
  assert( getline( is, line ) );
  unsigned n = boost::lexical_cast<unsigned>( line );

  std::vector<vertex_t> vertices( n );
  boost::generate( vertices, [&g]() { return boost::add_vertex( g ); } );

  /* read edges */
  unsigned cur = 0u;
  while ( cur < n )
  {
    assert( getline( is, line ) );
    std::vector<unsigned> ids;
    parse_string_list( ids, line );

    for ( unsigned i = 1u; i <= ids.front(); ++i )
    {
      boost::add_edge( vertices[cur], vertices[ids[i]], g );
    }

    ++cur;
  }

  fb.close();
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
