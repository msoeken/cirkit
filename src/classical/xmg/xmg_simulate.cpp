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

#include "xmg_simulate.hpp"

#include <core/utils/range_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Pattern simulation                                                         *
 ******************************************************************************/

xmg_pattern_simulator::xmg_pattern_simulator( const boost::dynamic_bitset<>& pattern )
  : pattern( pattern )
{
}

bool xmg_pattern_simulator::get_input( const xmg_node& node, const xmg_graph& xmg ) const
{
  return pattern[xmg.input_index( node )];
}

bool xmg_pattern_simulator::get_constant() const
{
  return false;
}

bool xmg_pattern_simulator::invert( const bool& v ) const
{
  return !v;
}

bool xmg_pattern_simulator::xor_op( const xmg_node& node, const bool& v1, const bool& v2 ) const
{
  return v1 != v2;
}

bool xmg_pattern_simulator::maj_op( const xmg_node& node, const bool& v1, const bool& v2, const bool& v3 ) const
{
  return ( v1 && v2 ) || ( v1 && v3 ) || ( v2 && v3 );
}

/******************************************************************************
 * Truth table simulation                                                     *
 ******************************************************************************/

tt xmg_tt_simulator::get_input( const xmg_node& node, const xmg_graph& xmg ) const
{
  return tt_nth_var( xmg.input_index( node ) );
}

tt xmg_tt_simulator::get_constant() const
{
  return tt_const0();
}

tt xmg_tt_simulator::invert( const tt& v ) const
{
  return ~v;
}

tt xmg_tt_simulator::xor_op( const xmg_node& node, const tt& v1, const tt& v2 ) const
{
  tt _v1 = v1;
  tt _v2 = v2;
  tt_align( _v1, _v2 );
  return v1 ^ v2;
}

tt xmg_tt_simulator::maj_op( const xmg_node& node, const tt& v1, const tt& v2, const tt& v3 ) const
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

BDD xmg_bdd_simulator::get_input( const xmg_node& node, const xmg_graph& xmg ) const
{
  return mgr.bddVar( xmg.input_index( node ) );
}

BDD xmg_bdd_simulator::get_constant() const
{
  return mgr.bddZero();
}

BDD xmg_bdd_simulator::invert( const BDD& v ) const
{
  return !v;
}

BDD xmg_bdd_simulator::xor_op( const xmg_node& node, const BDD& v1, const BDD& v2 ) const
{
  return v1.Xor( v2 );
}

BDD xmg_bdd_simulator::maj_op( const xmg_node& node, const BDD& v1, const BDD& v2, const BDD& v3 ) const
{
  return ( v1 & v2 ) | ( v1 & v3 ) | ( v2 & v3 );
}

/******************************************************************************
 * Utility functions for simulation                                           *
 ******************************************************************************/

boost::dynamic_bitset<> xmg_simulate_pattern( const xmg_graph& xmg, const boost::dynamic_bitset<>& pattern )
{
  xmg_pattern_simulator sim( pattern );
  const auto result = simulate_xmg( xmg, sim );

  boost::dynamic_bitset<> opattern( xmg.outputs().size() );
  for ( const auto& po : index( xmg.outputs() ) )
  {
    opattern.set( po.index, result.at( po.value.first ) );
  }

  return opattern;
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
