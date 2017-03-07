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
 * @file read_graph_file.hpp
 *
 * @brief Reads a simple vertex/edge graph file
 *
 * @author Mathias Soeken
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
  using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

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
