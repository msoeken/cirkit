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

/**
 * @file parallel_compute.hpp
 *
 * @brief Parallel computation on data structures
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef PARALLEL_COMPUTE_HPP
#define PARALLEL_COMPUTE_HPP

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/topological_sort.hpp>

#include <classical/aig.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

template<typename T>
void parallel_compute(
    const aig_graph& aig, const T& constant_result,
    const std::function<T( unsigned )>& on_input,
    const std::function<T( const T&, bool, const T&, bool )>& on_and,
    std::vector<T>& computed_values )
{
  /* topological order */
  std::vector<aig_node> topsort( num_vertices( aig ) );
  boost::topological_sort( aig, topsort.begin() );

  const auto& info = aig_info( aig );

  computed_values.resize( num_vertices( aig ) );
  boost::dynamic_bitset<>              computed( num_vertices( aig ) );
  std::mutex                           vector_mutex;
  std::vector<std::thread>             workers;
  std::vector<std::condition_variable> guards( num_vertices( aig ) );
  std::vector<std::mutex>              guard_locks( num_vertices( aig ) );
  computed.set( 0u );
  computed_values[0u] = constant_result;

  for ( auto n : topsort )
  {
    /* primary input */
    if ( out_degree( n, aig ) == 0u )
    {
      if ( n == 0u ) { continue; }

      const auto index = aig_input_index( info, n );
      workers.emplace_back( [&vector_mutex, &computed_values, &computed, n, &on_input, index, &guards, &guard_locks]() {
          const auto v = on_input( index );
          vector_mutex.lock();
          computed_values[n] = v;
          computed.set( n );
          vector_mutex.unlock();

          std::unique_lock<std::mutex> lck( guard_locks[n] );
          guards[n].notify_all();
        } );
    }
    else
    {
      const auto children = get_children( aig, n );

      workers.emplace_back( [&vector_mutex, &computed_values, &computed, n, &on_and, children, &guards, &guard_locks]() {
          {
            std::unique_lock<std::mutex> lck( guard_locks[children[0].node] );
            while ( !computed[children[0].node] ) { guards[children[0].node].wait( lck ); }
          }
          {
            std::unique_lock<std::mutex> lck( guard_locks[children[1].node] );
            while ( !computed[children[1].node] ) { guards[children[1].node].wait( lck ); }
          }

          const auto v = on_and( computed_values[children[0].node], children[0].complemented,
                                 computed_values[children[1].node], children[1].complemented );
          vector_mutex.lock();
          computed_values[n] = v;
          computed.set( n );
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

void parallel_process(
    const aig_graph& aig,
    const std::function<void( aig_node )>& on_input,
    const std::function<void( aig_node, const aig_function&, const aig_function& )>& on_and );

/* this is a usage demo */
void parallel_simulate( const aig_graph& aig, const boost::dynamic_bitset<>& pattern );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
