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

#include <core/utils/bdd_utils.hpp>

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
  void dd_from_bdd( dd& graph, bdd_function_t& bdds, const dd_from_bdd_settings& settings = dd_from_bdd_settings() );

  void dd_synthesis( circuit& circ, const dd& graph );

}

}

#endif /* DD_SYNTHESIS_P_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
