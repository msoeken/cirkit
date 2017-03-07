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

#include "variable.hpp"

#include <boost/lexical_cast.hpp>

namespace cirkit
{

  variable::variable( unsigned v ) : v( v ) {}
  variable::variable( const std::string& s )
  {
    bool polarity = s[0] != '!';
    unsigned line = boost::lexical_cast<unsigned>( polarity ? s : s.substr( 1u ) );
    v = 2 * line + (polarity ? 0u : 1u);
  }

  unsigned variable::line() const { return v >> 1u; }
  bool variable::polarity() const { return !(v % 2u); }
  void variable::set_line( unsigned l ) { v = ( l << 1u ) + !polarity(); }
  void variable::set_polarity( bool p ) { v = ( line() << 1u ) + !p; }

  bool operator==( variable v1, variable v2 )
  {
    return v1.v == v2.v;
  }

  bool operator<( variable v1, variable v2 )
  {
    return v1.v < v2.v;
  }

  std::ostream& operator<<( std::ostream& os, const variable& var )
  {
    os << (var.polarity() ? "" : "!") << var.line();
    return os;
  }

  variable make_var( unsigned line, bool polarity )
  {
    return variable( 2 * line + (polarity ? 0u : 1u) );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
