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

#include "z3_utils.hpp"

#include <core/utils/conversion_utils.hpp>

#include <iostream>

namespace cirkit
{

/*
 * Based on
 *   http://z3.codeplex.com/SourceControl/latest#examples/tptp/tptp5.cpp
 *   (branch: unstable)
 */
void check_error( const z3::context& ctx )
{
  const Z3_error_code e = Z3_get_error_code( ctx );
  if ( e != Z3_OK )
  {
    std::cerr << "[E] " << Z3_get_error_msg_ex( ctx, e ) << std::endl;
    exit(1);
  }
}

z3::expr operator<<( const z3::expr& a, const z3::expr& b )
{
  check_context( a, b );
  assert( a.is_bv() && b.is_bv() );
  Z3_ast r = Z3_mk_bvshl( a.ctx(), a, b );
  return z3::to_expr( a.ctx(), r );
}

boost::dynamic_bitset<> to_bitset( const z3::expr& a )
{
  return boost::dynamic_bitset<>( expr_to_bin( a ) );
}

const bool expr_to_bool( const z3::expr& e )
{
  assert( e.decl().decl_kind() == Z3_OP_TRUE ||
          e.decl().decl_kind() == Z3_OP_FALSE );
  return ( e.decl().decl_kind() == Z3_OP_TRUE );
}

const std::string expr_to_bin( const z3::expr &e )
{
  assert( e.decl().decl_kind() == Z3_OP_BNUM );
  std::ostringstream ss; ss << e;
  std::string val = ss.str();
  if ( val[0] == '#' && val[1] == 'b' )
  {
    assert( val[0] == '#' && val[1] == 'b' );
    val = val.substr(2, val.size()-2);
    assert( val.size() == e.decl().range().bv_size() );
    return val;
  }
  else
  {
    assert ( val[0] == '#' && val[1] == 'x' );
    val = val.substr(2, val.size()-2);
    return convert_hex2bin( val );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
