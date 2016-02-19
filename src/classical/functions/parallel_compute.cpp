/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
