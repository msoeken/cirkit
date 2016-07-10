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
  static const std::string bin2hex = "0123456789ABCDEF";

  char result = 0;

  for ( char n = 0; n < bits.length(); ++n )
  {
    result <<= 1;
    result |= ( bits[n] == '1' ? 1 : 0 );
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
