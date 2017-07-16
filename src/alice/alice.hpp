/* alice: A C++ EDA command line interface API
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
 * @file utils.hpp
 *
 * @brief Utility functions for the CLI
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <alice/command.hpp>
#include <alice/readline.hpp>
#include <alice/commands/alias.hpp>
#include <alice/commands/convert.hpp>
#include <alice/commands/current.hpp>
#include <alice/commands/help.hpp>
#include <alice/commands/print.hpp>
#include <alice/commands/ps.hpp>
#include <alice/commands/quit.hpp>
#include <alice/commands/read_io.hpp>
#include <alice/commands/set.hpp>
#include <alice/commands/show.hpp>
#include <alice/commands/store.hpp>
#include <alice/commands/write_io.hpp>

#ifdef ALICE_PYTHON
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif

namespace po = boost::program_options;

namespace alice
{

namespace detail
{

std::vector<std::string> split_commands( const std::string& commands )
{
  std::vector<std::string> result;
  std::string current;

  enum _state { normal, quote, escape };

  _state s = normal;

  for ( auto c : commands )
  {
    switch ( s )
    {
    case normal:
      switch ( c )
      {
      case '"':
        current += c;
        s = quote;
        break;

      case ';':
        boost::trim( current );
        result.push_back( current );
        current.clear();
        break;

      default:
        current += c;
        break;
      }
      break;

    case quote:
      switch ( c )
      {
      case '"':
        current += c;
        s = normal;
        break;

      case '\\':
        current += c;
        s = escape;
        break;

      default:
        current += c;
        break;
      };
      break;

    case escape:
      current += c;
      s = quote;
      break;
    }
  }

  boost::trim( current );
  if ( !current.empty() )
  {
    result.push_back( current );
  }

  return result;
}

// from http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
std::pair<int, std::string> execute_program( const std::string& cmd )
{
  char buffer[128];
  std::string result;
  int exit_status;

  std::shared_ptr<FILE> pipe( popen( cmd.c_str(), "r" ), [&exit_status]( FILE* fp ) { auto status = pclose( fp ); exit_status = WEXITSTATUS( status ); } );
  if ( !pipe )
  {
    throw std::runtime_error( "[e] popen() failed" );
  }
  while ( !feof( pipe.get() ) )
  {
    if ( fgets( buffer, 128, pipe.get() ) != NULL )
    {
      result += buffer;
    }
  }
  return {exit_status, result};
}

}

template<typename S>
int add_store_helper( const environment::ptr& env )
{
  constexpr auto key  = store_info<S>::key;
  constexpr auto name = store_info<S>::name;

  env->add_store<S>( key, name );

  return 0;
}

#ifdef ALICE_PYTHON
struct log_var_python_visitor : public boost::static_visitor<py::object>
{
public:
  py::object operator()( const std::string& s ) const
  {
    return py::str( s );
  }

  py::object operator()( int i ) const
  {
    return py::int_( i );
  }

  py::object operator()( unsigned i ) const
  {
    return py::int_( static_cast<int>( i ) );
  }

  py::object operator()( uint64_t i ) const
  {
    return py::int_( i );
  }

  py::object operator()( double d ) const
  {
    return py::float_( d );
  }

  py::object operator()( bool b ) const
  {
    return py::bool_( b );
  }

  template<typename T>
  py::object operator()( const std::vector<T>& v ) const
  {
    py::list value;

    for ( const auto& element : v )
    {
      value.append( operator()( element ) );
    }

    return value;
  }
};

class return_value_dict
{
public:
  return_value_dict( const command::log_map_t& map )
  {
    log_var_python_visitor vis;

    for ( const auto& lp : map )
    {
      if ( lp.first == "__repr__" )
      {
        repr = boost::get<std::string>( lp.second );
      }
      else if ( lp.first == "_repr_html_" )
      {
        repr_html = boost::get<std::string>( lp.second );
      }
      else
      {
        const auto value = boost::apply_visitor( vis, lp.second );
        _dict[py::str( lp.first )] = value;
      }
    }
  }

  py::object __getitem__( const std::string& key ) const
  {
    return _dict.attr( "__getitem__" )( py::str( key ) );
  }

  std::string __repr__() const
  {
    if ( repr.empty() )
    {
      return py::str( _dict.attr( "__repr__" )() );
    }
    else
    {
      return repr;
    }
  }

  std::string _repr_html_() const
  {
    if ( repr_html.empty() )
    {
      return boost::str( boost::format( "<pre>%s</pre>" ) % __repr__() );
    }
    else
    {
      return repr_html;
    }
  }

  const py::dict& dict() const
  {
    return _dict;
  }

private:
  py::dict    _dict;

  std::string repr;
  std::string repr_html;
};
#endif

template<class... S>
class cli_main
{
public:
  cli_main( const std::string& prefix )
    : env( std::make_shared<environment>() ),
      prefix( prefix )
  {
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
    set_category( "General" );
    insert_command( "alias",   std::make_shared<alias_command>( env ) );
    insert_command( "convert", std::make_shared<convert_command<S...>>( env ) );
    insert_command( "current", std::make_shared<current_command<S...>>( env ) );
    insert_command( "help",    std::make_shared<help_command>( env ) );
    insert_command( "print",   std::make_shared<print_command<S...>>( env ) );
    insert_command( "ps",      std::make_shared<ps_command<S...>>( env ) );
    insert_command( "quit",    std::make_shared<quit_command>( env ) );
    insert_command( "set",     std::make_shared<set_command>( env ) );
    insert_command( "show",    std::make_shared<show_command<S...>>( env ) );
    insert_command( "store",   std::make_shared<store_command<S...>>( env ) );

    opts.add_options()
      ( "command,c",     po::value( &command ), "process semicolon-separated list of commands" )
      ( "file,f",        po::value( &file ),    "process file with new-line seperated list of commands" )
      ( "echo,e",                               "echos the command if read from command line or file" )
      ( "counter,n",                            "show a counter in the prefix" )
      ( "interactive,i",                        "continue in interactive mode after processing commands (in command or file mode)" )
      ( "log,l",         po::value( &logname ), "logs the execution and stores many statistical information" )
      ( "help,h",                               "produce help message" )
      ;
  }

  void set_category( const std::string& _category )
  {
    category = _category;
  }

  void insert_command( const std::string& name, const std::shared_ptr<command>& cmd )
  {
    env->categories[category].push_back( name );
    env->commands[name] = cmd;
  }

  template<typename Tag>
  void insert_read_command( const std::string& name, const std::string& label )
  {
    insert_command( name, std::make_shared<read_io_command<Tag, S...>>( env, label ) );
  }

  template<typename Tag>
  void insert_write_command( const std::string& name, const std::string& label )
  {
    insert_command( name, std::make_shared<write_io_command<Tag, S...>>( env, label ) );
  }

  int run( int argc, char ** argv )
  {
    po::store( po::command_line_parser( argc, argv ).options( opts ).run(), vm );
    po::notify( vm );

    if ( vm.count( "help" ) || ( vm.count( "command" ) && vm.count( "file" ) ) )
    {
      std::cout << opts << std::endl;
      return 1;
    }

    read_aliases();

    if ( vm.count( "log" ) )
    {
      env->log = true;
      env->start_logging( logname );
    }

    if ( vm.count( "command" ) )
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
            if ( vm.count( "echo" ) ) { std::cout << get_prefix() << "abc -c \"" + batch_string << "\"" << std::endl; }
            std::cout << "abc" << ' ' << abc_opts << ' ' << batch_string << '\n';
            execute_line( ( boost::format("abc %s-c \"%s\"") % abc_opts % batch_string ).str() );
            batch_string.clear();
            collect_commands = false;
          }
        }
        else
        {
          if ( boost::starts_with( line, "abc " ) )
          {
            collect_commands = true;
            abc_opts = ( line.size() > 4u ? (line.substr( 4u ) + " ") : "" );
          }
          else
          {
            if ( vm.count( "echo" ) ) { std::cout << get_prefix() << line << std::endl; }
            if ( !execute_line( preprocess_alias( line ) ) )
            {
              return 1;
            }
          }
        }

        if ( env->quit ) { break; }
      }
    }
    else if ( vm.count( "file" ) )
    {
      process_file( file, vm.count( "echo" ) );

      if ( !vm.count( "interactive" ) )
      {
        env->quit = true;
      }
    }

    if ( ( !vm.count( "command" ) && !vm.count( "file" ) ) || ( !env->quit && vm.count( "interactive" ) ) )
    {
      readline rl( env );

      std::string line;
      while ( !env->quit && rl.read_command_line( get_prefix(), line ) )
      {
        execute_line( preprocess_alias( line ) );
        rl.add_to_history( line );
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
  bool execute_line( const std::string& line )
  {
    /* ignore comments and empty lines */
    if ( line.empty() || line[0] == '#' ) { return false; }

    /* split commands if line contains a semi-colon */
    const auto lines = detail::split_commands( line );

    if ( lines.size() > 1u )
    {
      auto result = true;

      for ( const auto& cline : lines )
      {
        result = result && execute_line( preprocess_alias( cline ) );
      }

      return result;
    }

    /* escape to shell */
    if ( line[0] == '!' )
    {
      const auto now = std::chrono::system_clock::now();
      const auto result = detail::execute_program( line.substr( 1u ) );

      std::cout << result.second;

      if ( !result.second.empty() && result.second.back() != '\n' )
      {
        std::cout << '%' << std::endl;
      }

      if ( env->log )
      {
        command::log_map_t log;
        log["status"] = result.first;
        log["output"] = result.second;
        env->log_command( command::log_opt_t( log ), line, now );
      }

      return true;
    }

    /* read commands from file */
    if ( line[0] == '<' )
    {
      auto filename = line.substr( 1u );
      boost::trim( filename );
      process_file( filename, vm.count( "echo" ) );
      return true;
    }

    std::vector<std::string> vline;
    boost::tokenizer<boost::escaped_list_separator<char>> tok( line, boost::escaped_list_separator<char>( '\\', ' ', '\"' ) );

    for ( const auto& s : tok )
    {
      if ( !s.empty() )
      {
        vline.push_back( s );
      }
    }

    const auto it = env->commands.find( vline.front() );
    if ( it != env->commands.end() )
    {
      const auto now = std::chrono::system_clock::now();
      const auto result = it->second->run( vline );

      if ( result && env->log )
      {
        env->log_command( it->second, line, now );
      }

      return result;
    }
    else
    {
      std::cout << "unknown command: " << vline.front() << std::endl;
      return false;
    }
  }

