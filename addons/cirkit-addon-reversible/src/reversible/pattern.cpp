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

#include "pattern.hpp"

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace cirkit
{

  class pattern::priv
  {
  public:
    priv() {}

    initializer_map initializers;
    std::vector<std::string> inputs;
    std::vector<std::vector<unsigned> > patterns;
  };

  pattern::pattern()
    : d( new priv() )
  {
  }

  pattern::~pattern()
  {
    delete d;
  }

  void pattern::add_initializer( const std::string& name, unsigned value )
  {
    d->initializers[name] = value;
  }

  void pattern::add_input( const std::string& name )
  {
    d->inputs += name;
  }

  void pattern::add_pattern( const std::vector<unsigned>& pattern )
  {
    d->patterns += pattern;
  }

  const pattern::initializer_map& pattern::initializers() const
  {
    return d->initializers;
  }

  const std::vector<std::string>& pattern::inputs() const
  {
    return d->inputs;
  }

  const pattern::pattern_vec& pattern::patterns() const
  {
    return d->patterns;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
