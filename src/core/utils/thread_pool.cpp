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
