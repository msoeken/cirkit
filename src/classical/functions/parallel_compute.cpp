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

#include "parallel_compute.hpp"

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

void parallel_process(
    const aig_graph& aig,
    const std::function<void( aig_node )>& on_input,
    const std::function<void( aig_node, const aig_function&, const aig_function& )>& on_and )
{
  /* topological order */
  std::vector<aig_node> topsort( num_vertices( aig ) );
  boost::topological_sort( aig, topsort.begin() );

  boost::dynamic_bitset<>              processed( num_vertices( aig ) );
  std::mutex                           vector_mutex;
  std::vector<std::thread>             workers;
  std::vector<std::condition_variable> guards( num_vertices( aig ) );
  std::vector<std::mutex>              guard_locks( num_vertices( aig ) );
  processed.set( 0u );

  for ( auto n : topsort )
  {
    /* primary input */
    if ( out_degree( n, aig ) == 0u )
    {
      if ( n == 0u ) { continue; }

      workers.emplace_back( [&vector_mutex, &processed, n, &on_input, &guards, &guard_locks]() {
          on_input( n );
          vector_mutex.lock();
          processed.set( n );
          vector_mutex.unlock();

          std::unique_lock<std::mutex> lck( guard_locks[n] );
          guards[n].notify_all();
        } );
    }
    else
    {
      const auto children = get_children( aig, n );

      workers.emplace_back( [&vector_mutex, &processed, n, &on_and, children, &guards, &guard_locks]() {
          {
            std::unique_lock<std::mutex> lck( guard_locks[children[0].node] );
            while ( !processed[children[0].node] ) { guards[children[0].node].wait( lck ); }
          }
          {
            std::unique_lock<std::mutex> lck( guard_locks[children[1].node] );
            while ( !processed[children[1].node] ) { guards[children[1].node].wait( lck ); }
          }

          on_and( n, children[0], children[1] );
          vector_mutex.lock();
          processed.set( n );
          vector_mutex.unlock();

          std::unique_lock<std::mutex> lck( guard_locks[n] );
          guards[n].notify_all();
        } );
    }
  }

  for ( auto& worker : workers )
  {
    worker.join();
  }
}

void parallel_simulate( const aig_graph& aig, const boost::dynamic_bitset<>& pattern )
{
  std::vector<bool> computed_values;
  parallel_compute<bool>( aig,
                          false,
                          [&pattern]( unsigned index ) { return pattern[index]; },
                          []( bool v1, bool c1, bool v2, bool c2 ) { return ( v1 != c1 ) && ( v2 != c2 ); },
                          computed_values );

  for ( const auto& output : aig_info( aig ).outputs )
  {
    std::cout << "[i] " << output.second << " : " << ( computed_values[output.first.node] != output.first.complemented ) << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
