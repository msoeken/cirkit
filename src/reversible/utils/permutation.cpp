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

#include "permutation.hpp"

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

permutation_t truth_table_to_permutation( const binary_truth_table& spec )
{
  permutation_t perm;

  for ( const auto& row : index( spec ) )
  {
    auto from = row.second.first;
    auto to   = row.second.second;

    assert( truth_table_cube_to_number( binary_truth_table::cube_type( from.first, from.second ) ) == row.first );
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

std::vector<unsigned> cycles_type( const cycles_t& cycles )
{
  std::vector<unsigned> type;
  boost::push_back( type, cycles | transformed( []( const permutation_t& cycle ) { return cycle.size(); } ) );
  boost::sort( type );
  return type;
}

bool is_involution( const permutation_t& perm )
{
  auto c = permutation_to_cycles( perm );
  return boost::find_if( c, []( const std::vector<unsigned>& cycle ) { return cycle.size() > 2u; } ) == c.end();
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
