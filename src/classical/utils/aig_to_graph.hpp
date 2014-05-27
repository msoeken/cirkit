/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file aig_to_graph.hpp
 *
 * @brief Create graph from AIG
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIG_TO_GRAPH_HPP
#define AIG_TO_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>

extern "C" {
#include <aiger.h>
}

namespace boost
{
  enum edge_polarity_t { edge_polarity };
  BOOST_INSTALL_PROPERTY(edge, polarity);
}

namespace revkit
{

  typedef boost::property<boost::edge_weight_t, unsigned, boost::property<boost::edge_polarity_t, bool> > edge_properties_t;
  typedef boost::property<boost::vertex_name_t, unsigned> vertex_properties_t;
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, vertex_properties_t, edge_properties_t> aig_graph;

  void aig_to_graph( const aiger * aig, aig_graph& graph );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
