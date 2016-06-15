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
 * @file show.hpp
 *
 * @brief Shows current data structure in DOT viewer
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 * @since  2.3
 */

#pragma once

#include <cstdlib>
#include <string>

#include <alice/command.hpp>

#include <boost/any.hpp>
#include <boost/format.hpp>

using namespace boost::program_options;

namespace alice
{

using show_commands_t = std::map<std::string, boost::any>;

template<typename S>
int init_show_commands( command& cmd, show_commands_t& show_commands )
{
  constexpr auto option   = store_info<S>::option;

  show_commands[option] = std::make_shared<show_store_entry<S>>( cmd );

  return 0;
}

template<typename S>
int show_helper( bool& result, command& cmd, const environment::ptr& env, show_commands_t& show_commands, const std::string& dotname )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) )
  {
    result = boost::any_cast<std::shared_ptr<show_store_entry<S>>>( show_commands[option] )->operator()( env->store<S>().current(), dotname, cmd );
  }
  return 0;
}

template<typename S>
int show_log_helper( command::log_opt_t& result, const command& cmd, const show_commands_t& show_commands )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) )
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
      ( "dotname",  value( &dotname )->default_value( dotname ), "filename for the DOT file" )
      ( "dotcmd",   value( &dotcmd ),                            "command to show DOT file\ncan be controlled with variable 'show_dotcmd' (default: 'xdg-open \"%s\" &')" )
      ( "silent,s",                                              "don't show the DOT file, i.e., just save it" )
      ;
    [](...){}( add_option_helper<S>( opts )... );
    [](...){}( init_show_commands<S>( *this, show_commands )... );
  }

protected:
  bool execute()
  {
    if ( !is_set( "dotcmd" ) )
    {
      dotcmd = env->variable_value( "show_dotcmd", "xdg-open \"%s\" &" );
    }

    if ( dotname == "/tmp/test-%s.dot" )
    {
      dotname = ( boost::format( dotname ) % rand() ).str();
    }

    auto result = false;
    [](...){}( show_helper<S>( result, *this, env, show_commands, dotname )... );

    if ( !is_set( "silent" ) && result )
    {
      system( boost::str( boost::format( dotcmd ) % dotname ).c_str() );
    }

    return true;
  }

public:
  log_opt_t log() const
  {
    log_opt_t result;
    [](...){}( show_log_helper<S>( result, *this, show_commands )... );

    return result;
  }

protected:
  std::string dotname = "/tmp/test-%s.dot";
  std::string dotcmd  = "xdg-open \"%s\" &";

  show_commands_t show_commands;
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
