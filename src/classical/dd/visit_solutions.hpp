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

/**
 * @file visit_solutions.hpp
 *
 * @brief Visit all solutions of BDD
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef VISIT_SOLUTIONS_HPP
#define VISIT_SOLUTIONS_HPP

#include <functional>
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/logic/tribool.hpp>

#include <classical/dd/bdd.hpp>

BOOST_TRIBOOL_THIRD_STATE(dontcare)

namespace cirkit
{

using visit_solutions_func = std::function<void(const boost::dynamic_bitset<>&)>;
using visit_paths_func     = std::function<void(const std::vector<boost::tribool>&)>;

void visit_solutions( const bdd& n, const visit_solutions_func& f );
void visit_paths( const bdd& n, const visit_paths_func& f );

void print_paths( const bdd& n, std::ostream& os = std::cout );
void print_paths( const std::vector<bdd>& ns, std::ostream& os = std::cout );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
