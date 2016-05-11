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
 * @file read_io.hpp
 *
 * @brief Generic command for reading I/O
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_READ_IO_COMMAND_HPP
#define CLI_READ_IO_COMMAND_HPP

#include <wordexp.h>
#include <string>

#include <boost/program_options.hpp>

#include <lscli/command.hpp>
#include <lscli/rules.hpp>

using namespace boost::program_options;

namespace cirkit
{

std::string process_filename( const std::string& filename )
{
  std::string result;

  wordexp_t p;
  wordexp( filename.c_str(), &p, 0 );

  for ( auto i = 0; i < p.we_wordc; ++i )
  {
    if ( !result.empty() ) { result += " "; }
    result += std::string( p.we_wordv[i] );
  }

  wordfree( &p );

  return result;
}

template<typename Tag, typename S>
int add_read_io_option_helper( command& cmd, unsigned& option_count, std::string& default_option )
{
  if ( store_can_read_io_type<S, Tag>( cmd.get_options() ) )
  {
    constexpr auto option = store_info<S>::option;

    option_count++;
    default_option = option;
    add_option_helper<S>( cmd.get_options().opts );
  }

  return 0;
}

template<typename Tag, typename S>
int read_io_helper( command& cmd, const std::string& default_option, const environment::ptr& env, const std::string& filename )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( cmd.is_set( option ) || option == default_option )
  {
    if ( cmd.is_set( "new" ) || env->store<S>().empty() )
    {
      env->store<S>().extend();
    }

    env->store<S>().current() = store_read_io_type<S, Tag>( filename, cmd.get_options() );
  }
  return 0;
}

template<class Tag, class... S>
class read_io_command : public command
{
public:
  read_io_command( const environment::ptr& env, const std::string& name )
    : command( env, boost::str( boost::format( "Read %s file" ) % name ) )
  {
    [](...){}( add_read_io_option_helper<Tag, S>( *this, option_count, default_option )... );
    if ( option_count != 1u )
    {
      default_option.clear();
    }

    add_positional_option( "filename" );
    opts.add_options()
      ( "filename", value( &filename ), "filename" )
      ( "new,n",                        "create new store entry" )
      ;
  }

protected:
  rules_t validity_rules() const
  {
    rules_t rules;

    rules.push_back( {[this]() { return option_count == 1 || exactly_one_true_helper( { is_set( store_info<S>::option )... } ); }, "exactly one store needs to be specified" } );
    rules.push_back( file_exists( process_filename( filename ), "filename" ) );

    return rules;
  }

  bool execute()
  {
    [](...){}( read_io_helper<Tag, S>( *this, default_option, env, process_filename( filename ) )... );

    return true;
  }

private:
  std::string filename;
  unsigned    option_count = 0u;
  std::string default_option;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
