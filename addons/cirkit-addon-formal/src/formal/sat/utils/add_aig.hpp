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

/**
 * @file add_aig.hpp
 *
 * @brief Adds clauses based from AIG
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef ADD_AIG_HPP
#define ADD_AIG_HPP

#include <boost/graph/depth_first_search.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/range_utils.hpp>

#include <classical/aig.hpp>
#include <classical/utils/aig_dfs.hpp>
#include <classical/utils/aig_utils.hpp>

#include <formal/sat/sat_solver.hpp>
#include <formal/sat/operations/logic.hpp>

namespace cirkit
{

template<class S>
class add_aig_visitor : public aig_dfs_visitor
{
public:
  add_aig_visitor( const aig_graph& aig, S& solver, int& sid, const std::vector<int>& piids, std::map<aig_node, int>& node_var_map, bool blocking_vars, std::map<aig_node, int>& blocking_var_map )
    : aig_dfs_visitor( aig ),
      solver( solver ),
      cur_var( sid ),
      piids( piids ),
      node_var_map( node_var_map ),
      blocking_vars( blocking_vars ),
      blocking_var_map( blocking_var_map )
  {
  }

  void finish_input( const aig_node& node, const std::string& name, const aig_graph& aig )
  {
    unsigned pos = std::distance( graph_info.inputs.begin(), boost::find( graph_info.inputs, node ) );
    node_var_map[node] = piids[pos];
  }

  void finish_constant( const aig_node& node, const aig_graph& aig )
  {
    node_var_map[node] = cur_var;
    add_clause( solver )( {-cur_var} );
    cur_var++;
  }

  void finish_aig_node( const aig_node& node, const aig_function& left, const aig_function& right, const aig_graph& aig )
  {
    node_var_map[node] = cur_var;
    int a = node_var_map[left.node];
    int b = node_var_map[right.node];
    assert( node != left.node );
    assert( node != right.node );
    assert( cur_var != a );
    assert( cur_var != b );

    if ( blocking_vars )
    {
      if ( blocking_var_map.find( node ) == blocking_var_map.end() )
      {
        blocking_var_map[node] = cur_var + 1u;
      }
      blocking_and( solver, blocking_var_map[node], left.complemented ? -a : a, right.complemented ? -b : b, cur_var );
      cur_var += 2u;
    }
    else
    {
      logic_and( solver, left.complemented ? -a : a, right.complemented ? -b : b, cur_var );
      cur_var++;
    }
  }

public:
  S& solver;
  int& cur_var;
  const std::vector<int>& piids;
  std::map<aig_node, int>& node_var_map;
  bool blocking_vars;
  std::map<aig_node, int>& blocking_var_map;
};

template<class S>
int add_aig( S& solver, const aig_graph& aig, int sid, std::vector<int>& piids, std::vector<int>& poids,
             properties::ptr settings = properties::ptr(),
             properties::ptr statistics = properties::ptr() )
{
  /* Settings */
  auto blocking_vars = get( settings, "blocking_vars", false );
  auto blocking_var_map = get( statistics, "blocking_var_map", std::map<aig_node, int>() );

  const auto& graph_info = aig_info( aig );
  std::map<aig_node, int> node_var_map;
  std::map<aig_node, boost::default_color_type> colors;

  piids.resize( graph_info.inputs.size() );
  poids.resize( graph_info.outputs.size() );

  boost::iota( piids, sid );
  sid += piids.size();

  for ( const auto& output : index( graph_info.outputs ) )
  {
    auto node = output.value.first.node;
    auto complement = output.value.first.complemented;

    add_aig_visitor<S> visitor( aig, solver, sid, piids, node_var_map, blocking_vars, blocking_var_map );
    boost::depth_first_visit( aig, node, visitor, boost::make_assoc_property_map( colors ) );

    if ( complement )
    {
      int new_output = poids[output.index] = sid++;
      not_equals( solver, node_var_map[node], new_output );
    }
    else
    {
      poids[output.index] = node_var_map[node];
    }
  }

  if ( statistics )
  {
    statistics->set( "node_var_map", node_var_map );

    if ( blocking_vars )
    {
      statistics->set( "blocking_var_map", blocking_var_map );
    }
  }

  return sid;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
