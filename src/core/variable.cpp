/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "variable.hpp"

#include <boost/lexical_cast.hpp>

namespace revkit
{

  variable::variable( unsigned v ) : v( v ) {}
  variable::variable( const std::string& s )
  {
    bool polarity = s[0] != '!';
    unsigned line = boost::lexical_cast<unsigned>( polarity ? s : s.substr( 1u ) );
    v = 2 * line + (polarity ? 0u : 1u);
  }

  unsigned variable::line() const { return v / 2u; }
  bool variable::polarity() const { return !(v % 2u); }

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
  }

  variable make_var( unsigned line, bool polarity )
  {
    return variable( 2 * line + (polarity ? 0u : 1u) );
  }

}

// Local Variables:
// c-basic-offset: 2
// End:
