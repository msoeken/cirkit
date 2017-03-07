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
 * @file aig_dfs.hpp
 *
 * @brief AIG DFS functions
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
