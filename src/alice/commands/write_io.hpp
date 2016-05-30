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
 * @file write_io.hpp
 *
 * @brief Generic command for writing I/O
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <string>

#include <boost/program_options.hpp>

#include <alice/command.hpp>

using namespace boost::program_options;

namespace alice
{

template<typename Tag, typename S>
int add_write_io_option_helper( command& cmd, unsigned& option_count, std::string& default_option )
{
  if ( store_can_write_io_type<S, Tag>( cmd ) )
  {
    constexpr auto option = store_info<S>::option;

    option_count++;
    default_option = option;
    add_option_helper<S>( cmd.opts );
  }

  return 0;
}

template<typename Tag, typename S>
int write_io_helper( command& cmd, const std::string& default_option, const environment::ptr& env, const std::string& filename )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( cmd.is_set( option ) || option == default_option )
  {
    if ( env->store<S>().current_index() == -1 )
    {
      std::cout << "[w] no " << name << " selected in store" << std::endl;
    }
    else
    {
      store_write_io_type<S, Tag>( env->store<S>().current(), filename, cmd );
    }
  }
  return 0;
}

template<class Tag, class... S>
class write_io_command : public command
{
public:
  write_io_command( const environment::ptr& env, const std::string& name )
    : command( env, boost::str( boost::format( "Write %s file" ) % name ) )
  {
    [](...){}( add_write_io_option_helper<Tag, S>( *this, option_count, default_option )... );
    if ( option_count != 1u )
    {
      default_option.clear();
    }

    add_positional_option( "filename" );
    opts.add_options()
      ( "filename", value( &filename ), "filename" )
      ;
  }

protected:
  rules_t validity_rules() const
  {
    rules_t rules;

    rules.push_back( {[this]() { return option_count == 1 || exactly_one_true_helper( { is_set( store_info<S>::option )... } ); }, "exactly one store needs to be specified" } );

    return rules;
  }

  bool execute()
  {
    [](...){}( write_io_helper<Tag, S>( *this, default_option, env, filename )... );

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
