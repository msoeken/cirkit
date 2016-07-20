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
 * @file xmg_dfs.hpp
 *
 * @brief XMG DFS functions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_DFS_HPP
#define XMG_DFS_HPP

#include <boost/graph/depth_first_search.hpp>

#include <classical/xmg/xmg.hpp>

namespace cirkit
{

class xmg_dfs_visitor : public boost::default_dfs_visitor
{
public:
  xmg_dfs_visitor( const xmg_graph& xmg );

  virtual void finish_vertex( const xmg_node& node, const xmg_graph::graph_t& g );

  virtual void finish_input( const xmg_node& node, const xmg_graph& xmg ) = 0;
  virtual void finish_constant( const xmg_node& node, const xmg_graph& xmg ) = 0;
  virtual void finish_xor_node( const xmg_node& node, const xmg_function& a, const xmg_function& b, const xmg_graph& xmg ) = 0;
  virtual void finish_maj_node( const xmg_node& node, const xmg_function& a, const xmg_function& b, const xmg_function& c, const xmg_graph& xmg ) = 0;

private:
  const xmg_graph& xmg;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
