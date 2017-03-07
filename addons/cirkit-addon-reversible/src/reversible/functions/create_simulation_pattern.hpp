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
 * @file create_simulation_pattern.hpp
 *
 * @brief Create simulation pattern for sequential simulation
 *
 * @author Mathias Soeken
 * @since  1.2
 */

#ifndef CREATE_SIMULATION_PATTERN_HPP
#define CREATE_SIMULATION_PATTERN_HPP

#include <string>
#include <vector>
#include <map>

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

  class circuit;
  class pattern;

  /**
   * @brief Create simulation pattern for sequential simulation
   *
   * @param p Pattern class
   * @param circ Circuit to be simulated
   * @param sim Empty vector which is filled with input assignments for every step
   * @param init Map which contains the initial assignment for state signals
   * @param error If not null, the string targeted by the pointer is assigned with an error message
   *
   * @since  1.2
   */
  bool create_simulation_pattern( const pattern& p, const circuit& circ, std::vector<boost::dynamic_bitset<> >& sim, std::map<std::string, boost::dynamic_bitset<> >& init, std::string* error = 0 );

}

#endif /* CREATE_SIMULATION_PATTERN_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
