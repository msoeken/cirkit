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

namespace cirkit
{

/* inspired by http://ubuntuforums.org/showthread.php?t=739716 */
std::string bin2hex = "0123456789ABCDEF";

char convert_bin2hex( const std::string& bits )
{
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

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
