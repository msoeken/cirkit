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
 * @file simulate_aig.hpp
 *
 * @brief AIG simuation
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef SIMULATE_AIG_HPP
#define SIMULATE_AIG_HPP

#include <map>

#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/properties.hpp>
#include <core/utils/timer.hpp>
#include <classical/aig.hpp>
#include <classical/utils/aig_dfs_visitor.hpp>
#include <classical/utils/truth_table_utils.hpp>

#include <cuddObj.hh>

namespace cirkit
{

/******************************************************************************
 * Abstract class for simulators                                              *
 ******************************************************************************/

template<typename T>
class aig_simulator
{
public:
  /**
   * @brief Simulator routine when input is encountered
   *
   * @param node AIG node reference in the `aig' graph
   * @param name Name of that node
   * @param pos  Position of that node (usually wrt. to `aig' inputs vector)
   * @param aig  AIG graph
   */
  virtual T get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const = 0;
  virtual T get_constant() const = 0;
  virtual T invert( const T& v ) const = 0;
  virtual T and_op( const aig_node& node, const T& v1, const T& v2 ) const = 0;

  virtual bool terminate( const aig_node& node, const aig_graph& aig ) const
  {
    return false;
  }
};

/******************************************************************************
 * Several simulator implementations                                          *
 ******************************************************************************/

class simple_assignment_simulator : public aig_simulator<bool>
{
public:
  typedef std::map<std::string, bool> aig_name_value_map;

  simple_assignment_simulator( const aig_name_value_map& assignment );

  bool get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool and_op( const aig_node& node, const bool& v1, const bool& v2 ) const;

private:
  const aig_name_value_map& assignment;
};

class simple_node_assignment_simulator : public aig_simulator<bool>
{
public:
  typedef std::map<aig_node, bool> aig_node_value_map;

  simple_node_assignment_simulator( const aig_node_value_map& assignment );

  bool get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool and_op( const aig_node& node, const bool& v1, const bool& v2 ) const;
  bool terminate( const aig_node& node, const aig_graph& aig ) const;

private:
  const aig_node_value_map& assignment;
};

class word_assignment_simulator : public aig_simulator<boost::dynamic_bitset<>>
{
public:
  typedef std::map<std::string, boost::dynamic_bitset<>> aig_name_value_map;

  word_assignment_simulator( const aig_name_value_map& assignment );

  boost::dynamic_bitset<> get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  boost::dynamic_bitset<> get_constant() const;
  boost::dynamic_bitset<> invert( const boost::dynamic_bitset<>& v ) const;
  boost::dynamic_bitset<> and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const;

private:
  const aig_name_value_map& assignment;
};

class word_node_assignment_simulator : public aig_simulator<boost::dynamic_bitset<>>
{
public:
  typedef std::map<aig_node, boost::dynamic_bitset<>> aig_node_value_map;

  word_node_assignment_simulator( const aig_node_value_map& assignment );

  boost::dynamic_bitset<> get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  boost::dynamic_bitset<> get_constant() const;
  boost::dynamic_bitset<> invert( const boost::dynamic_bitset<>& v ) const;
  boost::dynamic_bitset<> and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const;
  bool terminate( const aig_node& node, const aig_graph& aig ) const;

private:
  const aig_node_value_map& assignment;
};

class tt_simulator : public aig_simulator<tt>
{
public:
  tt get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  tt get_constant() const;
  tt invert( const tt& v ) const;
  tt and_op( const aig_node& node, const tt& v1, const tt& v2 ) const;
};

class bdd_simulator : public aig_simulator<BDD>
{
public:
  bdd_simulator() : mgr( Cudd() ) {}
  bdd_simulator( const Cudd& mgr ) : mgr( mgr ) {}

  BDD get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  BDD get_constant() const;
  BDD invert( const BDD& v ) const;
  BDD and_op( const aig_node& node, const BDD& v1, const BDD& v2 ) const;

protected:
  Cudd mgr;
};

template<typename T>
class partial_simulator : public aig_simulator<T>
{
public:
  partial_simulator( const aig_simulator<T>& total_simulator,
                     const std::map<aig_node, T>& assignment,
                     const aig_graph& aig )
    : total_simulator( total_simulator ),
      assignment( assignment )
  {
    using boost::adaptors::filtered;

    const auto& graph_info = boost::get_property( aig, boost::graph_name );
    boost::push_back( real_inputs, graph_info.inputs | filtered( [this]( const aig_node& n ) { return this->assignment.find( n ) == this->assignment.end(); } ) );
  }

