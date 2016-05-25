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

#include "mig_simulate.hpp"

namespace cirkit
{

/******************************************************************************
 * Boolean simulation                                                         *
 ******************************************************************************/

mig_simple_assignment_simulator::mig_simple_assignment_simulator( const mig_name_value_map& assignment ) : assignment( assignment ) {}

bool mig_simple_assignment_simulator::get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
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

bool mig_simple_assignment_simulator::get_constant() const
{
  return false;
}

bool mig_simple_assignment_simulator::invert( const bool& v ) const
{
  return !v;
}

bool mig_simple_assignment_simulator::maj_op( const mig_node& node, const bool& v1, const bool& v2, const bool& v3 ) const
{
  return ( v1 && v2 ) || ( v1 && v3 ) || ( v2 && v3 );
}

/******************************************************************************
 * Truth table simulation                                                     *
 ******************************************************************************/

tt mig_tt_simulator::get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
{
  return tt_nth_var( pos );
}

tt mig_tt_simulator::get_constant() const
{
  return tt_const0();
}

tt mig_tt_simulator::invert( const tt& v ) const
{
  return ~v;
}

tt mig_tt_simulator::maj_op( const mig_node& node, const tt& v1, const tt& v2, const tt& v3 ) const
{
  // NOTE: make this more performant?
  tt _v1 = v1;
  tt _v2 = v2;
  tt _v3 = v3;
  tt_align( _v1, _v2 );
  tt_align( _v1, _v3 );
  tt_align( _v2, _v3 );
  return ( _v1 & _v2 ) | ( _v1 & _v3 ) | ( _v2 & _v3 );
}

/******************************************************************************
 * BDD simulation                                                             *
 ******************************************************************************/

BDD mig_bdd_simulator::get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
{
  return mgr.bddVar( pos );
}

BDD mig_bdd_simulator::get_constant() const
{
  return mgr.bddZero();
}

BDD mig_bdd_simulator::invert( const BDD& v ) const
{
  return !v;
}

BDD mig_bdd_simulator::maj_op( const mig_node& node, const BDD& v1, const BDD& v2, const BDD& v3 ) const
{
  return ( v1 & v2 ) | ( v1 & v3 ) | ( v2 & v3 );
}

/******************************************************************************
 * depth simulation                                                           *
 ******************************************************************************/

unsigned mig_depth_simulator::get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
{
  return 0u;
}

unsigned mig_depth_simulator::get_constant() const
{
  return 0u;
}

unsigned mig_depth_simulator::invert( const unsigned& v ) const
{
  return v;
}

unsigned mig_depth_simulator::maj_op( const mig_node& node, const unsigned& v1, const unsigned& v2, const unsigned& v3 ) const
{
  return std::max( v1, std::max( v2, v3 ) ) + 1u;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
