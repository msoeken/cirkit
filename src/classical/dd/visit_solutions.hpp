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
