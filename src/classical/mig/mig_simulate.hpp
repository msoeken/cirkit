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
 * @file simulate_mig.hpp
 *
 * @brief MIG simulation
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef SIMULATE_MIG_HPP
#define SIMULATE_MIG_HPP

#include <unordered_map>

#include <core/utils/timer.hpp>
#include <classical/mig/mig.hpp>
#include <classical/mig/mig_dfs.hpp>
#include <classical/mig/mig_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

#include <cuddObj.hh>

namespace cirkit
{

/******************************************************************************
 * Abstract class for simulators                                              *
 ******************************************************************************/

template<typename T>
class mig_simulator
{
public:
  /**
   * @brief Simulator routine when input is encountered
   *
   * @param node MIG node reference in the `mig' graph
   * @param name Name of that node
   * @param pos  Position of that node (usually wrt. to `mig' inputs vector)
   * @param mig  MIG graph
   */
  virtual T get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const = 0;
  virtual T get_constant() const = 0;
  virtual T invert( const T& v ) const = 0;
  virtual T maj_op( const mig_node& node, const T& v1, const T& v2, const T& v3 ) const = 0;

  virtual bool terminate( const mig_node& node, const mig_graph& mig ) const
  {
    return false;
  }
};

/******************************************************************************
 * Several simulator implementations                                          *
 ******************************************************************************/

class mig_simple_assignment_simulator : public mig_simulator<bool>
{
public:
  using mig_name_value_map = std::unordered_map<std::string, bool>;

  mig_simple_assignment_simulator( const mig_name_value_map& assignment );

  bool get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool maj_op( const mig_node& node, const bool& v1, const bool& v2, const bool& v3 ) const;

private:
  const mig_name_value_map& assignment;
};

class mig_tt_simulator : public mig_simulator<tt>
{
public:
  tt get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const;
  tt get_constant() const;
  tt invert( const tt& v ) const;
  tt maj_op( const mig_node& node, const tt& v1, const tt& v2, const tt& v3 ) const;
};

class mig_bdd_simulator : public mig_simulator<BDD>
{
public:
  mig_bdd_simulator() : mgr( Cudd() ) {}
  mig_bdd_simulator( const Cudd& mgr ) : mgr( mgr ) {}

  BDD get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const;
  BDD get_constant() const;
  BDD invert( const BDD& v ) const;
  BDD maj_op( const mig_node& node, const BDD& v1, const BDD& v2, const BDD& v3 ) const;

protected:
  Cudd mgr;
};

class mig_depth_simulator : public mig_simulator<unsigned>
{
public:
  unsigned get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const;
  unsigned get_constant() const;
  unsigned invert( const unsigned& v ) const;
  unsigned maj_op( const mig_node& node, const unsigned& v1, const unsigned& v2, const unsigned& v3 ) const;
};

template<typename T>
class mig_partial_node_assignment_simulator : public mig_simulator<T>
{
public:
  mig_partial_node_assignment_simulator( const mig_simulator<T>& total_simulator,
                                         const std::map<mig_node, T>& assignment,
                                         const T& default_value )
    : total_simulator( total_simulator ),
      assignment( assignment ),
      default_value( default_value ) {}

  T get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
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
  T maj_op( const mig_node& node, const T& v1, const T& v2, const T& v3 ) const
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

  bool terminate( const mig_node& node, const mig_graph& mig ) const
  {
    return assignment.find( node ) != assignment.end();
  }

private:
  const mig_simulator<T>& total_simulator;
  const std::map<mig_node, T>& assignment;
  T default_value;
};

template<typename T>
class mig_lambda_simulator : public mig_simulator<T>
{
public:
  mig_lambda_simulator( const std::function<T(const mig_node&, const std::string&, unsigned, const mig_graph&)>& get_input_func,
                        const std::function<T()>& get_constant_func,
                        const std::function<T(const T&)>& invert_func,
                        const std::function<T(const mig_node&, const T&, const T&, const T&)>& maj_op_func )
    : get_input_func( get_input_func ),
      get_constant_func( get_constant_func ),
      invert_func( invert_func ),
      maj_op_func( maj_op_func )
  {
  }

  T get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
  {
    return get_input_func( node, name, pos, mig );
  }

