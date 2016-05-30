/* alice: A C++ EDA command line interface API
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file ps.hpp
 *
 * @brief Print statistics
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 * @since  2.3
 */

#pragma once

#include <vector>

#include <boost/format.hpp>

#include <alice/command.hpp>

namespace alice
{

template<typename S>
int ps_helper( const command& cmd, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( cmd.is_set( option ) )
  {
    if ( env->store<S>().current_index() == -1 )
    {
      std::cout << "[w] no " << name << " in store" << std::endl;
    }
    else
    {
      print_store_entry_statistics<S>( std::cout, env->store<S>().current() );
    }
  }

  return 0;
}

template<typename S>
int ps_log_helper( const command& cmd, const environment::ptr& env, command::log_opt_t& ret )
{
  if ( ret != boost::none )
  {
    return 0;
  }

  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) )
  {
    if ( env->store<S>().current_index() == -1 )
    {
      ret = boost::none;
    }
    else
    {
      ret = log_store_entry_statistics<S>( env->store<S>().current() );
    }
  }

  return 0;
}

template<class... S>
class ps_command : public command
{
public:
  ps_command( const environment::ptr& env )
    : command( env, "Print statistics" )
  {
    [](...){}( add_option_helper<S>( opts )... );
  }

protected:
  rules_t validity_rules() const
  {
    return {
      {[this]() { return any_true_helper( { is_set( store_info<S>::option )... } ); }, "no store has been specified" }
    };
  }

  bool execute()
  {
    [](...){}( ps_helper<S>( *this, env )... );

    return true;
  }

public:
  log_opt_t log() const
  {
    log_opt_t ret;
    [](...){}( ps_log_helper<S>( *this, env, ret )... );
    return ret;
  }
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
