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

void insert_gate( circuit& circ, unsigned& pos, const boost::dynamic_bitset<>& mask, unsigned target_line, unsigned current_line, bitset_vector_t& output_values )
{
  auto controls = get_control_lines_from_mask( mask );

  insert_toffoli( circ, pos, controls, target_line );

  for ( unsigned i = current_line; i < output_values.size(); ++i )
  {
    if ( ( output_values.at( i ) & mask ) == mask )
    {
      output_values[i].flip( circ.lines() - 1 - target_line );
    }
  }
}

void insert_gate_front( circuit& circ, unsigned& pos, const boost::dynamic_bitset<>& mask, unsigned target_line, unsigned current_line, bitset_vector_t& output_values )
{
  auto controls = get_control_lines_from_mask( mask );

  insert_toffoli( circ, pos, controls, target_line );

  for ( unsigned i = current_line; i < output_values.size(); ++i )
  {
    if ( ( boost::dynamic_bitset<>( circ.lines(), i ) & mask ) == mask )
    {
      unsigned otherpos = i ^ ( 1u << ( circ.lines() - 1 - target_line ) );
      std::cout << "otherpos = " << otherpos << " i = " << i << std::endl;
      if ( otherpos > i )
      {
        std::cout << "[i] swap" << std::endl;
        output_values[i].swap( output_values[otherpos] );
      }
    }
  }
}

void basic_first_step( circuit& circ, bitset_vector_t& output_values )
{
  for ( unsigned i = 0; i < circ.lines(); ++i )
  {
    if ( output_values[0u][i] )
    {
      prepend_not( circ, i );
      for ( auto& output : output_values )
      {
        output.flip( i );
      }
    }
  }
}

void basic_first_step2( circuit& circ, bitset_pair_vector_t& tt )
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

void insert_from_back( circuit& circ, unsigned& pos, unsigned line, bitset_vector_t& output_values )
{
  unsigned bw = circ.lines();
  boost::dynamic_bitset<> bline( bw, line ), mask;

  /* change 0 -> 1 */
  boost::dynamic_bitset<> p = ( bline ^ output_values[line] ) & bline;
  boost::dynamic_bitset<>::size_type bpos = p.find_first();
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    mask = output_values[line];
    mask.reset( bpos );
    insert_gate( circ, pos, mask, ( bw - 1u - bpos ), line, output_values );
    bpos = p.find_next( bpos );
  }

  /* change 1 -> 0 */
  boost::dynamic_bitset<> q = ( bline ^ output_values[line] ) & output_values[line];
  bpos = q.find_first();
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    mask = output_values[line];
    mask.reset( bpos );
    insert_gate( circ, pos, mask, ( bw - 1u - bpos ), line, output_values );
    bpos = q.find_next( bpos );
  }
}

void insert_gate2( circuit& circ, unsigned&pos, boost::dynamic_bitset<> controls, unsigned target, bitset_pair_vector_t& tt, direction_t dir )
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

  /* change 0 -> 1 */
  auto p = ( tt[line].first ^ tt[line].second ) & ( dir == direction_back ? tt[line].first : tt[line].second );
  auto bpos = p.find_first();
  auto mask = dir == direction_back ? tt[line].second : tt[line].first;
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    insert_gate2( circ, pos, mask, bpos, tt, dir );
    mask.set( bpos );
    bpos = p.find_next( bpos );
  }

  /* change 1 -> 0 */
  auto q = ( tt[line].first ^ tt[line].second ) & ( dir == direction_back ? tt[line].second : tt[line].first );
  bpos = q.find_first();
  //mask = dir == direction_back ? tt[line].second : tt[line].first;
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    mask.reset( bpos );
    insert_gate2( circ, pos, mask, bpos, tt, dir );
    bpos = q.find_next( bpos );
  }
}

void insert_from_front( circuit& circ, unsigned& pos, unsigned line, bitset_vector_t& output_values )
{
  unsigned bw = circ.lines();
  boost::dynamic_bitset<> bline( bw, line ), mask;
  boost::dynamic_bitset<>::size_type bpos;

  while ( bline != output_values[line] )
  {
    auto p = ( bline ^ output_values[line] ) & output_values[line];
    auto q = ( bline ^ output_values[line] ) & bline;

    auto value = output_values[line];

    // change 0 -> 1
    bpos = p.find_first();
    if ( bpos != boost::dynamic_bitset<>::npos )
    {
      mask = bline;
      bline.reset( bpos );
      insert_gate_front( circ, pos, mask, bw - 1u - bpos, 0u, output_values );
      ++pos;
    }

    // change 1 -> 0
    if ( p.none() && q.any() )
    {
      bpos = q.find_first();
      if ( bpos != boost::dynamic_bitset<>::npos )
      {
        mask = bline;
        bline.reset( bpos );
        insert_gate_front( circ, pos, mask, bw - 1u - bpos, 0u, output_values );
        ++pos;
      }
    }

    line = boost::find( output_values, value ) - output_values.begin();
    bline = boost::dynamic_bitset<>( bw, line );
  }
}

void print_current_state( unsigned index, const circuit& circ, const bitset_vector_t& output_values )
{
  std::cout << "[i] state at index " << index << std::endl;
  std::cout << "[i] current circuit: " << std::endl << circ << std::endl;
  std::cout << "[i] current spec: " << std::endl << any_join( output_values, "\n" ) << std::endl << std::endl;
}

void print_current_state2( unsigned index, const circuit& circ, const bitset_pair_vector_t& tt )
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
  bitset_vector_t output_values = truth_table_to_bitset_vector( spec );

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
      print_current_state2( 0u, circ, tt );
    }

    /*basic_first_step( circ, output_values );*/
    basic_first_step2( circ, tt );
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
      print_current_state2( i, circ, tt );
    }

    /*if ( boost::dynamic_bitset<>( bw, i ) == output_values[i] )
    {
      continue;
      }*/
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

    // NOTE maybe have two for loops so that the check has not to be done every time
    /*if ( bidirectional )
    {
      index = boost::find( output_values, boost::dynamic_bitset<>( bw, i ) ) - output_values.begin();
      std::cout << "[i] index: " << index << std::endl;
      from_back = hamming_distance( boost::dynamic_bitset<>( bw, index ), output_values[index] )
               >= hamming_distance( boost::dynamic_bitset<>( bw, i ), output_values[i] );
               }*/

    /*if ( from_back )
      {*/
      //        std::cout << "insert from back (line: " << i << ")" << std::endl;
      //insert_from_back( circ, pos, i, output_values );
    if ( verbose )
    {
      std::cout << "[i] adjust line: " << index << std::endl;
    }
    adjust_line( circ, pos, tt, index, dir );
      /*}
    else
    {*/
      //        std::cout << "insert from font (line: " << i << ", index: " << index << ")" << std::endl;
      //insert_from_front( circ, pos, index, output_values );
      // }
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
