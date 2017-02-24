/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
 * @file lut_graph.hpp
 *
 * @brief LUT Graph
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef LUT_GRAPH_HPP
#define LUT_GRAPH_HPP

#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

#include <core/utils/graph_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace boost
{

enum vertex_lut_type_t { vertex_lut_type };
BOOST_INSTALL_PROPERTY(vertex, lut_type);

enum vertex_lut_t { vertex_lut };
BOOST_INSTALL_PROPERTY(vertex, lut);

}

namespace cirkit
{

using boost::adjacency_list;
using boost::default_color_type;
using boost::directedS;
using boost::edge_name_t;
using boost::graph_traits;
using boost::property;
using boost::vecS;
using boost::vertex_color_t;
using boost::vertex_name_t;
using boost::vertex_lut_t;
using boost::vertex_lut_type_t;

enum class lut_type_t { pi, po, gnd, vdd, internal };

/******************************************************************************
 * LUT graph                                                                  *
 ******************************************************************************/

using lut_graph_vertex_properties_t = property<vertex_name_t, std::string,
                                      property<vertex_lut_t, std::string,
                                      property<vertex_lut_type_t, lut_type_t>>>;
using lut_graph_t  = digraph_t<lut_graph_vertex_properties_t>;
using lut_vertex_t = vertex_t<lut_graph_t>;
using lut_edge_t   = edge_t<lut_graph_t>;

unsigned lut_graph_lut_count( const lut_graph_t& g );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
