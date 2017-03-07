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
 * @file npn_canonization.hpp
 *
 * @brief Computes an NPN canonization for a truth table
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef NPN_CANONIZATION_HPP
#define NPN_CANONIZATION_HPP

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

tt exact_npn_canonization(
    const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
    const properties::ptr& settings = properties::ptr(),
    const properties::ptr& statistics = properties::ptr() );

tt npn_canonization( const tt& t, boost::dynamic_bitset<>& phase,
                     std::vector<unsigned>& perm,
                     const properties::ptr& settings = properties::ptr(),
                     const properties::ptr& statistics = properties::ptr() );

tt npn_canonization_flip_swap( const tt& t, boost::dynamic_bitset<>& phase,
                               std::vector<unsigned>& perm,
                               const properties::ptr& settings = properties::ptr(),
                               const properties::ptr& statistics = properties::ptr() );

tt npn_canonization_sifting( const tt& t, boost::dynamic_bitset<>& phase,
                             std::vector<unsigned>& perm,
                             const properties::ptr& settings = properties::ptr(),
                             const properties::ptr& statistics = properties::ptr() );

tt tt_from_npn( const tt& npn, const boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );

/* ABC methods */
tt npn_canonization_lucky( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
