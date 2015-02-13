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

#include "aig_support.hpp"

#include <boost/dynamic_bitset.hpp>

#include <classical/utils/simulate_aig.hpp>

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

std::map<aig_function, unsigned> aig_structural_support( const aig_graph& aig, properties::ptr settings, properties::ptr statistics )
{
  std::map<aig_function, unsigned> results;
  auto sim_results = simulate_aig( aig, aig_structural_support_simulator( boost::get_property( aig, boost::graph_name ).inputs.size() ) );
  for ( const auto& p : sim_results )
  {
    results[p.first] = p.second.count();
  }
  return results;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
