/* CirKit: A circuit toolkit
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

#include "gate.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

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
      d->controls = other.controls();
      d->targets = other.targets();
      d->target_type = other.type();
    }
    return *this;
  }

  gate::control_container& gate::controls() const
  {
    return d->controls;
  }

  gate::target_container& gate::targets() const
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
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
