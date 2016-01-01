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
 * @file thread_pool.hpp
 *
 * @brief Thread pool
 *
 * based on an implementation of Jakob Progsch and Václav Zeman
 * (see copyright notice below)
 * https://github.com/progschj/ThreadPool
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace cirkit
{

class thread_pool
{
public:
  /* number of threads is the same as number of cores */
  thread_pool();
  thread_pool( unsigned num_threads );
  ~thread_pool();

  template<class F, class... Args>
  auto enqueue( F&& f, Args&&... args ) -> std::future<typename std::result_of<F(Args...)>::type>
  {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind( std::forward<F>( f ), std::forward<Args>( args )... )
      );

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock( queue_mutex );

      /* don't allow enqueueing after stopping the pool */
      if ( stop )
      {
        throw std::runtime_error( "enqueue on stopped thread pool" );
      }

      tasks.emplace( [task]() { (*task)(); } );
    }

    condition.notify_one();
    return res;
  }

private:
  std::vector<std::thread>          workers;
  std::queue<std::function<void()>> tasks;
  std::mutex                        queue_mutex;
  std::condition_variable           condition;
  bool                              stop = false;
};

}

#endif

/* from: https://github.com/progschj/ThreadPool/blob/master/COPYING

Copyright (c) 2012 Jakob Progsch, Václav Zeman

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.

*/

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
