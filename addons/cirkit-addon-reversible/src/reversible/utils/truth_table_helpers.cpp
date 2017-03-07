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

#include "truth_table_helpers.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost::assign;

namespace cirkit
{

bitset_vector_t truth_table_to_bitset_vector( const binary_truth_table& spec )
{
  /* Number of variables */
  unsigned n = spec.num_inputs();

  bitset_vector_t vec( 1u << n );

  /* Fill vector */
  for ( const auto& row : spec )
  {
    boost::dynamic_bitset<> inv( n ), outv( n );

    unsigned pos = 0u;
    for ( const auto& in : boost::make_iterator_range( row.first ) )
    {
      inv[n - pos++ - 1u] = *in;
    }

    pos = 0u;
    for ( const auto& out : boost::make_iterator_range( row.second ) )
    {
      outv[n - pos++ - 1u] = *out;
    }

    vec[inv.to_ulong()] = outv;
  }

  return vec;
}

bitset_pair_vector_t truth_table_to_bitset_pair_vector( const binary_truth_table& spec )
{
  /* Number of variables */
  unsigned n = spec.num_inputs();

  bitset_pair_vector_t vec;

  /* Fill vector */
  for ( const auto& row : spec )
  {
    boost::dynamic_bitset<> inv( n ), outv( n );

    unsigned pos = 0u;
    for ( const auto& in : boost::make_iterator_range( row.first ) )
    {
      inv[n - pos++ - 1u] = *in;
    }

    pos = 0u;
    for ( const auto& out : boost::make_iterator_range( row.second ) )
    {
      outv[n - pos++ - 1u] = *out;
    }

    vec += std::make_pair( inv, outv );
  }

  return vec;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
