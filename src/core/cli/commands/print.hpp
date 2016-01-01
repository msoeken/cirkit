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
 * @file print.hpp
 *
 * @brief Prints current data structure
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_PRINT_COMMAND_HPP
#define CLI_PRINT_COMMAND_HPP

#include <core/cli/command.hpp>
#include <core/cli/environment.hpp>
#include <core/cli/store.hpp>

namespace cirkit
{

template<typename S>
int print_helper( const program_options& opts, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;

  if ( opts.is_set( option ) )
  {
    print_store_entry<S>( std::cout, env->store<S>().current() );
  }
  return 0;
}

template<class... S>
class print_command : public command
{
public:
  print_command( const environment::ptr& env ) : command( env, "Prints current data structure" )
  {
    [](...){}( add_option_helper<S>( opts )... );
  }

protected:
  rules_t validity_rules() const
  {
    return {
      {[&]() { return exactly_one_true_helper( { opts.is_set( store_info<S>::option )... } ); }, "exactly one store needs to be specified" }
    };
  }

  bool execute()
  {
    [](...){}( print_helper<S>( opts, env )... );

    return true;
  }
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
