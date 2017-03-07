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
 * @file conversion_utils.hpp
 *
 * @brief Some helper functions for conversion
 *
 * @author Mathias Soeken
 * @author Heinz Riener
 * @since  2.0
 */

#include "conversion_utils.hpp"

#include <cassert>

namespace cirkit
{

  /* inspired by http://ubuntuforums.org/showthread.php?t=739716 */
char convert_bin2hex( const std::string& bits )
{
  static const std::string bin2hex = "0123456789abcdef";

  char result = 0;

  for ( const auto& bit : bits )
  {
    result <<= 1;
    result |= ( bit == '1' ? 1 : 0 );
  }
  return bin2hex[result];
}

/*
 * Based on metaSMT's
 *   result_type operator() (bvtags::bvhex_tag,boost::any arg) in
 *   ''Z3_Backend.hpp''.
 */
std::string convert_hex2bin( const char& hex )
{
  switch ( tolower( hex ) )
  {
  case '0': return "0000";
  case '1': return "0001";
  case '2': return "0010";
  case '3': return "0011";
  case '4': return "0100";
  case '5': return "0101";
  case '6': return "0110";
  case '7': return "0111";
  case '8': return "1000";
  case '9': return "1001";
  case 'a': return "1010";
  case 'b': return "1011";
  case 'c': return "1100";
  case 'd': return "1101";
  case 'e': return "1110";
  case 'f': return "1111";
  default: break;
  }
  return "XXXX";
}

std::string convert_hex2bin( const std::string& hex )
{
  std::string bin( 4u*hex.size(), '\0' );
  for ( unsigned i = 0u; i < hex.size(); ++i )
  {
    bin.replace( 4u*i, 4u, convert_hex2bin( hex[i] ) );
  }
  return bin;
}

std::string invert_hex( const std::string& hex )
{
  std::string res;
  for ( auto c : hex )
  {
    switch ( c )
    {
    case '0': res += 'f'; break;
    case '1': res += 'e'; break;
    case '2': res += 'd'; break;
    case '3': res += 'c'; break;
    case '4': res += 'b'; break;
    case '5': res += 'a'; break;
    case '6': res += '9'; break;
    case '7': res += '8'; break;
    case '8': res += '7'; break;
    case '9': res += '6'; break;
    case 'a': res += '5'; break;
    case 'b': res += '4'; break;
    case 'c': res += '3'; break;
    case 'd': res += '2'; break;
    case 'e': res += '1'; break;
    case 'f': res += '0'; break;
    default: assert( false );
    }
  }

  return res;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