  T get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    /* Is in assignment? */
    auto it = assignment.find( node );
    if ( it != assignment.end() )
    {
      return it->second;
    }
    else
    {
      unsigned real_pos = std::distance( real_inputs.begin(), boost::find( real_inputs, node ) );
      return total_simulator.get_input( node, name, real_pos, aig );
    }
  }

  T get_constant() const { return total_simulator.get_constant(); }
  T invert( const T& v ) const { return total_simulator.invert( v ); }
  T and_op( const aig_node& node, const T& v1, const T& v2 ) const { return total_simulator.and_op( node, v1, v2 ); }

private:
  const aig_simulator<T>& total_simulator;
  const std::map<aig_node, T>& assignment;

  std::vector<aig_node> real_inputs;
};

/******************************************************************************
 * DFS visitor for actual simulation                                          *
 ******************************************************************************/

typedef std::map<aig_node, boost::default_color_type> aig_node_color_map;

template<typename T>
struct simulate_aig_node_visitor : public aig_dfs_visitor
{
public:
  simulate_aig_node_visitor( const aig_graph& aig, const aig_simulator<T>& simulator, std::map<aig_node, T>& node_values )
    : aig_dfs_visitor( aig ),
      simulator( simulator ),
      node_values( node_values ) {}

  void finish_input( const aig_node& node, const std::string& name, const aig_graph& aig )
  {
    unsigned pos = std::distance( graph_info.inputs.begin(), boost::find( graph_info.inputs, node ) );
    node_values[node] = simulator.get_input( node, name, pos, aig );
  }

  void finish_constant( const aig_node& node, const aig_graph& aig )
  {
    node_values[node] = simulator.get_constant();
  }

  void finish_aig_node( const aig_node& node, const aig_function& left, const aig_function& right, const aig_graph& aig )
  {
    T tleft  = node_values[left.first];
    T tright = node_values[right.first];

    node_values[node] = simulator.and_op( node, left.second ? simulator.invert( tleft ) : tleft, right.second ? simulator.invert( tright ) : tright );
  }

private:
  const aig_simulator<T>& simulator;
  std::map<aig_node, T>& node_values;
};

/******************************************************************************
 * Methods to trigger simulation                                              *
 ******************************************************************************/

template<typename T>
T simulate_aig_node( const aig_graph& aig, const aig_node& node,
                     const aig_simulator<T>& simulator,
                     aig_node_color_map& colors,
                     std::map<aig_node, T>& node_values )
{
  boost::depth_first_visit( aig, node,
                            simulate_aig_node_visitor<T>( aig, simulator, node_values ),
                            boost::make_assoc_property_map( colors ),
                            [&simulator]( const aig_node& node, const aig_graph& aig ) { return simulator.terminate( node, aig ); } );

  return node_values[node];
}

template<typename T>
T simulate_aig_node( const aig_graph& aig, const aig_node& node,
                     const aig_simulator<T>& simulator )
{
  aig_node_color_map colors;
  std::map<aig_node, T> node_values;

  return simulate_aig_node<T>( aig, node, simulator, colors, node_values );
}

template<typename T>
T simulate_aig_function( const aig_graph& aig, const aig_function& f,
                         const aig_simulator<T>& simulator,
                         aig_node_color_map& colors,
                         std::map<aig_node, T>& node_values )
{
  T value = simulate_aig_node<T>( aig, f.first, simulator, colors, node_values );
  return f.second ? simulator.invert( value ) : value;
}

template<typename T>
T simulate_aig_function( const aig_graph& aig, const aig_function& f,
                         const aig_simulator<T>& simulator )
{
  T value = simulate_aig_node<T>( aig, f.first, simulator );
  return f.second ? simulator.invert( value ) : value;
}

template<typename T>
void simulate_aig( const aig_graph& aig, const aig_simulator<T>& simulator, std::map<aig_function, T>& results,
                   const properties::ptr& settings = properties::ptr(),
                   const properties::ptr& statistics = properties::ptr() )
{
  /* settings */
  auto verbose = get( settings, "verbose", false );

  /* timer */
  new_properties_timer t( statistics );

  aig_node_color_map colors;
  std::map<aig_node, T> node_values;

  for ( const auto& o : boost::get_property( aig, boost::graph_name ).outputs )
  {
    if ( verbose )
    {
      std::cout << "[i] simulate '" << o.second << "'" << std::endl;
    }
    T value = simulate_aig_node<T>( aig, o.first.first, simulator, colors, node_values );
    if ( o.first.second )
    {
      value = simulator.invert( value );
    }
    results[o.first] = value;
  }

  if ( statistics )
  {
    statistics->set( "node_values", node_values );
  }
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
