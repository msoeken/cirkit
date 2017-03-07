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

#include <cassert>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/times.h>

#include <boost/format.hpp>
#include <boost/timer/timer.hpp>

#include <core/properties.hpp>

namespace cirkit {

/* this will soon replace the old print_timer */
class print_timer : public boost::timer::cpu_timer
{
public:
  print_timer( const std::string& format = "[i] run-time: %w secs\n", bool verbose = false, std::ostream& os = std::cout )
    : format( format ),
      verbose( verbose ),
      os( os )
  {
    start();
  }

  ~print_timer()
  {
    if ( !is_stopped() )
    {
      stop();
      if ( verbose )
      {
        os << cpu_timer::format( 2, format ) << std::endl;
      }
    }
  }

private:
  std::string   format;
  bool          verbose;
  std::ostream& os;
};

/* this will soon replace the old properties timer */
class properties_timer : public boost::timer::cpu_timer
{
public:
  properties_timer( const properties::ptr& statistics, const std::string& key = "runtime" )
    : statistics( statistics ), key( key )
  {
    start();
  }

  ~properties_timer()
  {
    if ( !is_stopped() )
    {
      stop();
      if ( statistics )
      {
        const double sec = 1000000000.0L;
        boost::timer::cpu_times const elapsed_times(elapsed());
        double total_sec = ( elapsed_times.system + elapsed_times.user ) / sec;
        statistics->set( key, total_sec );
        statistics->set( key + "_user", elapsed_times.user / sec );
        statistics->set( key + "_system", elapsed_times.system / sec );
        statistics->set( key + "_wall", elapsed_times.wall / sec );
      }
    }
  }

private:
  const properties::ptr& statistics;
  std::string            key;
};

class reference_timer : public boost::timer::cpu_timer
{
public:
  reference_timer( double* runtime )
    : runtime( runtime )
  {
    start();
  }

  ~reference_timer()
  {
    if ( !is_stopped() )
    {
      stop();
      if ( runtime )
      {
        const double sec = 1000000000.0L;
        boost::timer::cpu_times const elapsed_times(elapsed());
        *runtime = ( elapsed_times.system + elapsed_times.user ) / sec;
      }
    }
  }

private:
  double* runtime = nullptr;
};

class increment_timer : public boost::timer::cpu_timer
{
public:
  increment_timer( double* runtime ) : runtime( runtime )
  {
    start();
  };

  ~increment_timer()
  {
    if ( !is_stopped() )
    {
      stop();
      if ( runtime )
      {
        const double sec = 1000000000.0L;
        boost::timer::cpu_times const elapsed_times(elapsed());
        *runtime += ( elapsed_times.system + elapsed_times.user ) / sec;
      }
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
