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
 * @file xmg_simulate.hpp
 *
 * @brief XMG simulation
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_SIMULATE_HPP
#define XMG_SIMULATE_HPP

#include <map>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>
#include <classical/xmg/xmg_dfs.hpp>

#include <cuddObj.hh>

namespace cirkit
{

/******************************************************************************
 * Abstract class for simulators                                              *
 ******************************************************************************/

template<typename T>
class xmg_simulator
{
public:
  /**
   * @brief Simulator routine when input is encountered
   *
   * @param node XMG node reference in the `mig' graph
   * @param name Name of that node
   * @param pos  Position of that node (usually wrt. to the input vector)
   * @param xmg  XMG
   */
  virtual T get_input( const xmg_node& node, const xmg_graph& xmg ) const = 0;
  virtual T get_constant() const = 0;
  virtual T invert( const T& v ) const = 0;
  virtual T xor_op( const xmg_node& node, const T& v1, const T& v2 ) const = 0;
  virtual T maj_op( const xmg_node& node, const T& v1, const T& v2, const T& v3 ) const = 0;

  virtual bool terminate( const xmg_node& node, const xmg_graph& xmg ) const
  {
    return false;
  }
};

/******************************************************************************
 * Several simulator implementations                                          *
 ******************************************************************************/

class xmg_pattern_simulator : public xmg_simulator<bool>
{
public:
  xmg_pattern_simulator( const boost::dynamic_bitset<>& pattern );

  bool get_input( const xmg_node& node, const xmg_graph& xmg ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool xor_op( const xmg_node& node, const bool& v1, const bool& v2 ) const;
  bool maj_op( const xmg_node& node, const bool& v1, const bool& v2, const bool& v3 ) const;

private:
  boost::dynamic_bitset<> pattern;
};

class xmg_tt_simulator : public xmg_simulator<tt>
{
public:
  tt get_input( const xmg_node& node, const xmg_graph& xmg ) const;
  tt get_constant() const;
  tt invert( const tt& v ) const;
  tt xor_op( const xmg_node& node, const tt& v1, const tt& v2 ) const;
  tt maj_op( const xmg_node& node, const tt& v1, const tt& v2, const tt& v3 ) const;
};

class xmg_bdd_simulator : public xmg_simulator<BDD>
{
public:
  xmg_bdd_simulator() : mgr( Cudd() ) {}
  xmg_bdd_simulator( const Cudd& mgr ) : mgr( mgr ) {}

  BDD get_input( const xmg_node& node, const xmg_graph& xmg ) const;
  BDD get_constant() const;
  BDD invert( const BDD& v ) const;
  BDD xor_op( const xmg_node& node, const BDD& v1, const BDD& v2 ) const;
  BDD maj_op( const xmg_node& node, const BDD& v1, const BDD& v2, const BDD& v3 ) const;

protected:
  Cudd mgr;
};

template<typename T>
class xmg_partial_node_assignment_simulator : public xmg_simulator<T>
{
public:
  xmg_partial_node_assignment_simulator( const xmg_simulator<T>& total_simulator,
                                         const std::map<xmg_node, T>& assignment,
                                         const T& default_value )
    : total_simulator( total_simulator ),
      assignment( assignment ),
      default_value( default_value ) {}

  T get_input( const xmg_node& node, const xmg_graph& xmg ) const
  {
    auto it = assignment.find( node );
    if ( it == assignment.end() )
    {
      std::cout << "[w] no assignment given for '" << node << "', assume default" << std::endl;
      return default_value;
    }
    else
    {
      return it->second;
    }
  }

  T get_constant() const { return total_simulator.get_constant(); }
  T invert( const T& v ) const { return total_simulator.invert( v ); }
  T xor_op( const xmg_node& node, const T& v1, const T& v2 ) const
  {
    auto it = assignment.find( node );
    if ( it == assignment.end() )
    {
      return total_simulator.xor_op( node, v1, v2 );
    }
    else
    {
      return it->second;
    }
  }
  T maj_op( const xmg_node& node, const T& v1, const T& v2, const T& v3 ) const
  {
    auto it = assignment.find( node );
    if ( it == assignment.end() )
    {
      return total_simulator.maj_op( node, v1, v2, v3 );
    }
    else
    {
      return it->second;
    }
  }

