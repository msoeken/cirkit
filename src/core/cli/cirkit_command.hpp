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
 * @file cirkit_command.hpp
 *
 * @brief CirKit command
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CIRKIT_COMMAND_HPP
#define CIRKIT_COMMAND_HPP

#include <core/properties.hpp>
#include <core/cli/command.hpp>

namespace cirkit
{

class cirkit_command : public command
{
public:
  cirkit_command( const environment::ptr& env, const std::string& caption, const std::string& publications = std::string() );

  virtual bool run( const std::vector<std::string>& args );

protected:
  /* pre-defined options */
  inline void be_verbose() { opts.add_options()( "verbose,v", "Be verbose" ); }
  inline bool is_verbose() const { return is_set( "verbose" ); }

  /* get settings with often-used pre-defined options */
  properties::ptr make_settings() const;

  void print_runtime() const;

protected:
  properties::ptr statistics;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
