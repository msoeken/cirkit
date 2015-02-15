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

/**
 * @file aig_dfs.hpp
 *
 * @brief Aig DFS functions
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIG_DFS_HPP
#define AIG_DFS_HPP

#include <functional>

#include <boost/graph/depth_first_search.hpp>
#include <boost/optional.hpp>

#include <classical/aig.hpp>

namespace cirkit
{

class aig_dfs_visitor : public boost::default_dfs_visitor
{
public:
  aig_dfs_visitor( const aig_graph& aig );

  virtual void finish_vertex( const aig_node& node, const aig_graph& aig );

  virtual void finish_input( const aig_node& node, const std::string& name, const aig_graph& aig ) = 0;
  virtual void finish_constant( const aig_node& node, const aig_graph& aig ) = 0;
  virtual void finish_aig_node( const aig_node& node, const aig_function& left, const aig_function& right, const aig_graph& aig ) = 0;

protected:
  const aig_graph_info& graph_info;
};

/**
 * @brief Allows iterative DFS calls
 *
 * A problem with the Boost.Graph DFS algorithm is that for each call all vertices are
 * re-initialized.  Hence, it cannot be called for multiple start vertices.  This class
 * helps to initialize all vertices once and then call DFS on selected vertices by keeping
 * old coloring information.
 */
class aig_partial_dfs
{
public:
  using color_map     = std::map<aig_node, boost::default_color_type>;
  using color_amap    = boost::associative_property_map<color_map>;
  using color_value   = boost::property_traits<color_amap>::value_type;
  using color_type    = boost::color_traits<color_value>;
  using term_func     = std::function<bool(const aig_node&, const aig_graph&)>;
  using term_func_opt = boost::optional<term_func>;

public:
  aig_partial_dfs( const aig_graph& aig, const term_func_opt& term = boost::none );

  void search( const aig_node& node );

  color_amap& color();

private:
  const aig_graph& _aig;
  color_map        _color_map;
  color_amap       _color = make_assoc_property_map( _color_map );
  term_func_opt    _term;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
