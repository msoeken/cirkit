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
