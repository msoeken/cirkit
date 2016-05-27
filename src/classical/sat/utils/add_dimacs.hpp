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
 * @file add_dimacs.hpp
 *
 * @brief Adds clauses based from DIMACS file
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef ADD_DIMACS_HPP
#define ADD_DIMACS_HPP

#include <boost/regex.hpp>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/string_utils.hpp>

#include <classical/sat/sat_solver.hpp>

namespace cirkit
{

template<class S>
int add_dimacs( S& solver, const std::string& filename, int sid, unsigned * pnum_vars = nullptr, unsigned * pnum_clauses = nullptr )
{
  using boost::adaptors::transformed;

  auto offset = sid - 1;
  unsigned num_clauses, num_vars, count = 0u;
  line_parser( filename,
               { { boost::regex( "p cnf (\\d+) (\\d+)" ), [&]( const boost::smatch& m ) {
                     num_vars    = boost::lexical_cast<unsigned>( m[1] );
                     num_clauses = boost::lexical_cast<unsigned>( m[2] );
                   } },
                 { boost::regex( "^-?\\d.*$" ), [&]( const boost::smatch& m ) {
                     if ( count < num_clauses )
                     {
                       std::vector<int> clause;
                       parse_string_list( clause, m[0] );
                       assert( clause.back() == 0u );
                       add_clause( solver )( boost::make_iterator_range( clause.begin(), clause.end() - 1 ) | transformed( [&offset]( int l ) { return l < 0 ? l - offset : l + offset; } ) );
                       ++count;
                     }
                   } }
               });

  if ( pnum_vars )
  {
    *pnum_vars = num_vars;
  }
  if ( pnum_clauses )
  {
    *pnum_clauses = num_clauses;
  }

  return sid + num_vars;
}


}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
