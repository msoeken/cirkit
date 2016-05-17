/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "nct_mapping.hpp"

#include <core/utils/timer.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/find_lines.hpp>
#include <reversible/utils/circuit_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void map_barenco72( const circuit& src, const gate& g, circuit& dest, unsigned pos )
{
  /* only for Toffolis and size-preserving circuits */
  assert( is_toffoli( g ) );
  assert( src.lines() == dest.lines() );

  /* enough empty lines? */
  const auto c = g.controls().size();
  assert( src.lines() + 1u >= ( c << 1u ) );

  /* trivial case */
  if ( c <= 2u )
  {
    dest.insert_gate( pos ) = g;
    return;
  }

  std::vector<unsigned> empty, targets;
  find_empty_lines( g, src.lines(), std::back_inserter( empty ) );
  targets = empty;

  const auto e = empty.size();
  targets.push_back( g.targets().front() );

  for ( auto offset = 0u; offset <= 1u; ++offset )
  {
    for ( auto i = offset; i < c - 2u; ++i )
    {
      insert_toffoli( dest, pos++ )( g.controls()[c - 1u - i], make_var( empty[e - 1u - i] ) )( targets[e - i] );
      insert_toffoli( dest, pos )( g.controls()[c - 1u - i], make_var( empty[e - 1u - i] ) )( targets[e - i] );
    }
    insert_toffoli( dest, pos++ )( g.controls()[0u], g.controls()[1u] )( targets[e - ( c - 2u )] );
    pos += ( c - 2u - offset );
  }
}

void map_barenco73( const circuit& src, const gate& g, circuit& dest, unsigned pos )
{
  /* only for Toffolis and size-preserving circuits */
  assert( is_toffoli( g ) );
  assert( src.lines() == dest.lines() );

  /* one empty lines */
  const auto c = g.controls().size();
  assert( src.lines() >= c + 2u );

  /* trivial case */
  if ( c <= 2u )
  {
    dest.insert_gate( pos ) = g;
    return;
  }

  std::vector<unsigned> empty;
  find_empty_lines( g, src.lines(), std::back_inserter( empty ) );

  const auto target = g.targets().front();
  const auto e = empty.front();
  const auto m = c >> 1u;

  gate::control_container c1, c2;
  for ( auto i = 0u; i < m; ++i )
  {
    c1.push_back( g.controls()[i] );
  }
  for ( auto i = m; i < c; ++i )
  {
    c2.push_back( g.controls()[i] );
  }
  c2.push_back( make_var( e ) );

  insert_toffoli( dest, pos++, c1, e );
  insert_toffoli( dest, pos++, c2, target );
  insert_toffoli( dest, pos++, c1, e );
  insert_toffoli( dest, pos++, c2, target );
}

void map_barenco72_inplace( circuit& circ, unsigned pos )
{
  const auto& g = circ[pos];

  map_barenco72( circ, g, circ, pos + 1u );
  circ.remove_gate_at( pos );
}

void map_barenco73_inplace( circuit& circ, unsigned pos )
{
  const auto& g = circ[pos];

  map_barenco73( circ, g, circ, pos + 1u );
  circ.remove_gate_at( pos );
}

void nct_mapping_inplace( circuit& circ, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */
  const auto verbose                   = get( settings, "verbose", false );
  const auto extra_ancilla_input_name  = get( settings, "extra_ancilla_input_name", std::string( "h" ) );
  const auto extra_ancilla_output_name = get( settings, "extra_ancilla_output_name", std::string( "h" ) );
  const auto constant_value            = get( settings, "constant_value", constant( false ) );

  /* timing */
  properties_timer t( statistics );

  /* ancilla needed */
  if ( circ.lines() > 3u && has_fully_controlled_gate( circ ) )
  {
    add_line_to_circuit( circ, extra_ancilla_input_name, extra_ancilla_output_name, constant_value, true );
  }

  const auto n = circ.lines();
  auto pos = 0u;

  while ( pos < circ.num_gates() )
  {
    const auto& g = circ[pos];
    const auto c = g.controls().size();

    if ( c <= 2u )
    {
      ++pos;
    }
    else if ( n + 1u >= ( c << 1u ) )
    {
      map_barenco72_inplace( circ, pos );
      pos += ( c - 2u ) << 2u;
    }
    else
    {
      map_barenco73_inplace( circ, pos );
    }
  }
}

circuit nct_mapping( const circuit& src, const properties::ptr& settings, const properties::ptr& statistics )
{
  circuit dst;

  copy_circuit( src, dst );
  nct_mapping_inplace( dst, settings, statistics );
  return dst;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
