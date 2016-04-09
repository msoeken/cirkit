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
 * @file utils.hpp
 *
 * @brief Utility functions for the CLI
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_UTILS_HPP
#define CLI_UTILS_HPP

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <core/cli/command.hpp>
#include <core/cli/environment.hpp>
#include <core/cli/store.hpp>
#include <core/cli/commands/convert.hpp>
#include <core/cli/commands/current.hpp>
#include <core/cli/commands/help.hpp>
#include <core/cli/commands/print.hpp>
#include <core/cli/commands/ps.hpp>
#include <core/cli/commands/quit.hpp>
#include <core/cli/commands/read_io.hpp>
#include <core/cli/commands/show.hpp>
#include <core/cli/commands/store.hpp>
#include <core/cli/commands/write_io.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>

namespace cirkit
{

bool read_command_line( const std::string& prefix, std::string& line );
bool execute_line( const environment::ptr& env, const std::string& line, const std::map<std::string, std::shared_ptr<command>>& commands );

template<typename S>
int add_store_helper( const environment::ptr& env )
{
  constexpr auto key  = store_info<S>::key;
  constexpr auto name = store_info<S>::name;

  env->add_store<S>( key, name );

  return 0;
}

template<class... S>
class cli_main
{
public:
  cli_main( const std::string& prefix )
    : env( std::make_shared<environment>() ),
      prefix( prefix )
  {
    using namespace boost::program_options;

    [](...){}( add_store_helper<S>( env )... );

    /* These are some generic commands that work on all store entries.
     *
     * For most of the commands there are some functions that need to be
     * implemented for the specific store entry types, e.g., for convert
     * one needs to implement store_can_convert and store_convert. Most
     * of the commands have default implementations, some of them throw
     * assertions in the default implementations.
     *
     * see store.hpp for more details
     */
    env->commands.insert( {"convert", std::make_shared<convert_command<S...>>( env )} );
    env->commands.insert( {"current", std::make_shared<current_command<S...>>( env )} );
    env->commands.insert( {"help",    std::make_shared<help_command>( env )} );
    env->commands.insert( {"quit",    std::make_shared<quit_command>( env )} );
    env->commands.insert( {"show",    std::make_shared<show_command<S...>>( env )} );
    env->commands.insert( {"store",   std::make_shared<store_command<S...>>( env )} );
    env->commands.insert( {"print",   std::make_shared<print_command<S...>>( env )} );
    env->commands.insert( {"ps",      std::make_shared<ps_command<S...>>( env )} );

    env->commands.insert( {"read_verilog", std::make_shared<read_io_command<io_verilog_tag_t, S...>>( env, "Verilog" )} );
    env->commands.insert( {"write_edgelist", std::make_shared<write_io_command<io_edgelist_tag_t, S...>>( env, "Edge list" )} );
    env->commands.insert( {"write_verilog", std::make_shared<write_io_command<io_verilog_tag_t, S...>>( env, "Verilog" )} );

    opts.add_options()
      ( "command,c", value( &command ), "Process semicolon-separated list of commands" )
      ( "file,f",    value( &file ),    "Process file with new-line seperated list of commands" )
      ( "echo,e",                       "Echos the command if read from command line or file" )
      ( "counter,n",                    "Show a counter in the prefix" )
      ( "log,l",     value( &logname ), "Logs the execution and stores many statistical information" )
      ;

    read_aliases();
  }

