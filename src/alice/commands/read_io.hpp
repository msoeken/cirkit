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
 * @file read_io.hpp
 *
 * @brief Generic command for reading I/O
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <wordexp.h>
#include <string>

#include <boost/program_options.hpp>

#include <alice/command.hpp>
#include <alice/rules.hpp>

using namespace boost::program_options;

namespace alice
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
  if ( store_can_read_io_type<S, Tag>( cmd ) )
  {
    constexpr auto option = store_info<S>::option;

    option_count++;
    default_option = option;
    add_option_helper<S>( cmd.opts );
  }

  return 0;
}

template<typename Tag, typename S>
int read_io_helper( command& cmd, const std::string& default_option, const environment::ptr& env, const std::string& filename )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) || option == default_option )
  {
    if ( cmd.is_set( "new" ) || env->store<S>().empty() )
    {
      env->store<S>().extend();
    }

    env->store<S>().current() = store_read_io_type<S, Tag>( filename, cmd );
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
    rules.push_back( file_exists_if_set( *this, process_filename( filename ), "filename" ) );

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

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
