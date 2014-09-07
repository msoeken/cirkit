/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
  std::stringstream s;
  s << a;
  return boost::dynamic_bitset<>( s.str().substr( 2u ) );
}

const bool expr_to_bool( const z3::expr& e )
{
  assert( e.decl().decl_kind() == Z3_OP_TRUE ||
          e.decl().decl_kind() == Z3_OP_FALSE );
  std::ostringstream ss; ss << e;
  assert( ss.str() == "true" || ss.str() == "false" );
  return ( ss.str() == "true" );
}

/*
 * Based on metaSMT's
 *   result_type operator() (bvtags::bvhex_tag,boost::any arg) in
 *   ''Z3_Backend.hpp''.
 */
std::string hex_string_to_bin_string( const std::string& hex )
{
  std::string bin( 4u*hex.size(), '\0' );
  for ( unsigned i = 0; i < hex.size(); ++i )
  {
    switch ( tolower( hex[i] ) )
    {
    case '0': bin.replace( 4u*i, 4u, "0000" ); break;
    case '1': bin.replace( 4u*i, 4u, "0001" ); break;
    case '2': bin.replace( 4u*i, 4u, "0010" ); break;
    case '3': bin.replace( 4u*i, 4u, "0011" ); break;
    case '4': bin.replace( 4u*i, 4u, "0100" ); break;
    case '5': bin.replace( 4u*i, 4u, "0101" ); break;
    case '6': bin.replace( 4u*i, 4u, "0110" ); break;
    case '7': bin.replace( 4u*i, 4u, "0111" ); break;
    case '8': bin.replace( 4u*i, 4u, "1000" ); break;
    case '9': bin.replace( 4u*i, 4u, "1001" ); break;
    case 'a': bin.replace( 4u*i, 4u, "1010" ); break;
    case 'b': bin.replace( 4u*i, 4u, "1011" ); break;
    case 'c': bin.replace( 4u*i, 4u, "1100" ); break;
    case 'd': bin.replace( 4u*i, 4u, "1101" ); break;
    case 'e': bin.replace( 4u*i, 4u, "1110" ); break;
    case 'f': bin.replace( 4u*i, 4u, "1111" ); break;
    }
  }
  return bin;
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
    return hex_string_to_bin_string( val );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
