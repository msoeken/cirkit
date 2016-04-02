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
 * @file write_io.hpp
 *
 * @brief Generic command for writing I/O
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_WRITE_IO_COMMAND_HPP
#define CLI_WRITE_IO_COMMAND_HPP

#include <string>

#include <core/cli/command.hpp>

namespace cirkit
{

template<typename Tag, typename S>
int add_write_io_option_helper( program_options& opts )
{
  if ( store_can_write_io_type<S, Tag>( opts ) )
  {
    add_option_helper<S>( opts );
  }

  return 0;
}

template<typename Tag, typename S>
int write_io_helper( program_options& opts, const environment::ptr& env, const std::string& filename, const properties::ptr& settings )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( opts.is_set( option ) )
  {
    if ( env->store<S>().current_index() == -1 )
    {
      std::cout << "[w] no " << name << " selected in store" << std::endl;
    }
    else
    {
      store_write_io_type<S, Tag>( env->store<S>().current(), filename, opts, settings );
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
    [](...){}( add_write_io_option_helper<Tag, S>( opts )... );

    opts.add_options()
      ( "filename", value( &filename ), "Filename" )
      ;

    be_verbose();
  }

protected:
  bool execute()
  {
    auto settings = make_settings();

    [](...){}( write_io_helper<Tag, S>( opts, env, filename, settings )... );

    return true;
  }

private:
  std::string filename;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
