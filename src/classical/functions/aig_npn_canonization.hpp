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
 * @file aig_npn_canonization.hpp
 *
 * @brief NPN canonization using AIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_NPN_CANONIZATION_HPP
#define AIG_NPN_CANONIZATION_HPP

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/* requires a single-output AIG */
void aig_npn_canonization_flip_swap( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                     const properties::ptr& settings = properties::ptr(),
                                     const properties::ptr& statistics = properties::ptr() );

/* requires a single-output AIG */
void aig_npn_canonization_flip_swap_shared_miter( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                                  const properties::ptr& settings = properties::ptr(),
                                                  const properties::ptr& statistics = properties::ptr() );

/* requires a single-output AIG */
void aig_npn_canonization_sifting( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                   const properties::ptr& settings = properties::ptr(),
                                   const properties::ptr& statistics = properties::ptr() );

/* requires a single-output AIG */
void aig_npn_canonization_sifting_shared_miter( const aig_graph& aig, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm,
                                                const properties::ptr& settings = properties::ptr(),
                                                const properties::ptr& statistics = properties::ptr() );

/* canonicizes all outputs and builds a new AIG */
aig_graph aig_npn_canonization( const aig_graph& aig,
                                const properties::ptr& settings = properties::ptr(),
                                const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
