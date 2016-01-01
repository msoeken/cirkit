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

#include "bus_collection.hpp"

#include <algorithm>
#include <cassert>

namespace cirkit
{

  class bus_collection::priv
  {
  public:
    priv() {}

    bus_collection::map buses;
    std::map<std::string, unsigned> initial_values;
  };

  bus_collection::bus_collection()
    : d( new priv() )
  {
  }

  bus_collection::bus_collection( const bus_collection& other )
    : d( new priv( *other.d ) )
  {
  }

  bus_collection::~bus_collection()
  {
    /* HINT this caused problems in the past */
    delete d;
  }

  bus_collection& bus_collection::operator=( const bus_collection& other )
  {
    *d = *other.d;
    return *this;
  }

  void bus_collection::add( const map::key_type& name, const map::mapped_type& line_indices, const boost::optional<unsigned>& initial_value )
  {
    d->buses.insert( std::make_pair( name, line_indices ) );

    if ( initial_value )
    {
      d->initial_values[name] = *initial_value;
    }
  }

  const bus_collection::map::mapped_type& bus_collection::get( const map::key_type& name ) const
  {
    map::const_iterator it = d->buses.find( name );

    if ( it != d->buses.end() )
    {
      return it->second;
    }
    else
    {
      assert( false );
    }
  }

  const bus_collection::map& bus_collection::buses() const
  {
    return d->buses;
  }

  bus_collection::map::key_type bus_collection::find_bus( map::mapped_type::value_type line_index ) const
  {
    for ( const auto& p : d->buses )
    {
      if ( std::find( p.second.begin(), p.second.end(), line_index ) != p.second.end() )
      {
        return p.first;
      }
    }

    return map::key_type();
  }

  bool bus_collection::has_bus( map::mapped_type::value_type line_index ) const
  {
    for ( const auto& p : d->buses )
    {
      if ( std::find( p.second.begin(), p.second.end(), line_index ) != p.second.end() )
      {
        return true;
      }
    }

    return false;
  }

  unsigned bus_collection::signal_index( unsigned line_index ) const
  {
    for ( const auto& p : d->buses )
    {
      map::mapped_type::const_iterator it = std::find( p.second.begin(), p.second.end(), line_index );
      if ( it != p.second.end() )
      {
        return std::distance( p.second.begin(), it );
      }
    }

    assert( false );
  }

  void bus_collection::set_initial_value( const std::string& name, unsigned initial_value )
  {
    map::const_iterator it = d->buses.find( name );

    if ( it != d->buses.end() )
    {
      d->initial_values[name] = initial_value;
    }
  }

  boost::optional<unsigned> bus_collection::initial_value( const std::string& name ) const
  {
    map::const_iterator it = d->buses.find( name );

    if ( it != d->buses.end() )
    {
      std::map<std::string, unsigned>::const_iterator it2 = d->initial_values.find( name );
      if ( it2 != d->initial_values.end() )
      {
        return it2->second;
      }
      else
      {
        return boost::optional<unsigned>();
      }
    }
    else
    {
      return boost::optional<unsigned>();
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