  bool terminate( const xmg_node& node, const xmg_graph& xmg ) const
  {
    return assignment.find( node ) != assignment.end();
  }

private:
  const xmg_simulator<T>& total_simulator;
  const std::map<xmg_node, T>& assignment;
  T default_value;
};

/******************************************************************************
 * DFS visitor for actual simulation                                          *
 ******************************************************************************/

using xmg_node_color_map = std::map<xmg_node, boost::default_color_type>;

template<typename T>
struct simulate_xmg_node_visitor : public xmg_dfs_visitor
{
public:
  simulate_xmg_node_visitor( const xmg_graph& xmg, const xmg_simulator<T>& simulator, std::map<xmg_node, T>& node_values )
    : xmg_dfs_visitor( xmg ),
      simulator( simulator ),
      node_values( node_values ) {}

  void finish_input( const xmg_node& node, const xmg_graph& xmg )
  {
    node_values[node] = simulator.get_input( node, xmg );
  }

  void finish_constant( const xmg_node& node, const xmg_graph& xmg )
  {
    node_values[node] = simulator.get_constant();
  }

  void finish_xor_node( const xmg_node& node, const xmg_function& a, const xmg_function& b, const xmg_graph& xmg )
  {
    T ta = node_values[a.node];
    T tb = node_values[b.node];

    node_values[node] = simulator.xor_op( node,
                                          a.complemented ? simulator.invert( ta ) : ta,
                                          b.complemented ? simulator.invert( tb ) : tb );
  }

  void finish_maj_node( const xmg_node& node, const xmg_function& a, const xmg_function& b, const xmg_function& c, const xmg_graph& xmg )
  {
    T ta = node_values[a.node];
    T tb = node_values[b.node];
    T tc = node_values[c.node];

    node_values[node] = simulator.maj_op( node,
                                          a.complemented ? simulator.invert( ta ) : ta,
                                          b.complemented ? simulator.invert( tb ) : tb,
                                          c.complemented ? simulator.invert( tc ) : tc );
  }

private:
  const xmg_simulator<T>& simulator;
  std::map<xmg_node, T>& node_values;
};

/******************************************************************************
 * Methods to trigger simulation                                              *
 ******************************************************************************/

template<typename T>
T simulate_xmg_node( const xmg_graph& xmg, const xmg_node& node,
                     const xmg_simulator<T>& simulator,
                     xmg_node_color_map& colors,
                     std::map<xmg_node, T>& node_values )
{
  boost::depth_first_visit( xmg.graph(), node,
                            simulate_xmg_node_visitor<T>( xmg, simulator, node_values ),
                            boost::make_assoc_property_map( colors ),
                            [&simulator, &xmg]( const xmg_node& node, const xmg_graph::graph_t& g ) { return simulator.terminate( node, xmg ); } );

  return node_values[node];
}

template<typename T>
T simulate_xmg_node( const xmg_graph& xmg, const xmg_node& node,
                     const xmg_simulator<T>& simulator )
{
  xmg_node_color_map colors;
  std::map<xmg_node, T> node_values;

  return simulate_xmg_node<T>( xmg, node, simulator, colors, node_values );
}

template<typename T>
T simulate_xmg_function( const xmg_graph& xmg, const xmg_function& f,
                         const xmg_simulator<T>& simulator,
                         xmg_node_color_map& colors,
                         std::map<xmg_node, T>& node_values )
{
  T value = simulate_xmg_node<T>( xmg, f.node, simulator, colors, node_values );
  return f.complemented ? simulator.invert( value ) : value;
}

template<typename T>
T simulate_xmg_function( const xmg_graph& xmg, const xmg_function& f,
                         const xmg_simulator<T>& simulator )
{
  T value = simulate_xmg_node<T>( xmg, f.node, simulator );
  return f.complemented ? simulator.invert( value ) : value;
}

template<typename T>
std::map<xmg_function, T> simulate_xmg( const xmg_graph& xmg, const xmg_simulator<T>& simulator,
                                        const properties::ptr& settings = properties::ptr(),
                                        const properties::ptr& statistics = properties::ptr() )
{
  /* settings */
  const auto verbose = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  xmg_node_color_map colors;
  std::map<xmg_node, T> node_values;

  std::map<xmg_function, T> results;

  for ( const auto& o : xmg.outputs() )
  {
    if ( verbose )
    {
      std::cout << "[i] simulate '" << o.second << "'" << std::endl;
    }
    T value = simulate_xmg_node<T>( xmg, o.first.node, simulator, colors, node_values );

    /* value may need to be inverted */
    results[o.first] = o.first.complemented ? simulator.invert( value ) : value;
  }

  set( statistics, "node_values", node_values );

  return results;
}

/******************************************************************************
 * Utility functions for simulation                                           *
 ******************************************************************************/

boost::dynamic_bitset<> xmg_simulate_pattern( const xmg_graph& xmg, const boost::dynamic_bitset<>& pattern );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
