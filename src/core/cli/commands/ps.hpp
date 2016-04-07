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
 * @file ps.hpp
 *
 * @brief Print statistics
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef CLI_PS_COMMAND_HPP
#define CLI_PS_COMMAND_HPP

#include <vector>

#include <boost/format.hpp>

#include <core/cli/command.hpp>
#include <core/cli/store.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>

namespace cirkit
{

template<typename S>
int ps_helper( const program_options& opts, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( opts.is_set( option ) )
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
int ps_log_helper( const program_options& opts, const environment::ptr& env, command::log_opt_t& ret )
{
  if ( ret != boost::none )
  {
    return 0;
  }

  constexpr auto option = store_info<S>::option;

  if ( opts.is_set( option ) )
  {
    ret = log_store_entry_statistics<S>( env->store<S>().current() );
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
      {[&]() { return any_true_helper( { opts.is_set( store_info<S>::option )... } ); }, "no store has been specified" }
    };
  }

  bool execute()
  {
    [](...){}( ps_helper<S>( opts, env )... );

    return true;
  }

public:
  log_opt_t log() const
  {
    log_opt_t ret;
    [](...){}( ps_log_helper<S>( opts, env, ret )... );
    return ret;
  }
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
