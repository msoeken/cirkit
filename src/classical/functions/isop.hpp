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
 * @file isop.hpp
 *
 * @brief Compute ISOP with truth tables
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef TT_ISOP_HPP
#define TT_ISOP_HPP

#include <vector>

#include <core/cube.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

/* based on ABC's Abc_Tt6IsopCover */
tt tt_isop( const tt& on, const tt& ondc, std::vector<int>& cover );

/* based on ABC's Abc_Tt6Cnf */
std::vector<int> tt_cnf( const tt& f );
void tt_cnf( const tt& f, std::vector<int>& cover );

/* converters to cube_vec_t */
cube_vec_t cover_to_cubes( const std::vector<int>& cover, unsigned num_vars );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
