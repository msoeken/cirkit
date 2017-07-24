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

#include "costs.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/integer/integer_log2.hpp>

#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/flatten_circuit.hpp>
#include <reversible/synthesis/optimal_quantum_circuits.hpp>

#include <cmath>
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

cost_t depth_costs::operator()( const circuit& circ ) const
{
  auto mask = ~boost::dynamic_bitset<>( circ.lines() );
  auto depth = 0;

  for ( const auto& g : circ )
  {
    boost::dynamic_bitset<> gate_mask( circ.lines() );
    for ( const auto& c : g.controls() ) { gate_mask.set( c.line() ); }
    for ( auto t : g.targets() ) { gate_mask.set( t ); }

    if ( gate_mask.intersects( mask ) )
    {
      ++depth;
      mask = gate_mask;
    }
    else
    {
      mask |= gate_mask;
    }
  }

  return depth;
}

cost_t sk2013_quantum_costs::operator()( const gate& g, unsigned lines ) const
{
  unsigned ac = g.controls().size();
  unsigned nc = boost::count_if( g.controls(), []( const variable& v ) { return !v.polarity(); } );

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
      return (( ceil((float) lines / 2 ) >= controls ) ? 8 : 10 );
      break;
    default:
      return (( ceil((float) lines / 2 ) >= controls ) ? 4 * ( controls - 2 ) : 8 * ( controls - 3 ) );
  }
}

inline unsigned all_negative( const gate& g, unsigned lines )
{
  bool ng = boost::find_if( g.controls(), []( const variable& v ) { return v.polarity(); } ) == g.controls().end();
  if ( ng )
    return (( ceil((float) lines / 2 ) >= g.controls().size() ) ? 2 : 4 );
  return 0;
}

cost_t ncv_quantum_costs::operator()( const gate& g, unsigned lines ) const
{
  unsigned ac = g.controls().size();
  return ((ac < 2) ? 1ull : 5ull * toffoli_gates( ac, lines ) + all_negative( g, lines ) );
}

cost_t clifford_t_quantum_costs::operator()( const gate& g, unsigned lines ) const
{
  unsigned ac = g.controls().size();
  return ((ac < 2) ? 1ull : 16ull * toffoli_gates( ac, lines ) + all_negative( g, lines ) );
}

cost_t t_depth_costs::operator()( const gate& g, unsigned lines ) const
{
  return cost_invalid();
}

cost_t t_costs::operator()( const gate& g, unsigned lines ) const
{
  /* the following computation is based on
   * [D. Maslov, Phys Rev A 93, 022311, 2016.
   */
  if ( is_toffoli( g ) || is_fredkin( g ) )
  {
    const auto ac = g.controls().size();

    switch ( ac )
    {
    case 0u:
    case 1u:
      return 0;

    case 2u:
      return 7;

    default:
      if ( lines - ac - 1 >= ( ac - 1 ) / 2 )
      {
        return 8 * ( ac - 1 );
      }
      else
      {
        return 16 * ( ac - 1 );
      }
    }
  }
  else if ( is_hadamard( g ) )
  {
    return 0ull;
  }
  else if ( is_pauli( g ) )
  {
    const auto& tag = boost::any_cast<pauli_tag>( g.type() );
    return ( tag.axis == pauli_axis::Z && tag.root == 4u ) ? 1ull : 0ull;
  }
  else if ( is_stg( g ) )
  {
    const auto& tag = boost::any_cast<stg_tag>( g.type() );
    const auto spec = tag.affine_class.size() ? tag.affine_class : tag.function;
    const auto num_vars = boost::integer_log2( spec.size() );
    if ( num_vars >= 2 && num_vars <= 5 )
    {
      const auto& idx_map = optimal_quantum_circuits::spectral_classification_index[num_vars - 2u];
      const auto it = idx_map.find( spec.to_ulong() );
      if ( it == idx_map.end() )
      {
        const auto& idx_map = optimal_quantum_circuits::affine_classification_index[num_vars - 2u];
        const auto it = idx_map.find( spec.to_ulong() );
        if ( it == idx_map.end() )
        {
          return cost_invalid();
        }
        else
        {
          return optimal_quantum_circuits::affine_classification_tcount[num_vars - 2u][it->second];
        }
      }
      else
      {
        return optimal_quantum_circuits::spectral_classification_tcount[num_vars - 2u][it->second];
      }
    }
    else
    {
      return cost_invalid();
    }
  }
  else
  {
    return cost_invalid();
  }
}

cost_t h_costs::operator()( const gate& g, unsigned lines ) const
{
  unsigned ac = g.controls().size();
  if ( ac == 0u ) return 2ull;
  return ((ac < 2) ? 0ull : 2ull * toffoli_gates( ac, lines ) );
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
    cost_t sum = 0ull, tmp{};
    for ( const auto& g : circ )
    {
      // respect modules
      if ( is_module( g ) )
      {
        tmp = costs( *boost::any_cast<module_tag>( g.type() ).reference.get(), f );
      }
      else
      {
        tmp = ( circ.lines() == g.controls().size() + 1 ) ? f( g, circ.lines() + 1 ) : f( g, circ.lines() ) ;
      }

      if ( tmp == cost_invalid() )
      {
        return tmp;
      }
      else
      {
        sum += tmp;
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

cost_t costs( const circuit& circ, unsigned begin, unsigned end, const costs_by_gate_func& f )
{
  cost_t sum = 0ull, tmp{};
  while ( begin < end )
  {
    const auto& g = circ[begin++];

    // respect modules
    if ( is_module( g ) )
    {
      tmp = costs( *boost::any_cast<module_tag>( g.type() ).reference.get(), f );
    }
    else
    {
      tmp = ( circ.lines() == g.controls().size() + 1 ) ? f( g, circ.lines() + 1 ) : f( g, circ.lines() ) ;
    }

    if ( tmp == cost_invalid() )
    {
      return tmp;
    }
    else
    {
      sum += tmp;
    }
  }
  return sum;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
