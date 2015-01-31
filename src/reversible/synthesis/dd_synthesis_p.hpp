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
 * @file dd_synthesis_p.hpp
 *
 * @brief DD Based Synthesis
 *
 * @author Mathias Soeken
 * @author Robert Wille
 * @since  1.0
 */

#ifndef DD_SYNTHESIS_P_HPP
#define DD_SYNTHESIS_P_HPP

#include <boost/graph/adjacency_list.hpp>

#include <cudd.h>

namespace cirkit
{

  class circuit;

namespace internal
{

  enum decomposition_type
  {
    shannon, positive_davio, negative_davio
  };

  struct node_properties
  {
    node_properties()
      : var( 0 ),
        dtl( shannon ) {}

    unsigned       var;
    unsigned short dtl;
  };

  struct edge_properties
  {
    edge_properties()
      : complemented( false ) {}

    bool complemented;
  };

  struct graph_properties
  {
    graph_properties()
      : ninputs( 0 ) {}

    std::vector<std::size_t> roots;
    std::vector<std::string> labels;
    unsigned ninputs;
  };

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                boost::property<boost::vertex_name_t, node_properties>,
                                boost::property<boost::edge_name_t, edge_properties>,
                                boost::property<boost::graph_name_t, graph_properties> > dd;

  void dd_to_dot( const dd& graph, const std::string& filename );

  struct dd_from_kfdd_settings
  {
    dd_from_kfdd_settings();

    unsigned default_decomposition;
    unsigned reordering;
    double   sift_factor;
    char     sifting_growth_limit;
    char     sifting_method;
    unsigned*      node_count;
  };

  void dd_from_kfdd( dd& graph, const std::string& filename, const dd_from_kfdd_settings& settings = dd_from_kfdd_settings() );

  struct dd_from_bdd_settings
  {
    dd_from_bdd_settings();

    bool complemented_edges;
    unsigned reordering;
    unsigned* node_count;
    std::string infofilename;
  };

  void dd_from_bdd( dd& graph, const std::vector<DdNode*>& nodes );
  void dd_from_bdd( dd& graph, const std::string& filename, const dd_from_bdd_settings& settings = dd_from_bdd_settings() );

  void dd_synthesis( circuit& circ, const dd& graph );

}

}

#endif /* DD_SYNTHESIS_P_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
