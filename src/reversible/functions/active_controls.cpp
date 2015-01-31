/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "active_controls.hpp"

#include <boost/range/algorithm.hpp>

namespace cirkit
{
  class active_controls::priv
  {
  public:
    priv() {}

    gate::control_container _active_controls;
  };

  active_controls::active_controls()
    : d( new priv() )
  {
  }

  active_controls::~active_controls()
  {
    //delete d;
  }

  void active_controls::add( variable control )
  {
    d->_active_controls.push_back( control );
  }

  void active_controls::remove( variable control )
  {
    d->_active_controls.erase( boost::remove( d->_active_controls, control ) );
  }

  const gate::control_container& active_controls::controls() const
  {
    return d->_active_controls;
  }

  void active_controls::operator()( gate& g ) const
  {
    boost::for_each( d->_active_controls, [&g]( variable c ) { g.add_control( c ); } );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
