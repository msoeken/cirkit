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
