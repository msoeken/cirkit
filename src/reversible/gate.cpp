/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "gate.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

namespace cirkit
{
  using namespace boost::assign;

  class gate::priv
  {
  public:
    priv() {}

    priv( priv& other )
      : controls( other.controls ),
        targets( other.targets ),
        target_type( other.target_type ) {}

      control_container controls;
      target_container targets;
      boost::any     target_type;
  };


  gate::gate()
    : d( new priv() )
  {
  }

  gate::gate( const gate& other )
    : d( new priv() )
  {
    operator=( other );
  }

  gate::~gate()
  {
    delete d;
  }

  gate& gate::operator=( const gate& other )
  {
    if ( this != &other )
    {
      d->controls.clear();
      boost::push_back( d->controls, other.controls() );
      d->targets.clear();
      boost::push_back( d->targets, other.targets() );
      d->target_type = other.type();
    }
    return *this;
  }

    gate::control_container gate::controls() const
  {
    return d->controls;
  }

  gate::target_container gate::targets() const
  {
    return d->targets;
  }

  unsigned gate::size() const
  {
    return d->controls.size() + d->targets.size();
  }

  void gate::add_control( variable c )
  {
    d->controls += c;
  }

  void gate::remove_control( variable c )
  {
    d->controls.erase( boost::remove( d->controls, c ) );
  }

  void gate::add_target( unsigned l )
  {
    d->targets += l;
  }

  void gate::remove_target( unsigned l )
  {
    d->targets.erase( boost::remove( d->targets, l ) );
  }

  void gate::set_type( const boost::any& t )
  {
    d->target_type = t;
  }

  const boost::any& gate::type() const
  {
    return d->target_type;
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
