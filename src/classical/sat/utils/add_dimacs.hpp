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

#include <regex>
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
               { { std::regex( "p cnf (\\d+) (\\d+)" ), [&]( const std::smatch& m ) {
                     num_vars    = boost::lexical_cast<unsigned>( m[1] );
                     num_clauses = boost::lexical_cast<unsigned>( m[2] );
                   } },
                 { std::regex( "^-?\\d.*$" ), [&]( const std::smatch& m ) {
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
