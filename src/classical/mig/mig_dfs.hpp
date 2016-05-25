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
 * @file mig_dfs.hpp
 *
 * @brief MIG DFS functions
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef MIG_DFS_HPP
#define MIG_DFS_HPP

#include <boost/graph/depth_first_search.hpp>

#include <classical/mig/mig.hpp>

namespace cirkit
{

class mig_dfs_visitor : public boost::default_dfs_visitor
{
public:
  mig_dfs_visitor( const mig_graph& mig );

  virtual void finish_vertex( const mig_node& node, const mig_graph& mig );

  virtual void finish_input( const mig_node& node, const std::string& name, const mig_graph& mig ) = 0;
  virtual void finish_constant( const mig_node& node, const mig_graph& mig ) = 0;
  virtual void finish_mig_node( const mig_node& node, const mig_function& a, const mig_function& b, const mig_function& c, const mig_graph& mig ) = 0;

protected:
  const mig_graph_info& graph_info;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
