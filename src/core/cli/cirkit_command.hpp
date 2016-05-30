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

#include <lscli/command.hpp>

#include <core/properties.hpp>

namespace cirkit
{

using namespace alice;

class cirkit_command : public command
{
public:
  cirkit_command( const environment::ptr& env, const std::string& caption, const std::string& publications = std::string() );

  virtual bool run( const std::vector<std::string>& args );

protected:
  /* pre-defined options */
  inline void be_verbose() { opts.add_options()( "verbose,v", "be verbose" ); }
  inline bool is_verbose() const { return is_set( "verbose" ); }

  inline void add_new_option( bool with_short = true )
  {
    auto option = std::string( "new" );
    if ( with_short )
    {
      option += ",n";
    }
    opts.add_options()( option.c_str(), "create new store entry" );
  }

  template<typename Store>
  inline void extend_if_new( Store& store )
  {
    if ( store.empty() || is_set( "new" ) )
    {
      store.extend();
    }
  }

  /* get settings with often-used pre-defined options */
  properties::ptr make_settings() const;

  void print_runtime( const std::string& key = "runtime", const std::string& label = std::string() ) const;

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
