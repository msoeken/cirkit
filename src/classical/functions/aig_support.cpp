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

#include "aig_support.hpp"

#include <boost/dynamic_bitset.hpp>

#include <classical/functions/simulate_aig.hpp>

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_structural_support_simulator : public aig_simulator<boost::dynamic_bitset<>>
{
public:
  aig_structural_support_simulator( unsigned num_inputs ) : num_inputs( num_inputs ) {}

  boost::dynamic_bitset<> get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    boost::dynamic_bitset<> b( num_inputs );
    b.set( pos );
    return b;
  }

  boost::dynamic_bitset<> get_constant() const
  {
    return boost::dynamic_bitset<>( num_inputs );
  }

  boost::dynamic_bitset<> invert( const boost::dynamic_bitset<>& v ) const
  {
    return v;
  }

  boost::dynamic_bitset<> and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const
  {
    return v1 | v2;
  }

private:
  unsigned num_inputs;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

support_map_t aig_structural_support( const aig_graph& aig, properties::ptr settings, properties::ptr statistics )
{
  return simulate_aig( aig, aig_structural_support_simulator( boost::get_property( aig, boost::graph_name ).inputs.size() ), settings, statistics );
}


boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, unsigned num_pis )
{
  boost::dynamic_bitset<> support( num_pis );

  auto pos = ( po * num_pis ) << 1u;

  for ( auto i = 0u; i < num_pis; ++i )
  {
    if ( !u[pos] || !u[pos + 1] )
    {
      support.set( i );
    }
    pos += 2u;
  }

  return support;
}

boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, const aig_graph_info& info )
{
  return get_functional_support( u, po, info.inputs.size() );
}

boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, const aig_graph& aig )
{
  return get_functional_support( u, po, aig_info( aig ).inputs.size() );
}

support_map_t aig_functional_support( const aig_graph& aig )
{
  const auto& info = aig_info( aig );
  assert( !info.unateness.empty() );

  support_map_t result;
  for ( auto j = 0u; j < info.outputs.size(); ++j )
  {
    result[info.outputs[j].first] = get_functional_support( info.unateness, j, info.inputs.size() );
  }

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
