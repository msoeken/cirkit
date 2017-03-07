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
 * @file aig_cone.hpp
 *
 * @brief Computes a new smaller AIG based on output cones
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_CONE_HPP
#define AIG_CONE_HPP

#include <string>
#include <vector>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/**
 * @brief Computes a new smaller AIG based on output cones
 *
 * This function takes a combinational AIG, extracts all cones based on given
 * output names and returns a new AIG that consists of only those outputs and
 * the union of all structural cones.  Names of primary inputs and primary outputs
 * are kept, but internal node ids are adjusted.
 *
 * @param aig   Source AIG
 * @param names Names of primary outputs
 * @param settings The following settings are possible
 *                 +---------+------+---------+
 *                 | Name    | Type | Default |
 *                 +---------+------+---------+
 *                 | verbose | bool | false   |
 *                 +---------+------+---------+
 * @param statistics The following statistics are given
 *                 +---------+--------+---------------------------+
 *                 | Name    | Type   | Description               |
 *                 +---------+--------+---------------------------+
 *                 | runtime | double | Runtime of the SAT solver |
 *                 +---------+--------+---------------------------+
 */
aig_graph aig_cone( const aig_graph& aig, const std::vector<std::string>& names,
                    const properties::ptr& settings = properties::ptr(),
                    const properties::ptr& statistics = properties::ptr() );

/**
 * @brief Computes a smaller AIG based on output cones
 *
 * Version that takes output indexes instead of names.
 */
aig_graph aig_cone( const aig_graph& aig, const std::vector<unsigned>& index,
                    const properties::ptr& settings = properties::ptr(),
                    const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
