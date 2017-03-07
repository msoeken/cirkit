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
 * @file circuit_hierarchy.hpp
 *
 * @brief Returns the hierarchy of a circuits by its modules
 *
 * @author Mathias Soeken
 * @since  1.1
 */

#ifndef CIRCUIT_HIERARCHY_HPP
#define CIRCUIT_HIERARCHY_HPP

#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_as_tree.hpp>
#include <boost/range/iterator_range.hpp>

using namespace boost::assign;

namespace cirkit
{
  class circuit;

  /**
   * @brief Hierarchy Graph
   *
   * Graph concept class for holding a hierarchy graph which
   * is encapsulated inside a tree with hierarchy_tree. For
   * each node it holds the name and the reference of the module
   * inside a tuple stored in the \p vertex_name property.
   *
   * For further information on how to process graphs and
   * trees, please refer to the <a href="http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/index.html" target="_blank">Boost Graph Libary</a>.
   *
   * @since  1.1
   */
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_name_t, boost::tuple<std::string, const circuit*> > > hierarchy_graph;


  /**
   * @brief This class represents a tree based on a Boost.Graph
   *
   * This class uses the boost::graph_as_tree, but offers
   * convinient methods for access the root, children, and
   * parents.
   *
   * @since  1.1
   */
  template<typename Graph>
  class constructible_tree
    : public boost::graph_as_tree<Graph,
                                  boost::iterator_property_map<typename std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>::iterator, typename boost::property_map<Graph, boost::vertex_index_t>::type>,
                                  typename boost::graph_traits<Graph>::vertex_descriptor>
  {
  public:
    /**
     * @brief Type of super class
     *
     * @since  1.1
     */
    typedef boost::graph_as_tree<Graph,
                                 boost::iterator_property_map<typename std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>::iterator, typename boost::property_map<Graph, boost::vertex_index_t>::type>,
                                 typename boost::graph_traits<Graph>::vertex_descriptor> super;

    /**
     * @brief Type of the graph
     *
     * @since  1.1
     */
    typedef Graph graph_type;

    /**
     * @brief Type of a node
     *
     * @since  1.1
     */
    typedef typename boost::graph_traits<Graph>::vertex_descriptor node_type;

    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @since  1.1
     */
    constructible_tree() : super( g, 0 )
    {
    }

    /**
     * @brief Returns the underlying graph
     *
     * @since  1.1
     */
    const Graph& graph() const { return g; }

    /**
     * @brief Returns a mutable reference to the underlying graph
     *
     * @since  1.1
     */
    Graph& graph() { return g; }

    /**
     * @brief Returns the root of the graph
     *
     * @return Node
     *
     * @since  1.1
     */
    node_type root() const { return super::_root; }

    /**
     * @brief Sets the root of the graph
     *
     * @param root Node
     *
     * @since  1.1
     */
    void set_root( node_type root ) { super::_root = root; }

    /**
     * @brief Returns the parent of a node
     *
     * @param n Node
     *
     * @return Parent of node \p n
     *
     * @since  1.1
     */
    node_type parent( node_type n ) const
    {
      typename boost::graph_traits<Graph>::in_edge_iterator it, itEnd;
      boost::tie( it, itEnd ) = boost::in_edges( n, g );

      if ( it == itEnd ) /* root */
      {
        return super::_root; // return root as parent of root
      }
      else if ( std::distance( it, itEnd ) == 1u )
      {
        return boost::source( *it, g );
      }
      else
      {
        assert( false );
      }
    }

    /**
     * @brief Returns the children of a node
     *
     * @param n Node
     * @param children Empty list containing the children of node \p n after the call
     *
     * @since  1.1
     */
    void children( node_type n, std::vector<node_type>& children ) const
    {
      for ( const auto& child : boost::make_iterator_range( boost::adjacent_vertices( n, g ) ) )
      {
        children += child;
      }
    }

  private:
    Graph g;
  };

  /**
   * @brief Hierarchy Tree
   *
   * This tree is used to store the hierarchy of a tree.
   * It is based on the hierarchy_graph
   *
   * @since  1.1
   */
  typedef constructible_tree<hierarchy_graph> hierarchy_tree;

  /**
   * @brief Calculates the hierarchy of a circuit by its modules
   *
   * This function traverses all modules starting from a top
   * circuit \p circ recursively and builds a tree and stores it in
   * \p tree.
   *
   * @param circ Circuit
   * @param tree Hierarchy Tree
   *
   * @since  1.1
   */
  void circuit_hierarchy( const circuit& circ, hierarchy_tree& tree );
}

#endif /* CIRCUIT_HIERARCHY_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
