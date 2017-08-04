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
 * @file permutation.hpp
 *
 * @brief Permutation helpers
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef PERMUTATION_HPP
#define PERMUTATION_HPP

#include <vector>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>

namespace cirkit
{

using permutation_t = std::vector<unsigned>;
using cycles_t      = std::vector<std::vector<unsigned>>;

permutation_t identity_permutation( unsigned size );
permutation_t truth_table_to_permutation( const binary_truth_table& spec );
permutation_t circuit_to_permutation( const circuit& circ );
cycles_t permutation_to_cycles( const permutation_t& perm, bool sort = true );
std::vector<std::pair<unsigned, unsigned>> permutation_to_transpositions( const permutation_t& perm );
unsigned permutation_inv( const permutation_t& perm );
int permutation_sign( const permutation_t& perm );
permutation_t permutation_multiply( const permutation_t& a, const permutation_t& b );
permutation_t permutation_invert( const permutation_t& perm );
std::vector<unsigned> cycles_type( const cycles_t& cycles );
bool is_involution( const permutation_t& perm );
bool is_simple( const permutation_t& perm );

std::string permutation_to_string( const permutation_t& perm );
std::string cycles_to_string( const cycles_t& cycles, bool print_fixpoints = false );
std::string cycles_to_string( const permutation_t& perm, bool print_fixpoints = false );
std::string type_to_string( const std::vector<unsigned>& type );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
