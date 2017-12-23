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
 * @file timer.hpp
 *
 * @brief A generic way for measuring time
 *
 * @author Mathias Soeken
 * @author Stefan Frehse
 * @since  1.0
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <iostream>

#include <core/properties.hpp>

#include <fmt/format.h>

namespace cirkit
{

class timer
{
public:
  timer() : start( std::chrono::high_resolution_clock::now() ) {}

protected:
  double stop()
  {
    auto _stop = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>{_stop - start}.count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

class print_timer : public timer
{
public:
  print_timer( const std::string& format = "[i] run-time: {} secs\n", bool verbose = false, std::ostream& os = std::cout )
      : timer(),
        format( format ),
        verbose( verbose ),
        os( os )
  {
  }

  ~print_timer()
  {
    const auto runtime = stop();
    if ( verbose )
    {
      os << fmt::format( format, runtime );
    }
  }

private:
  std::string format;
  bool verbose;
  std::ostream& os;
};

/* this will soon replace the old properties timer */
class properties_timer : public timer
{
public:
  properties_timer( const properties::ptr& statistics, const std::string& key = "runtime" )
      : timer(),
        statistics( statistics ), key( key )
  {
  }

  ~properties_timer()
  {
    const auto runtime = stop();
    if ( statistics )
    {
      statistics->set( key, runtime );
    }
  }

private:
  const properties::ptr& statistics;
  std::string key;
};

class reference_timer : public timer
{
public:
  reference_timer( double* runtime )
      : timer(),
        runtime( runtime )
  {
  }

  ~reference_timer()
  {
    const auto time = stop();
    if ( runtime )
    {
      *runtime = time;
    }
  }

private:
  double* runtime = nullptr;
};

class increment_timer : public timer
{
public:
  increment_timer( double* runtime )
      : timer(),
        runtime( runtime )
  {
  }

  ~increment_timer()
  {
    const auto time = stop();
    if ( runtime )
    {
      *runtime += time;
    }
  }

private:
  double* runtime;
};
}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
