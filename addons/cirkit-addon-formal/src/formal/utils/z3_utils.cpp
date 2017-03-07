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
    std::cerr << "[e] " << Z3_get_error_msg( ctx, e ) << std::endl;
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

z3::expr logic_xor( const z3::expr& a, const z3::expr& b )
{
  check_context( a, b );
  assert( a.is_bool() && b.is_bool() );
  Z3_ast r = Z3_mk_xor( a.ctx(), a, b );
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
