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

#include "costs.hpp"

#include <boost/range/algorithm.hpp>

#include "../target_tags.hpp"
#include "../functions/flatten_circuit.hpp"

#include <cmath.h> 
namespace cirkit
{

  cost_t gate_costs::operator()( const circuit& circ ) const
  {
    return circ.num_gates();
  }

  cost_t line_costs::operator()( const circuit& circ ) const
  {
    return circ.lines();
  }

  cost_t transistor_costs::operator()( const gate& g, unsigned lines ) const
  {
    return 8ull * g.controls().size();
  }

  cost_t sk2013_quantum_costs::operator()( const gate& g, unsigned lines ) const
  {
    unsigned ac = g.controls().size();
    unsigned nc = boost::count_if( g.controls(), []( const variable & v ) { return ! v.polarity(); } );

    return 2ull * nc + 2ull * ac * ac - 2ull * ac + 1ull;
  }

  cost_t toffoli_gates( unsigned controls, unsigned lines )
  {
    switch ( controls )
    {
      case 0u:
      case 1u:
        return 0;
        break;
      case 2u:
        return 1;
        break;
      case 3u:
        return 4;
        break;
      case 4u:
        return (( ceil( ( lines + 1 ) / 2 ) >= controls ) ? 8 : 10 );
        break;
      default:
        return (( ceil( ( lines + 1 ) / 2 ) >= controls ) ? 4 * ( controls - 2 ) : 8 * ( controls - 3 ) );
    }
  }

  inline unsigned all_negative( const gate& g, unsigned lines )
  {
    bool ng = boost::find_if( g.controls(), []( const variable & v ) { return v.polarity(); } ) == g.controls().end();
    if ( ng )
      return ( ( ceil( ( lines + 1 ) / 2 ) >= g.controls().size() ) ? 2 : 4 );
    return 0;
  }

  cost_t ncv_quantum_costs::operator()( const gate& g, unsigned lines ) const
  {
    unsigned ac = g.controls().size();
    return ( ( ac < 2 ) ? 1ull : 5ull * toffoli_gates( ac, lines ) + all_negative( g, lines ) );
  }

  cost_t clifford_t_quantum_costs::operator()( const gate& g, unsigned lines ) const
  {
    unsigned ac = g.controls().size();
    return ( ( ac < 2 ) ? 1ull : 16ull * toffoli_gates( ac, lines ) + all_negative( g, lines ) );
  }

  cost_t t_depth_costs::operator()( const gate& g, unsigned lines ) const
  {
    unsigned ac;
    if ( is_toffoli( g ) )
    {
      ac = g.controls().size();
    }
    else if ( is_fredkin( g ) )
    {
      ac = g.controls().size() + 1u;
    }
    else
    {
      assert( false );
    }
    return 3ull * toffoli_gates( ac, lines );
  }

  cost_t t_costs::operator()( const gate& g, unsigned lines ) const
  {
    unsigned ac = g.controls().size();
    return 7ull * toffoli_gates( ac, lines );
  }

  cost_t h_costs::operator()( const gate& g, unsigned lines ) const
  {
    unsigned ac = g.controls().size();
    if ( ac == 0u ) return 2ull;
    return ( ( ac < 2 ) ? 0ull : 2ull * toffoli_gates( ac, lines ) );
  }

  struct costs_visitor : public boost::static_visitor<cost_t>
  {

    explicit costs_visitor( const circuit& circ ) : circ( circ ) {}

    cost_t operator()( const costs_by_circuit_func& f ) const
    {
      // flatten before if the circuit has modules
      if ( circ.modules().empty() )
      {
        return f( circ );
      }
      else
      {
        circuit flattened;
        flatten_circuit( circ, flattened );
        return f( flattened );
      }
    }

    cost_t operator()( const costs_by_gate_func& f ) const
    {
      cost_t sum = 0ull;
      for ( const auto& g : circ )
      {
        // respect modules
        if ( is_module( g ) )
        {
          sum += costs( *boost::any_cast<module_tag>( g.type() ).reference.get(), f );
        }
        else
        {
          sum += f( g, circ.lines() );
        }
      }
      return sum;
    }

  private:
    const circuit& circ;
  };

  cost_t costs( const circuit& circ, const cost_function& f )
  {
    return boost::apply_visitor( costs_visitor( circ ), f );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End: