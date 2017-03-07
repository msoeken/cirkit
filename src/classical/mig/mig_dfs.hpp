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
