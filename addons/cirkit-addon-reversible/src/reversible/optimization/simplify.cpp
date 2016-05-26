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

#include "simplify.hpp"

#include <cmath>

#include <boost/dynamic_bitset.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/reverse_circuit.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>> control_mask( const gate::control_container& controls, unsigned n )
{
  boost::dynamic_bitset<> cs( n ), ps( n );

  for ( const auto& c : controls )
  {
    assert( c.line() < n );

    cs[c.line()] = 1;
    ps[c.line()] = c.polarity();
  }

  return {cs, ps};
}

gate::control_container make_controls( const boost::dynamic_bitset<>& cs, const boost::dynamic_bitset<>& ps )
{
  gate::control_container controls;

  foreach_bit( cs, [&controls, &ps]( unsigned pos ) {
      controls.push_back( make_var( pos, ps[pos] ) );
    } );

  return controls;
}

circuit simplify_not_gates( const circuit& base )
{
  circuit circ;
  circ.set_lines( base.lines() );
  copy_metadata( base, circ );

  boost::dynamic_bitset<> not_state( circ.lines() );

  for ( const auto& g : base )
  {
    assert( is_toffoli( g ) );
    const auto target = g.targets().front();

    if ( g.controls().empty() )
    {
      /* do not copy but remember */
      not_state.flip( target );
    }
    else if ( g.controls().size() == 1 && not_state[target] )
    {
      /* merge stored not gate into CNOT with the same target */
      append_cnot( circ, make_var( g.controls().front().line(), !g.controls().front().polarity() ), target );
      not_state.flip( target );
    }
    else
    {
      /* flip controls based on remembered NOT gates */
      gate::control_container controls;

      for ( const auto& c : g.controls() )
      {
        controls.push_back( make_var( c.line(), c.polarity() != not_state[c.line()] ) );
      }

      append_toffoli( circ, controls, target );
    }
  }

  /* remaining NOT gates */
  foreach_bit( not_state, [&circ]( unsigned pos ) {
      append_not( circ, pos );
    } );

  return circ;
}

circuit simplify_adjacent( const circuit& base )
{
  circuit circ;
  circ.set_lines( base.lines() );
  copy_metadata( base, circ );

  auto pos = 0u;

  while ( pos < base.num_gates() )
  {
    if ( pos + 1 == base.num_gates() )
    {
      /* last gate */
      circ.append_gate() = base[pos];
      break;
    }

    const auto& g1 = base[pos];
    const auto& g2 = base[pos + 1];

    /* candidate for simplication */
    if ( is_toffoli( g1 ) && is_toffoli( g2 ) && ( g1.targets().front() == g2.targets().front() ) && abs( static_cast<int>( g1.controls().size() ) - static_cast<int>( g2.controls().size() ) ) <= 1 )
    {
      boost::dynamic_bitset<> cs1, ps1, cs2, ps2;

      std::tie( cs1, ps1 ) = control_mask( g1.controls(), base.lines() );
      std::tie( cs2, ps2 ) = control_mask( g2.controls(), base.lines() );

      const auto cdc = ( cs1 ^ cs2 ).count();
      const auto pdc = ( ps1 ^ ps2 ).count();

      if ( cs1 == cs2 && ps1 == ps2 )
      {
        /* same gate, remove */
        pos += 2;
      }
      else if ( cs1 == cs2 && pdc == 1 )
      {
        /* remove one control */
        const auto bit = ( ps1 ^ ps2 ).find_first();
        assert( cs1[bit] );
        cs1[bit] = 0;

        append_toffoli( circ, make_controls( cs1, ps1 ), g1.targets().front() );

        pos += 2;
      }
      else if ( cdc == 1 && pdc <= 1 && ( ( pdc == 0 ) || ( cs1 ^ cs2 ).find_first() == ( ps1 ^ ps2 ).find_first() ) )
      {
        const auto bit = ( cs1 ^ cs2 ).find_first();
        cs1[bit] = true;
        ps1[bit] = pdc == 1 ? false : true;

        append_toffoli( circ, make_controls( cs1, ps1 ), g1.targets().front() );

        pos += 2;
      }
      else
      {
        circ.append_gate() = base[pos++];
      }
    }
    else
    {
      circ.append_gate() = base[pos++];
    }
  }

  return circ;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool simplify( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
{
  /* timer */
  properties_timer t( statistics );

  circuit tmp;

  tmp = simplify_not_gates( base );
  tmp = simplify_adjacent( tmp );

  reverse_circuit( tmp );
  tmp = simplify_not_gates( tmp );
  tmp = simplify_adjacent( tmp );
  reverse_circuit( tmp );

  copy_circuit( tmp, circ );
  copy_metadata( base, circ );

  return true;
}

optimization_func simplify( properties::ptr settings, properties::ptr statistics )
{
  optimization_func f = [&settings, &statistics]( circuit& circ, const circuit& base ) {
    return simplify( circ, base, settings, statistics );
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
