/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "simulate_aig.hpp"

namespace cirkit
{

/******************************************************************************
 * Pattern simulation (single pattern)                                        *
 ******************************************************************************/

pattern_simulator::pattern_simulator( const boost::dynamic_bitset<>& pattern ) : pattern( pattern ) {}

bool pattern_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  return pattern[pos];
}

bool pattern_simulator::get_constant() const
{
  return false;
}

bool pattern_simulator::invert( const bool& v ) const
{
  return !v;
}

bool pattern_simulator::and_op( const aig_node& node, const bool& v1, const bool& v2 ) const
{
  return v1 && v2;
}

/******************************************************************************
 * Boolean simulation                                                         *
 ******************************************************************************/

simple_assignment_simulator::simple_assignment_simulator( const aig_name_value_map& assignment ) : assignment( assignment ) {}

bool simple_assignment_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  auto it = assignment.find( name );
  if ( it == assignment.end() )
  {
    std::cout << "[w] no assignment given for '" << name << "', assume 0" << std::endl;
    return false;
  }
  else
  {
    return it->second;
  }
}

bool simple_assignment_simulator::get_constant() const
{
  return false;
}

bool simple_assignment_simulator::invert( const bool& v ) const
{
  return !v;
}

bool simple_assignment_simulator::and_op( const aig_node& node, const bool& v1, const bool& v2 ) const
{
  return v1 && v2;
}

/******************************************************************************
 * Boolean simulation (on nodes)                                              *
 ******************************************************************************/

simple_node_assignment_simulator::simple_node_assignment_simulator( const aig_node_value_map& assignment ) : assignment( assignment ) {}

bool simple_node_assignment_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  auto it = assignment.find( node );
  if ( it == assignment.end() )
  {
    std::cout << "[w] no assignment given for '" << node << "', assume 0" << std::endl;
    return false;
  }
  else
  {
    return it->second;
  }
}

bool simple_node_assignment_simulator::get_constant() const
{
  return false;
}

bool simple_node_assignment_simulator::invert( const bool& v ) const
{
  return !v;
}

bool simple_node_assignment_simulator::and_op( const aig_node& node, const bool& v1, const bool& v2 ) const
{
  auto it = assignment.find( node );
  return ( it == assignment.end() ) ? ( v1 && v2 ) : it->second;
}

bool simple_node_assignment_simulator::terminate( const aig_node& node, const aig_graph& aig ) const
{
  return assignment.find( node ) != assignment.end();
}

/******************************************************************************
 * Word simulation                                                            *
 ******************************************************************************/

word_assignment_simulator::word_assignment_simulator( const aig_name_value_map& assignment ) : assignment( assignment ) {}

boost::dynamic_bitset<> word_assignment_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  auto it = assignment.find( name );
  if ( it == assignment.end() )
  {
    boost::dynamic_bitset<> v( assignment.begin()->second.size() );
    std::cout << "[w] no assignment given for '" << name << "', assume " << v << std::endl;
    return v;
  }
  else
  {
    return it->second;
  }
}

boost::dynamic_bitset<> word_assignment_simulator::get_constant() const
{
  return boost::dynamic_bitset<>( assignment.begin()->second.size() );
}

boost::dynamic_bitset<> word_assignment_simulator::invert( const boost::dynamic_bitset<>& v ) const
{
  return ~v;
}

boost::dynamic_bitset<> word_assignment_simulator::and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const
{
  return v1 & v2;
}

/******************************************************************************
 * Word simulation (on nodes)                                                 *
 ******************************************************************************/

word_node_assignment_simulator::word_node_assignment_simulator( const aig_node_value_map& assignment ) : assignment( assignment ) {}

boost::dynamic_bitset<> word_node_assignment_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  auto it = assignment.find( node );
  if ( it == assignment.end() )
  {
    boost::dynamic_bitset<> v( assignment.begin()->second.size() );
    std::cout << "[w] no assignment given for '" << node << "', assume " << v << std::endl;
    return v;
  }
  else
  {
    return it->second;
  }
}

boost::dynamic_bitset<> word_node_assignment_simulator::get_constant() const
{
  return boost::dynamic_bitset<>( assignment.begin()->second.size() );
}

boost::dynamic_bitset<> word_node_assignment_simulator::invert( const boost::dynamic_bitset<>& v ) const
{
  return ~v;
}

boost::dynamic_bitset<> word_node_assignment_simulator::and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const
{
  auto it = assignment.find( node );
  return ( it == assignment.end() ) ? ( v1 & v2 ) : it->second;
}

bool word_node_assignment_simulator::terminate( const aig_node& node, const aig_graph& aig ) const
{
  return assignment.find( node ) != assignment.end();
}

/******************************************************************************
 * Truth table simulation                                                     *
 ******************************************************************************/

tt tt_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  return tt_nth_var( pos );
}

tt tt_simulator::get_constant() const
{
  return tt_const0();
}

tt tt_simulator::invert( const tt& v ) const
{
  return ~v;
}

tt tt_simulator::and_op( const aig_node& node, const tt& v1, const tt& v2 ) const
{

  // NOTE: make this more performant?
  tt _v1 = v1;
  tt _v2 = v2;
  tt_align( _v1, _v2 );
  return _v1 & _v2;
}

/******************************************************************************
 * BDD simulation                                                             *
 ******************************************************************************/

BDD bdd_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  return mgr.bddVar( pos );
}

BDD bdd_simulator::get_constant() const
{
  return mgr.bddZero();
}

BDD bdd_simulator::invert( const BDD& v ) const
{
  return !v;
}

BDD bdd_simulator::and_op( const aig_node& node, const BDD& v1, const BDD& v2 ) const
{
  return v1 & v2;
}

/******************************************************************************
 * depth simulation                                                           *
 ******************************************************************************/

unsigned depth_simulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  return 0u;
}

unsigned depth_simulator::get_constant() const
{
  return 0u;
}

unsigned depth_simulator::invert( const unsigned& v ) const
{
  return v;
}

unsigned depth_simulator::and_op( const aig_node& node, const unsigned& v1, const unsigned& v2 ) const
{
  return std::max( v1, v2 ) + 1u;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
