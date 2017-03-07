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
