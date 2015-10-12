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

#include "thread_pool.hpp"

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

thread_pool::thread_pool()
  : thread_pool( std::thread::hardware_concurrency() )
{
}

thread_pool::thread_pool( unsigned num_threads )
{
  for ( auto i = 0u; i < num_threads; ++i )
  {
    workers.emplace_back( [this] {
        while ( true )
        {
          std::function<void()> task;

          {
            std::unique_lock<std::mutex> lock( this->queue_mutex );
            this->condition.wait( lock,
                                  [this]{ return this->stop || !this->tasks.empty(); } );
            if ( this->stop && this->tasks.empty() ) { return; }
            task = std::move( this->tasks.front() );
            this->tasks.pop();
          }
          task();
        }
      });
  }
}

thread_pool::~thread_pool()
{
  {
    std::unique_lock<std::mutex> lock( queue_mutex );
    stop = true;
  }

  condition.notify_all();
  for ( auto& worker : workers )
  {
    worker.join();
  }
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