  T get_constant() const
  {
    return get_constant_func();
  }

  T invert( const T& v ) const
  {
    return invert_func( v );
  }

  T maj_op( const mig_node& node, const T& v1, const T& v2, const T& v3 ) const
  {
    return maj_op_func( node, v1, v2, v3 );
  }

private:
  std::function<T(const mig_node&, const std::string&, unsigned, const mig_graph&)> get_input_func;
  std::function<T()> get_constant_func;
  std::function<T(const T&)> invert_func;
  std::function<T(const mig_node&, const T&, const T&, const T&)> maj_op_func;
};

/******************************************************************************
 * DFS visitor for actual simulation                                          *
 ******************************************************************************/

using mig_node_color_map = std::map<mig_node, boost::default_color_type>;

template<typename T>
struct simulate_mig_node_visitor : public mig_dfs_visitor
{
public:
  simulate_mig_node_visitor( const mig_graph& mig, const mig_simulator<T>& simulator, std::map<mig_node, T>& node_values )
    : mig_dfs_visitor( mig ),
      simulator( simulator ),
      node_values( node_values ) {}

  void finish_input( const mig_node& node, const std::string& name, const mig_graph& mig )
  {
    unsigned pos = std::distance( graph_info.inputs.begin(), boost::find( graph_info.inputs, node ) );
    node_values[node] = simulator.get_input( node, name, pos, mig );
  }

  void finish_constant( const mig_node& node, const mig_graph& mig )
  {
    node_values[node] = simulator.get_constant();
  }

  void finish_mig_node( const mig_node& node, const mig_function& a, const mig_function& b, const mig_function& c, const mig_graph& mig )
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
  const mig_simulator<T>& simulator;
  std::map<mig_node, T>& node_values;
};

/******************************************************************************
 * Methods to trigger simulation                                              *
 ******************************************************************************/

template<typename T>
T simulate_mig_node( const mig_graph& mig, const mig_node& node,
                     const mig_simulator<T>& simulator,
                     mig_node_color_map& colors,
                     std::map<mig_node, T>& node_values )
{
  boost::depth_first_visit( mig, node,
                            simulate_mig_node_visitor<T>( mig, simulator, node_values ),
                            boost::make_assoc_property_map( colors ),
                            [&simulator]( const mig_node& node, const mig_graph& mig ) { return simulator.terminate( node, mig ); } );

  return node_values[node];
}

template<typename T>
T simulate_mig_node( const mig_graph& mig, const mig_node& node,
                     const mig_simulator<T>& simulator )
{
  mig_node_color_map colors;
  std::map<mig_node, T> node_values;

  return simulate_mig_node<T>( mig, node, simulator, colors, node_values );
}

template<typename T>
T simulate_mig_function( const mig_graph& mig, const mig_function& f,
                         const mig_simulator<T>& simulator,
                         mig_node_color_map& colors,
                         std::map<mig_node, T>& node_values )
{
  T value = simulate_mig_node<T>( mig, f.node, simulator, colors, node_values );
  return f.complemented ? simulator.invert( value ) : value;
}

template<typename T>
T simulate_mig_function( const mig_graph& mig, const mig_function& f,
                         const mig_simulator<T>& simulator )
{
  T value = simulate_mig_node<T>( mig, f.node, simulator );
  return f.complemented ? simulator.invert( value ) : value;
}

template<typename T>
std::map<mig_function, T> simulate_mig( const mig_graph& mig, const mig_simulator<T>& simulator,
                                        const properties::ptr& settings = properties::ptr(),
                                        const properties::ptr& statistics = properties::ptr() )
{
  /* settings */
  auto verbose = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  mig_node_color_map colors;
  std::map<mig_node, T> node_values;

  std::map<mig_function, T> results;

  for ( const auto& o : mig_info( mig ).outputs )
  {
    if ( verbose )
    {
      std::cout << "[i] simulate '" << o.second << "'" << std::endl;
    }
    T value = simulate_mig_node<T>( mig, o.first.node, simulator, colors, node_values );

    /* value may need to be inverted */
    results[o.first] = o.first.complemented ? simulator.invert( value ) : value;
  }

  set( statistics, "node_values", node_values );

  return results;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
