/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "transformation_based_synthesis.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/fully_specified.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/utils/truth_table_helpers.hpp>

#include "synthesis_utils_p.hpp"

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

enum direction_t
{
  direction_back, direction_front
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

gate::control_container get_control_lines_from_mask( const boost::dynamic_bitset<>& mask )
{
  gate::control_container controls;
  boost::dynamic_bitset<>::size_type pos = mask.find_first();
  while ( pos != boost::dynamic_bitset<>::npos )
  {
    controls += make_var( mask.size() - 1u - pos );
    pos = mask.find_next( pos );
  }
  return controls;
}

bitset_pair_vector_t& sort_truth_table( bitset_pair_vector_t& tt )
{
  typedef bitset_pair_vector_t::value_type value_type;
  return boost::sort( tt, []( const value_type& p1, const value_type& p2 ) { return p1.first < p2.first; } );
}

unsigned hamming_distance( const std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>>& p )
{
  return ( p.first ^ p.second ).count();
}

void basic_first_step( circuit& circ, bitset_pair_vector_t& tt )
{
  boost::dynamic_bitset<>::size_type bpos = tt[0].second.find_first();

  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    prepend_not( circ, circ.lines() - 1u - bpos );
    for ( auto& p : tt )
    {
      p.second.flip( bpos );
    }
    bpos = tt[0].second.find_next( bpos );
  }
}

void insert_gate( circuit& circ, unsigned&pos, boost::dynamic_bitset<> controls, unsigned target, bitset_pair_vector_t& tt, direction_t dir )
{
  insert_toffoli( circ, pos, get_control_lines_from_mask( controls ), circ.lines() - 1u - target );

  for ( auto& p : tt )
  {
    auto& bits = ( dir == direction_back ) ? p.second : p.first;
    if ( ( bits & controls ) == controls )
    {
      bits.flip( target );
    }
  }

  if ( dir == direction_front )
  {
    ++pos;
    tt = sort_truth_table( tt );
  }
}

void adjust_line( circuit& circ, unsigned& pos, bitset_pair_vector_t& tt, unsigned line, direction_t dir )
{
  unsigned bw = circ.lines();

  auto p = ( tt[line].first ^ tt[line].second ) & ( dir == direction_back ? tt[line].first : tt[line].second );
  auto q = ( tt[line].first ^ tt[line].second ) & ( dir == direction_back ? tt[line].second : tt[line].first );

  /* change 0 -> 1 */
  auto bpos = p.find_first();
  auto mask = dir == direction_back ? tt[line].second : tt[line].first;
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    insert_gate( circ, pos, mask, bpos, tt, dir );
    mask.set( bpos );
    bpos = p.find_next( bpos );
  }

  /* change 1 -> 0 */
  bpos = q.find_first();
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    mask.reset( bpos );
    insert_gate( circ, pos, mask, bpos, tt, dir );
    bpos = q.find_next( bpos );
  }
}

void print_current_state( unsigned index, const circuit& circ, const bitset_pair_vector_t& tt )
{
  std::cout << "[i] state at index " << index << std::endl;
  std::cout << "[i] current circuit: " << std::endl << circ << std::endl;
  std::cout << "[i] current spec: " << std::endl;
  for ( auto p : tt )
  {
    std::cout << p.first << " |-> " << p.second << std::endl;
  }
  std::cout << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool transformation_based_synthesis( circuit& circ, const binary_truth_table& spec,
                                     properties::ptr settings,
                                     properties::ptr statistics )
{
  /* Settings */
  bool bidirectional = get( settings, "bidirectional", true  );
  bool verbose       = get( settings, "verbose",       false );

  timer<properties_timer> t;

  if ( statistics )
  {
    properties_timer rt( statistics );
    t.start( rt );
  }

  /* circuit has to be empty */
  clear_circuit( circ );

  /* truth table has to be fully specified */
  if ( !fully_specified( spec ) )
  {
    set_error_message( statistics, "truth table `spec` is not fully specified." );
    return false;
  }

  /* truth table to bitsets */
  bitset_pair_vector_t tt = truth_table_to_bitset_pair_vector( spec );
  sort_truth_table( tt );

  unsigned bw = spec.num_outputs();
  circ.set_lines( bw );

  /* copy metadata */
  copy_metadata( spec, circ );

  /* Step 1 */
  if ( !bidirectional )
  {
    if ( verbose )
    {
      print_current_state( 0u, circ, tt );
    }

    basic_first_step( circ, tt );
  }

  /* Step 2 */
  unsigned start_index = bidirectional ? 0u : 1u;
  unsigned pos = 0u;

  direction_t dir = direction_back;
  unsigned index = 0u;

  for ( unsigned i = start_index; i < tt.size(); ++i )
  {
    if ( verbose )
    {
      print_current_state( i, circ, tt );
    }

    if ( tt[i].first == tt[i].second )
    {
      continue;
    }

    dir = direction_back;
    index = i;
    if ( bidirectional )
    {
      typedef bitset_pair_vector_t::value_type value_type;
      unsigned other_index = boost::find_if( tt, [&tt, &i]( const value_type& p ) { return p.second == tt[i].first; } ) - tt.begin();
      if ( hamming_distance( tt[other_index] ) < hamming_distance( tt[i] ) )
      {
        dir = direction_front;
        index = other_index;
      }
    }

    if ( verbose )
    {
      std::cout << "[i] adjust line: " << index << std::endl;
    }
    adjust_line( circ, pos, tt, index, dir );
  }

  return true;
}

truth_table_synthesis_func transformation_based_synthesis_func( properties::ptr settings,
                                                                properties::ptr statistics )
{
  truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
    return transformation_based_synthesis( circ, spec, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
