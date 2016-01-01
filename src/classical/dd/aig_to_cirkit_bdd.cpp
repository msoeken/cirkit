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

#include "aig_to_cirkit_bdd.hpp"

#include <boost/assign/std/vector.hpp>

#include <classical/utils/aig_utils.hpp>

using namespace boost::assign;

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

cirkit_bdd_simulator::cirkit_bdd_simulator( const aig_graph& aig, unsigned log_max_objs )
    : mgr( bdd_manager::create( aig_info( aig ).inputs.size(), log_max_objs ) )
{
}

cirkit_bdd_simulator::cirkit_bdd_simulator( const bdd_manager_ptr& mgr )
  : mgr( mgr )
{
}

bdd cirkit_bdd_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  return mgr->bdd_var( pos );
}

bdd cirkit_bdd_simulator::get_constant() const
{
  return mgr->bdd_bot();
}

bdd cirkit_bdd_simulator::invert( const bdd& v ) const
{
  return !v;
}

bdd cirkit_bdd_simulator::and_op( const aig_node& node, const bdd& v1, const bdd& v2 ) const
{
  return v1 && v2;
}

std::vector<bdd> aig_to_bdd( const aig_graph& aig, const bdd_manager_ptr& mgr )
{
  auto info = aig_info( aig );

  std::vector<bdd> fs;
  cirkit_bdd_simulator sim( mgr );
  auto map = simulate_aig( aig, sim );

  for ( const auto& out : info.outputs )
  {
    fs += map[out.first];
  }

  return fs;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
