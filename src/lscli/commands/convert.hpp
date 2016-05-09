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
 * @file convert.hpp
 *
 * @brief Convert something into something else
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_CONVERT_COMMAND_HPP
#define CLI_CONVERT_COMMAND_HPP

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <lscli/command.hpp>

namespace po = boost::program_options;

namespace cirkit
{

template<typename D, typename S>
int add_combination_helper_inner( po::options_description& opts )
{
  if ( store_can_convert<S, D>() )
  {
    constexpr auto source_option = store_info<S>::option;
    constexpr auto dest_option   = store_info<D>::option;

    constexpr auto source_name   = store_info<S>::name;
    constexpr auto dest_name     = store_info<D>::name;

    opts.add_options()
      ( ( boost::format( "%s_to_%s" ) % source_option % dest_option ).str().c_str(),
        ( boost::format( "convert %s to %s" ) % source_name % dest_name ).str().c_str() )
      ;
  }
  return 0;
};

template<typename D, class... S>
int add_combination_helper( po::options_description& opts )
{
  [](...){}( add_combination_helper_inner<D, S>( opts )... );
  return 0;
};

template<typename D, class S>
int convert_helper_inner( const environment::ptr& env, const command& cmd )
{
  if ( store_can_convert<S, D>() )
  {
    constexpr auto source_option = store_info<S>::option;
    constexpr auto dest_option   = store_info<D>::option;

    if ( cmd.is_set( ( boost::format( "%s_to_%s" ) % source_option % dest_option ).str().c_str() ) )
    {
      constexpr auto source_name = store_info<S>::name;
      const auto& source_store = env->store<S>();

      if ( source_store.current_index() == -1 )
      {
        std::cout << boost::format( "[w] there is no %s to convert from" ) % source_name << std::endl;
        return 0;
      }

      auto& dest_store = env->store<D>();
      dest_store.extend();
      dest_store.current() = store_convert<S, D>( source_store.current() );
    }
  }
  return 0;
}

template<typename D, class... S>
int convert_helper( const environment::ptr& env, const command& cmd )
{
  [](...){}( convert_helper_inner<D, S>( env, cmd )... );
  return 0;
}

template<class... S>
class convert_command : public command
{
public:
  convert_command( const environment::ptr& env )
    : command( env, "Convert something into something else" )
  {
    [](...){}( add_combination_helper<S, S...>( opts )... );
  }

protected:
  bool execute()
  {
    [](...){}( convert_helper<S, S...>( env, *this )... );
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
