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

#include "permutation.hpp"

#include <numeric>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/range_utils.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/simulation/simple_simulation.hpp>

using namespace boost::assign;
using boost::adaptors::transformed;

namespace cirkit
{

permutation_t identity_permutation( unsigned size )
{
  permutation_t perm( size );
  std::iota( perm.begin(), perm.end(), 0u );
  return perm;
}

permutation_t truth_table_to_permutation( const binary_truth_table& spec )
{
  permutation_t perm;

  for ( const auto& row : index( spec ) )
  {
    auto from = row.value.first;
    auto to   = row.value.second;

    assert( truth_table_cube_to_number( binary_truth_table::cube_type( from.first, from.second ) ) == row.index );
    perm += truth_table_cube_to_number( binary_truth_table::cube_type( to.first, to.second ) );
  }

  return perm;
}

permutation_t circuit_to_permutation( const circuit& circ )
{
  binary_truth_table spec;
  circuit_to_truth_table( circ, spec, simple_simulation_func() );
  return truth_table_to_permutation( spec );
}

cycles_t permutation_to_cycles( const permutation_t& perm, bool sort )
{
  cycles_t cycles;

  boost::dynamic_bitset<> vismask = ~boost::dynamic_bitset<>( perm.size() );
  unsigned start, current;

  while ( vismask.any() )
  {
    start = vismask.find_first();
    permutation_t cycle;
    cycle += start;
    current = perm[start];
    vismask.reset( current );

    while ( current != start ) {
      cycle += current;
      current = perm[current];
      vismask.reset( current );
    }

    cycles += cycle;
  }

  assert( vismask.none() );

  if ( sort )
  {
    boost::sort( cycles, []( const std::vector<unsigned>& x1, const std::vector<unsigned>& x2 ) { return x1.size() > x2.size(); } );
  }

  return cycles;
}

std::vector<std::pair<unsigned, unsigned>> permutation_to_transpositions( const permutation_t& perm )
{
  auto perm_copy = perm;
  std::vector<std::pair<unsigned, unsigned>> transpositions;

  for ( int i = perm_copy.size() - 1; i > 0; --i )
  {
    if ( static_cast<int>( perm_copy[i] ) == i ) { continue; }

    /* where is i? */
    auto other = std::distance( perm_copy.begin(), std::find( perm_copy.begin(), perm_copy.begin() + i, i ) );
    transpositions.push_back( {i, other} );

    std::swap( perm_copy[i], perm_copy[other] );
  }

  return transpositions;
}

unsigned permutation_inv( const permutation_t& perm )
{
  unsigned inv = 0u;

  for ( unsigned i = 0u; i < perm.size() - 1u; ++i )
  {
    for ( unsigned j = i + 1u; j < perm.size(); ++j )
    {
      if ( perm[i] > perm[j] )
      {
        ++inv;
      }
    }
  }

  return inv;
}

int permutation_sign( const permutation_t& perm )
{
  return ( permutation_inv( perm ) % 2 == 0u ) ? 1 : -1;
}

permutation_t permutation_multiply( const permutation_t& a, const permutation_t& b )
{
  assert( a.size() == b.size() );
  permutation_t result( a.size() );

  for ( auto i = 0u; i < a.size(); ++i )
  {
    result[i] = b[a[i]];
  }

  return result;
}

permutation_t permutation_invert( const permutation_t& perm )
{
  permutation_t result( perm.size() );

  for ( auto i = 0u; i < perm.size(); ++i )
  {
    result[perm[i]] = i;
  }

  return result;
}

std::vector<unsigned> cycles_type( const cycles_t& cycles )
{
  std::vector<unsigned> type( cycles.size() );
  boost::transform( cycles, type.begin(), []( const permutation_t& cycle ) { return cycle.size(); } );
  boost::sort( type );
  return type;
}

bool is_involution( const permutation_t& perm )
{
  auto c = permutation_to_cycles( perm );
  return boost::find_if( c, []( const std::vector<unsigned>& cycle ) { return cycle.size() > 2u; } ) == c.end();
}

inline unsigned pos( unsigned i, unsigned j, unsigned n )
{
  return i * n + ( i * ( i + 1 ) ) / 2 + j;
}

/* dynamic programming algorithm to check whether a permutation is
   simple.

   [M.H. Albert, M.D. Atkinson, M. Klazar, Journal of Integer Sequences 6 (2003), 03.4.4]
*/
bool is_simple( const permutation_t& perm )
{
  const unsigned n = perm.size();
  const auto sumn  = ( n * ( n + 1 ) ) / 2;

  std::vector<unsigned> m_min( sumn );
  std::vector<unsigned> m_max( sumn );

  auto p = 0u;
  for ( auto i = 0u; i < n; ++i )
  {
    m_min[p] = m_max[p] = perm.at( i );

    for ( auto j = ( i + 1u ); j < n; ++j )
    {
      m_min[p + 1u] = std::min( m_min[p], perm.at( j ) );
      m_max[p + 1u] = std::max( m_max[p], perm.at( j ) );
      ++p;
    }

    ++p;
  }

  p = 0u;
  for ( auto i = 0u; i < ( n - 1u ); ++i )
  {
    ++p;
    for ( auto j = ( i + 1u ); j < n; ++j )
    {
      if ( ( m_max[p] - m_min[p] ) == ( j - i ) && !( ( i == 0u ) && ( j == ( n - 1u ) ) ) )
      {
        return false;
      }
      ++p;
    }
  }

  return true;
}

std::string permutation_to_string( const permutation_t& perm )
{
  return "[" + any_join( perm, " " ) + "]";
}

std::string cycles_to_string( const cycles_t& cycles, bool print_fixpoints )
{
  return boost::join( cycles | transformed( [&print_fixpoints]( const permutation_t& cycle ) {
        return ( cycle.size() > 1u || print_fixpoints ) ? ( "(" + any_join( cycle, " " ) + ")" ) : std::string();
      } ), "" );
}

std::string cycles_to_string( const permutation_t& perm, bool print_fixpoints )
{
  return cycles_to_string( permutation_to_cycles( perm ), print_fixpoints );
}

std::string type_to_string( const std::vector<unsigned>& type )
{
  return "(" + any_join( type, ", " ) + ")";
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
