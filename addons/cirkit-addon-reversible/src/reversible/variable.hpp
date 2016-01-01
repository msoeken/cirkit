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
 * @file variable.hpp
 *
 * @brief All about lines and variables
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <iostream>
#include <string>

namespace cirkit
{

  struct variable
  {
  public:
    explicit variable( unsigned v );
    variable( const std::string& s );

    unsigned line() const;
    bool polarity() const;

    void set_line( unsigned l );
    void set_polarity( bool p );

    friend bool operator==( variable v1, variable v2 );
    friend bool operator<( variable v1, variable v2 );

  private:
    unsigned v;
  };

  std::ostream& operator<<( std::ostream& os, const variable& var );
  variable make_var( unsigned line, bool polarity = true );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
