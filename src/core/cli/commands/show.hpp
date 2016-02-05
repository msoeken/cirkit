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
 * @file show.hpp
 *
 * @brief Shows current data structure in DOT viewer
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_SHOW_COMMAND_HPP
#define CLI_SHOW_COMMAND_HPP

#include <cstdlib>
#include <string>

#include <core/cli/command.hpp>
#include <core/cli/environment.hpp>
#include <core/cli/store.hpp>

#include <boost/any.hpp>
#include <boost/format.hpp>

using namespace boost::program_options;

namespace cirkit
{

using show_commands_t = std::map<std::string, boost::any>;

template<typename S>
int init_show_commands( program_options& opts, show_commands_t& show_commands )
{
  constexpr auto option   = store_info<S>::option;
  constexpr auto mnemonic = store_info<S>::mnemonic;

  show_commands[option] = std::make_shared<show_store_entry<S>>( opts );

  return 0;
}

template<typename S>
int show_helper( bool& result, const program_options& opts, const environment::ptr& env, show_commands_t& show_commands, const std::string& dotname, const properties::ptr& settings )
{
  constexpr auto option = store_info<S>::option;

  if ( opts.is_set( option ) )
  {
    result = boost::any_cast<std::shared_ptr<show_store_entry<S>>>( show_commands[option] )->operator()( env->store<S>().current(), dotname, opts, settings );
  }
  return 0;
}

template<typename S>
int show_log_helper( command::log_opt_t& result, const program_options& opts, const show_commands_t& show_commands )
{
  constexpr auto option = store_info<S>::option;

  if ( opts.is_set( option ) )
  {
    result = boost::any_cast<std::shared_ptr<show_store_entry<S>>>( show_commands.at( option ) )->log();
  }
  return 0;
}

template<class... S>
class show_command : public command
{
public:
  show_command( const environment::ptr& env ) : command( env, "Shows current data structure in DOT viewer" )
  {
    opts.add_options()
      ( "dotname", value_with_default( &dotname ), "Filename for the DOT file" )
      ( "dotcmd",  value_with_default( &dotcmd ),  "Command to show DOT file" )
      ( "silent,s",                                "Don't show the DOT file, i.e., just save it" )
      /*( "expr,e",                                  "Show as string expression (only for MIGs)" )*/
      ;
    [](...){}( add_option_helper<S>( opts )... );
    [](...){}( init_show_commands<S>( opts, show_commands )... );
    be_verbose();
  }

protected:
  bool execute()
  {
    auto settings = make_settings();

    if ( dotname == "/tmp/test-%s.dot" )
    {
      dotname = ( boost::format( dotname ) % rand() ).str();
    }

    auto result = false;
    [](...){}( show_helper<S>( result, opts, env, show_commands, dotname, settings )... );

    if ( !opts.is_set( "silent" ) && result )
    {
      system( boost::str( boost::format( dotcmd ) % dotname ).c_str() );
    }

    return true;
  }

public:
  log_opt_t log() const
  {
    log_opt_t result;
    [](...){}( show_log_helper<S>( result, opts, show_commands )... );

    return result;
  }

protected:
  std::string dotname = "/tmp/test-%s.dot";
  std::string dotcmd  = "xdg-open \"%s\" &";

  show_commands_t show_commands;
};

/*
OLD

  if ( opts.is_set( "expr" ) )
  {
    for ( const auto& output : mig_info().outputs )
    {
      std::cout << boost::format( "[i] %s: %s" ) % output.second % mig_to_string( mig(), output.first ) << std::endl;
    }
  }
*/

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
