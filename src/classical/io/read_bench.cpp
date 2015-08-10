/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include "read_bench.hpp"

#include <classical/utils/aig_utils.hpp>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <sstream>
#include <iterator>

namespace cirkit
{

namespace detail
{

/**
 * Based on http://stackoverflow.com/questions/5607589/right-way-to-split-an-stdstring-into-a-vectorstring
 */
struct tokens : std::ctype< char >
{
  tokens() : std::ctype<char>( get_table() ) {}

  static std::ctype_base::mask const* get_table()
  {
    using cctype = std::ctype< char >;
    static const cctype::mask* const_rc = cctype::classic_table();

    static cctype::mask rc[ cctype::table_size ];
    std::memcpy( rc, const_rc, cctype::table_size * sizeof(cctype::mask) );

    rc[','] = std::ctype_base::space;
    rc['='] = std::ctype_base::space;
    rc['('] = std::ctype_base::space;
    rc[')'] = std::ctype_base::space;
    rc[' '] = std::ctype_base::space;
    return &rc[0];
  }
};

}

void read_bench( aig_graph& aig, std::ifstream& is )
{
  std::map< std::string, aig_function > the_map;
  std::list< std::string > pos;

  aig_initialize( aig );

  std::string line;
  while ( std::getline( is, line ) )
  {
    // std::cout << line << '\n';
    if ( line == "" || boost::starts_with( line, "#" ) ) continue;
    std::istringstream iis( line );
    iis.imbue( std::locale( std::locale(), new detail::tokens() ) );

    std::istream_iterator< std::string > iit( iis ), eos;
    while ( iit != eos )
    {
      const auto token = *iit;
      // std::cout << token << '\n';
      if ( token == "INPUT" )
      {
        ++iit;
        const auto name = *iit;
        the_map.insert( {name, aig_create_pi( aig, name )} );
        ++iit;
        continue;
      }

      if ( token == "OUTPUT" )
      {
        ++iit;
        const auto name = *iit;
        pos.push_back( name );
        ++iit;
        continue;
      }

      const std::string res = *iit; ++iit;
      const std::string kind = boost::to_upper_copy( *iit ); ++iit;
      std::vector< aig_function > ops;
      while ( iit != eos )
      {
        ops.push_back( the_map[ *iit ] );
        ++iit;
      }

      const auto num_ops = ops.size();
      assert( num_ops > 0u );

      /* unary operators */
      if ( num_ops == 1u )
      {
        assert( kind == "NOT" || kind == "BUF" );
        if ( kind == "NOT" )
        {
          the_map.insert( {res, !ops[0u]} );
        }
        else if ( kind == "BUF" )
        {
          the_map.insert( {res, aig_create_and( aig, ops[0u], ops[0u] ) } );
        }
        continue;
      }

      /* nary operators */
      if ( kind == "AND" )
      {
        the_map.insert( {res, aig_create_nary_and( aig, ops ) } );
        continue;
      }
      else if ( kind == "NAND" )
      {
        the_map.insert( {res, aig_create_nary_nand( aig, ops ) } );
        continue;
      }
      else if ( kind == "OR" )
      {
        the_map.insert( {res, aig_create_nary_or( aig, ops ) } );
        continue;
      }
      else if ( kind == "NOR" )
      {
        the_map.insert( {res, aig_create_nary_nor( aig, ops ) } );
        continue;
      }
      else if ( kind == "XOR" )
      {
        the_map.insert( {res, aig_create_nary_xor( aig, ops ) } );
        continue;
      }

      // std::cout << kind << '\n';
      assert( false && "Yet not implemented gate type" );
    }
  }

  for ( const auto po : pos )
  {
    aig_create_po( aig, the_map[ po ], po );
  }
}

void read_bench( aig_graph& aig, const std::string& filename )
{
  std::ifstream is( filename.c_str() );
  read_bench( aig, is );
  auto& info = aig_info( aig );
  info.model_name = boost::filesystem::path( filename ).stem().string();
  is.close();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