#ifdef ALICE_PYTHON
public:
  py::module pymodule()
  {
    py::module m( prefix.c_str(), "Python bindings" );

    py::class_<return_value_dict> representer( m, "ReturnValueDict" );
    representer
      .def( "__getitem__", &return_value_dict::__getitem__ )
      .def( "__repr__", &return_value_dict::__repr__ )
      .def( "_repr_html_", &return_value_dict::_repr_html_ )
      .def( "dict", &return_value_dict::dict )
      ;

    for ( const auto& p : env->commands )
    {
      m.def( p.first.c_str(), [p]( py::kwargs kwargs ) -> py::object {
          std::vector<std::string> pargs = {p.first};

          for ( const auto& kp : kwargs )
          {
            auto key = kp.first.ptr();
            auto value = kp.second.ptr();

            const auto skey = std::string( PyUnicode_AsUTF8( key ) );

            if ( PyBool_Check( value ) )
            {
              if ( value == Py_True )
              {
                pargs.push_back( "--" + skey );
              }
            }
            else if ( PyLong_Check( value ) )
            {
              pargs.push_back( "--" + skey );
              pargs.push_back( std::to_string( PyLong_AsLong( value ) ) );
            }
            else
            {
              pargs.push_back( "--" + skey );
              pargs.push_back( std::string( PyUnicode_AsUTF8( value ) ) );
            }
          }
          p.second->run( pargs );

          const auto log = p.second->log();

          if ( log )
          {
            return py::cast( return_value_dict( *log ) );
          }
          else
          {
            return py::none();
          }

//             log_var_python_visitor vis;

//             /* Python API */
// #ifdef ALICE_PYTHON
//             const auto it = log->find( "__repr__" );
//             if ( it != log->end() )
//             {
//               std::string repr = boost::get<std::string>( it->second );
//               std::string repr_html;
//               const auto it2 = log->find( "_repr_html_" );
//               if ( it2 != log->end() )
//               {
//                 repr_html = boost::get<std::string>( it2->second );
//               }
//               return py::cast( python_representer( repr, repr_html ) );
//             }
// #endif

//             for ( const auto& lp : *log )
//             {
//               const auto value = boost::apply_visitor( vis, lp.second );
//               dict[py::str( lp.first )] = value;
//             }
        }, p.second->caption().c_str() );
    }

    return m;
  }
