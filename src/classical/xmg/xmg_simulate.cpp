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

#include "xmg_simulate.hpp"

namespace cirkit
{

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

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
