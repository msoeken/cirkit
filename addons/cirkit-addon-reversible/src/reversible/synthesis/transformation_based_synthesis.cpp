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

#include "transformation_based_synthesis.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/bitset_utils.hpp>
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

/* returns true if all bits have been checked and false if while loop was broken */
bool foreach_bit( const boost::dynamic_bitset<>& bits, std::function<bool(boost::dynamic_bitset<>::size_type)> func )
{
  auto bpos = bits.find_first();
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    if ( !func( bpos ) ) return false;
    bpos = bits.find_next( bpos );
  }
  return true;
}

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

void insert_toffoli_gate( circuit& circ, unsigned& pos, boost::dynamic_bitset<> controls, unsigned target, bitset_pair_vector_t& tt, direction_t dir )
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

void insert_fredkin_gate( circuit& circ, unsigned& pos, boost::dynamic_bitset<> controls, unsigned t1, unsigned t2, bitset_pair_vector_t& tt, direction_t dir )
{
  insert_fredkin( circ, pos, get_control_lines_from_mask( controls ), circ.lines() - 1u - t1, circ.lines() - 1u - t2 );

  for ( auto& p : tt )
  {
    auto& bits = ( dir == direction_back ) ? p.second : p.first;
    if ( ( bits & controls ) == controls )
    {
      bool tmp = bits[t1]; bits[t1] = bits[t2]; bits[t2] = tmp;
    }
  }

  if ( dir == direction_front )
  {
    ++pos;
    tt = sort_truth_table( tt );
  }
}

void adjust_line( circuit& circ, unsigned& pos, bitset_pair_vector_t& tt, unsigned line, direction_t dir, bool try_fredkin, bool fredkin_lookback )
{
  unsigned bw = circ.lines();

  auto input = tt[line].first;
  auto output = tt[line].second;
  auto p = ( input ^ output ) & ( dir == direction_back ? input : output );
  auto q = ( input ^ output ) & ( dir == direction_back ? output : input );
  auto mask = dir == direction_back ? output : input;

  /* fredkin */
  if ( try_fredkin )
  {
    bool found;

    do
    {
      found = false;

      auto b1 = p.find_first();
      while ( b1 != boost::dynamic_bitset<>::npos )
      {
        auto b2 = q.find_first();
        while ( b2 != boost::dynamic_bitset<>::npos )
        {
          auto mask_copy = mask;
          mask_copy.reset( b1 );
          mask_copy.reset( b2 );

          auto mask_compare = ( dir == direction_back ) ? input : output;
          bool mask_valid = mask_copy > mask_compare;

          if ( !mask_valid && fredkin_lookback ) /* try harder */
          {
            mask_valid = true;
            boost::dynamic_bitset<> current( bw );
            do {
              if ( ( mask_copy & current ) == mask_copy && ( current[b1] != current[b2] ) )
              {
                mask_valid = false;
                break;
              }
              inc( current );
            } while ( current != mask_compare );
          }

          if ( mask_valid )
          {
            //std::cout << "[i] insert fred on " << b1 << " and " << b2 << " with mask = " << mask_copy << std::endl;
            insert_fredkin_gate( circ, pos, mask_copy, b1, b2, tt, dir );
            p.reset( b1 );
            q.reset( b2 );
            mask.set( b1 );
            mask.reset( b2 );
            found = true;
            break;
          }
          b2 = q.find_next( b2 );
        }

        if ( found ) break;
        b1 = p.find_next( b1 );
      }
    } while ( found);
  }

  /* change 0 -> 1 */
  auto bpos = p.find_first();
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    insert_toffoli_gate( circ, pos, mask, bpos, tt, dir );
    mask.set( bpos );
    bpos = p.find_next( bpos );
  }

  /* change 1 -> 0 */
  bpos = q.find_first();
  while ( bpos != boost::dynamic_bitset<>::npos )
  {
    mask.reset( bpos );
    insert_toffoli_gate( circ, pos, mask, bpos, tt, dir );
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
                                     const properties::ptr& settings,
                                     const properties::ptr& statistics )
{
  /* Settings */
  const auto bidirectional    = get( settings, "bidirectional",    true  );
  const auto fredkin          = get( settings, "fredkin",          false );
  const auto fredkin_lookback = get( settings, "fredkin_lookback", false );
  const auto verbose          = get( settings, "verbose",          false );

  /* Warning */
  if ( !fredkin && fredkin_lookback && verbose )
  {
    std::cout << "[w] fredkin_lookback option has no effect since fredkin option is disabled." << std::endl;
  }

  properties_timer t( statistics );

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

  const auto bw = spec.num_outputs();
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
  const auto start_index = bidirectional ? 0u : 1u;
  auto pos = 0u;

  direction_t dir = direction_back;
  auto index = 0u;

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
      const auto other_index = boost::find_if( tt, [&tt, &i]( const value_type& p ) { return p.second == tt[i].first; } ) - tt.begin();
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
    adjust_line( circ, pos, tt, index, dir, fredkin, fredkin_lookback );
  }

  return true;
}

truth_table_synthesis_func transformation_based_synthesis_func( const properties::ptr& settings,
                                                                const properties::ptr& statistics )
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