#endif

private:
  /**
   * @param filename filename with commands
   * @param echo     true, if command should be echoed before execution
   *
   * @return true, if program should exit after this call
   */
  bool process_file( const std::string& filename, bool echo )
  {
    std::ifstream in( filename.c_str(), std::ifstream::in );

    if ( !in.good() )
    {
      std::cout << "[e] file " << filename << " not found" << std::endl;
      return true;
    }

    std::string line;

    while ( getline( in, line ) )
    {
      boost::trim( line );

      if ( echo )
      {
        std::cout << get_prefix() << line << std::endl;
      }

      execute_line( preprocess_alias( line ) );

      if ( env->quit )
      {
        /* quit */
        return true;
      }
    }

    /* do not quit */
    return false;
  }

  std::string get_prefix()
  {
    if ( vm.count( "counter" ) )
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

      process_file( alias_path, false );
    }
  }

  std::string preprocess_alias( const std::string& line )
  {
    std::smatch m;

    for ( const auto& p : env->aliases )
    {
      if ( std::regex_match( line, m, std::regex( p.first ) ) )
      {
        auto fmt = boost::format( p.second );

        for ( auto i = 1u; i < m.size(); ++i )
        {
          fmt = fmt % std::string( m[i] );
        }

        auto str = fmt.str();
        boost::trim( str );
        return preprocess_alias( str );
      }
    }

    return line;
  }

private:
  std::string             prefix;

  po::options_description opts;
  po::variables_map       vm;

  std::string             category;
  std::string             command;
  std::string             file;
  std::string             logname;

  unsigned                counter = 1u;
};

#define ALICE_S(x) #x
#define ALICE_SX(x) ALICE_S(x)

#define ADD_COMMAND( name ) cli.insert_command( #name, std::make_shared<name##_command>( cli.env ) );
#define ADD_READ_COMMAND( name, label ) cli.insert_read_command<io_##name##_tag_t>( "read_" ALICE_SX(name), label );
#define ADD_WRITE_COMMAND( name, label ) cli.insert_write_command<io_##name##_tag_t>( "write_" ALICE_SX(name), label );

#ifndef ALICE_PYTHON
#define ALICE_BEGIN(name) int main( int argc, char ** argv ) {
#define ALICE_END return cli.run( argc, argv ); }
#else
#define ALICE_BEGIN(name) PYBIND11_PLUGIN(name) {
#define ALICE_END return cli.pymodule().ptr(); }
#endif

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
