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
 * @file store.hpp
 *
 * @brief Store management
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef CLI_STORE_COMMAND_HPP
#define CLI_STORE_COMMAND_HPP

#include <vector>

#include <boost/format.hpp>

#include <core/cli/command.hpp>
#include <core/cli/store.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>

namespace cirkit
{

template<typename S>
int show_helper( const command& cmd, const environment::ptr& env )
{
  constexpr auto option      = store_info<S>::option;
  constexpr auto name_plural = store_info<S>::name_plural;

  const auto& store = env->store<S>();

  if ( cmd.is_set( option ) )
  {
    if ( store.empty() )
    {
      std::cout << boost::format( "[i] no %s in store" ) % name_plural << std::endl;
    }
    else
    {
      std::cout << boost::format( "[i] %s in store:" ) % name_plural << std::endl;
      for ( const auto& element : index( store.data() ) )
      {
        std::cout << boost::format( "  %c %2d: " ) % ( store.current_index() == element.index ? '*' : ' ' ) % element.index;
        std::cout << store_entry_to_string<S>( element.value ) << std::endl;
      }
    }
  }

  return 0;
}

template<typename S>
int clear_helper( const command& cmd, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) )
  {
    env->store<S>().clear();
  }
  return 0;
}

template<class... S>
class store_command : public command
{
public:
  store_command( const environment::ptr& env )
    : command( env, "Store management" )
  {
    opts.add_options()
      ( "show",  "Show contents" )
      ( "clear", "Clear contents" )
      ;

    [](...){}( add_option_helper<S>( opts )... );
  }

protected:
  rules_t validity_rules() const
  {
    return {
      {[this]() { return static_cast<unsigned>( is_set( "show" ) ) + static_cast<unsigned>( is_set( "clear" ) ) <= 1u; }, "only one operation can be specified" },
      {[this]() { return any_true_helper( { is_set( store_info<S>::option )... } ); }, "no store has been specified" }
    };
  }

  bool execute()
  {
    if ( is_set( "show" ) || !is_set( "clear" ) )
    {
      [](...){}( show_helper<S>( *this, env )... );
    }
    else if ( is_set( "clear" ) )
    {
      [](...){}( clear_helper<S>( *this, env )... );
    }

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