  int run( int argc, char ** argv )
  {
    opts.parse( argc, argv );

    if ( !opts.good() || ( opts.is_set( "command" ) && opts.is_set( "file" ) ) )
    {
      std::cout << opts << std::endl;
      return 1;
    }

    if ( opts.is_set( "log" ) )
    {
      env->log = true;
      env->start_logging( logname );
    }

    if ( opts.is_set( "command" ) )
    {
      std::vector<std::string> split;
      boost::algorithm::split( split, command, boost::is_any_of( ";" ), boost::algorithm::token_compress_on );

      auto collect_commands = false;
      std::string batch_string;
      std::string abc_opts;
      for ( auto& line : split )
      {
        boost::trim( line );
        if ( collect_commands )
        {
          batch_string += ( line + "; " );
          if ( line == "quit" )
          {
            if ( opts.is_set( "echo" ) ) { std::cout << get_prefix() << "abc -c \"" + batch_string << "\"" << std::endl; }
            std::cout << "abc" << ' ' << abc_opts << ' ' << batch_string << '\n';
            execute_line( env, ( boost::format("abc %s-c \"%s\"") % abc_opts % batch_string ).str(), env->commands );
            batch_string.clear();
            collect_commands = false;
          }
        }
        else
        {
          if ( boost::starts_with( line, "abc" ) )
          {
            collect_commands = true;
            abc_opts = ( line.size() > 4u ? (line.substr( 4u ) + " ") : "" );
          }
          else
          {
            if ( opts.is_set( "echo" ) ) { std::cout << get_prefix() << line << std::endl; }
            if ( !execute_line( env, preprocess_alias( line ), env->commands ) )
            {
              return 1;
            }
          }
        }

        if ( env->quit ) { break; }
      }
    }
    else if ( opts.is_set( "file" ) )
    {
      foreach_line_in_file( file, [&]( const std::string& line ) {
          if ( opts.is_set( "echo" ) ) { std::cout << get_prefix() << line << std::endl; }
          execute_line( env, preprocess_alias( line ), env->commands );

          return !env->quit;
        } );
    }
    else
    {
#ifdef USE_READLINE
      initialize_readline();
#endif

      std::string line;
      while ( !env->quit && read_command_line( get_prefix(), line ) )
      {
        execute_line( env, preprocess_alias( line ), env->commands );
#ifdef USE_READLINE
        add_history( line.c_str() );
#endif
      }
    }

    if ( env->log )
    {
      env->stop_logging();
    }

    return 0;
  }

public:
  std::shared_ptr<environment> env;

private:
  std::string get_prefix()
  {
    if ( opts.is_set( "counter" ) )
    {
      return prefix + boost::str( boost::format( " %d> " ) % counter++ );
    }
    else
    {
      return prefix + "> ";
    }
  }

  void read_aliases()
  {
    if ( auto* path = std::getenv( "CIRKIT_HOME" ) )
    {
      std::string alias_path = boost::str( boost::format( "%s/alias" ) % path );
      if ( !boost::filesystem::exists( alias_path ) ) { return; }

      foreach_line_in_file( alias_path, [this]( const std::string& line ) {
          if ( line.empty() || line[0] == '#' ) { return true; }

          auto p = split_string_pair( line, "=" );
          boost::trim( p.first );
          boost::trim( p.second );

          aliases.insert( p );

          return true;
        } );
    }
  }

  std::string preprocess_alias( const std::string& line )
  {
    const auto it = aliases.find( line );
    if ( it != aliases.end() )
    {
      return it->second;
    }
    else
    {
      return line;
    }
  }

#ifdef USE_READLINE
private:
  static cli_main<S...>*   instance;
  std::vector<std::string> command_names;

  void initialize_readline()
  {
    instance = this;
    command_names = get_map_keys( env->commands );
    rl_attempted_completion_function = cli_main<S...>::readline_completion_s;
  }

  static char ** readline_completion_s( const char* text, int start, int end )
  {
    if ( start == 0 )
    {
      return rl_completion_matches( text, []( const char* text, int state ) { return instance->command_iterator( text, state ); } );
    }
    else
    {
      return nullptr;
    }
  }

  char * command_iterator( const char * text, int state )
  {
    static std::vector<std::string>::const_iterator it;

    if ( state == 0 )
    {
      it = command_names.begin();
    }

    while ( it != command_names.end() )
    {
      const auto& name = *it++;
      if ( name.find( text ) != std::string::npos )
      {
        char * completion = new char[name.size()];
        strcpy( completion, name.c_str() );
        return completion;
      }
    }

    return nullptr;
  }
#endif

private:
  std::string                        prefix;

  program_options                    opts;

  std::string                        command;
  std::string                        file;
  std::string                        logname;

  unsigned                           counter = 1u;

  std::map<std::string, std::string> aliases;
};

#define ADD_COMMAND( name ) cli.env->commands.insert( {#name, std::make_shared<name##_command>( cli.env ) } );

#ifdef USE_READLINE
template<class... S>
cli_main<S...>* cli_main<S...>::instance = nullptr;
#endif

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
