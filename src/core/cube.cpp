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

#include "cube.hpp"

#include <cstdlib>
#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>

#include <core/io/pla_parser.hpp>
#include <core/utils/range_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

cube_assigner::cube_assigner( cube& c, unsigned index )
  : c( c ), index( index )
{
}

void cube_assigner::operator=( char v )
{
  c.data.first[index] = ( v == '1' );
  c.data.second[index] = ( v != '-' );
}

cube::cube()
  : cube( 0u )
{
}

cube::cube( unsigned n )
  : data( {boost::dynamic_bitset<>( n ), boost::dynamic_bitset<>( n )} )
{
}

cube::cube( const boost::dynamic_bitset<>& bits, const boost::dynamic_bitset<>& care )
  : data( {bits, care} )
{
}

cube::cube( const std::string& s )
  : cube( s.size() )
{
  for ( auto it : index( s ) )
  {
    operator[]( it.first ) = it.second;
  }
}

unsigned cube::match( const cube& other ) const
{
  auto check = care() & other.care() & ~( bits() ^ other.bits() );
  return check.count();
}

int cube::match_intersect( const cube& other ) const
{
  auto _care = care() & other.care();
  if ( ( _care & ( bits() ^ other.bits() ) ).any() )
  {
    return -1;
  }
  else
  {
    return ( _care & ~( bits() ^ other.bits() ) ).count();
  }
}

std::vector<cube> cube::disjoint_sharp( const cube& other ) const
{
  std::vector<cube> vec;

  /* transform to positional notation */
  auto bpos = ~bits();
  auto cpos = ~bits() ^ care();

  auto bposother = ~other.bits();
  auto cposother = ~other.bits() ^ other.care();
  boost::dynamic_bitset<> first_mask( length() );

  for ( unsigned i = 0u; i < length(); ++i )
  {
    first_mask.set( i );

    if ( !other.care()[i] || ( care()[i] && ( bits()[i] == other.bits()[i] ) ) )
    {
      continue;
    }

    bposother.flip( i ); cposother.flip( i );
    auto _bits = bpos & bposother & first_mask | bpos & ~first_mask;
    auto _care = cpos & cposother & first_mask | cpos & ~first_mask;
    bposother.flip( i ); cposother.flip( i );

    vec += cube( ~_bits, _bits ^ _care );
  }

  return vec;
}

cube_assigner cube::operator[]( unsigned index )
{
  return cube_assigner( *this, index );
}

std::string cube::to_string() const
{
  std::string str( length(), '0' );
  for ( unsigned i = 0u; i < length(); ++i )
  {
    str[i] = data.second[i] ? ( data.first[i] ? '1' : '0' ) : '-';
  }
  return str;
}

bool cube::operator==( const cube& other ) const
{
  return data == other.data;
}

bool cube::operator<( const cube& other ) const
{
  return data < other.data;
}

class common_pla_read_single_processor : public pla_processor
{
public:
  explicit common_pla_read_single_processor( cube_vec_t& cubes, unsigned output )
    : cubes( cubes ),
      output( output )
  {
  }

  void on_num_outputs( unsigned num_outputs )
  {
    assert( output < num_outputs );
  }

  void on_cube( const std::string& in, const std::string& out )
  {
    if ( out[output] == '1' )
    {
      cubes += cube( in );
    }
  }

private:
  cube_vec_t& cubes;
  unsigned output;
};

class common_pla_read_processor : public pla_processor
{
public:
  explicit common_pla_read_processor( cube_vec_vec_t& cubes )
    : cubes( cubes )
  {
  }

  void on_num_outputs( unsigned num_outputs )
  {
    cubes.resize( num_outputs );
  }

  void on_cube( const std::string& in, const std::string& out )
  {
    cube c( in );

    for ( auto it : index( out ) )
    {
      if ( it.second == '1' )
      {
        cubes[it.first] += c;
      }
    }
  }

private:
  cube_vec_vec_t& cubes;
};

cube_vec_t common_pla_read_single( const std::string& filename, unsigned output )
{
  cube_vec_t cubes;
  common_pla_read_single_processor p( cubes, output );

  pla_parser( filename, p );

  return cubes;
}

cube_vec_vec_t common_pla_read( const std::string& filename )
{
  cube_vec_vec_t cubes;
  common_pla_read_processor p( cubes );

  pla_parser( filename, p );

  return cubes;
}

void common_pla_write_single( const cube_vec_t& cubes, const std::string& filename )
{
  assert( cubes.size() );

  std::ofstream os( filename.c_str(), std::ofstream::out );

  os << boost::format( ".i %d" ) % cubes.front().length() << std::endl
     << ".o 1" << std::endl;

  for ( const auto& c : cubes )
  {
    os << c.to_string() << " 1" << std::endl;
  }
  os << ".e" << std::endl;

  os.close();
}

void common_pla_write( const cube_vec_vec_t& cubes, const std::string& filename )
{
  assert( cubes.size() );

  std::ofstream os( filename.c_str(), std::ofstream::out );

  os << boost::format( ".i %d" ) % cubes.front().front().length() << std::endl
     << boost::format( ".o %d" ) % cubes.size() << std::endl;

  /* share cubes */
  std::map<cube, boost::dynamic_bitset<>> cube_map;
  for ( auto it : index( cubes ) )
  {
    for ( const auto& c : it.second )
    {
      auto itc = cube_map.find( c );
      if ( itc == cube_map.end() )
      {
        boost::dynamic_bitset<> mask( cubes.size() );
        mask.set( it.first );
        cube_map[c] = mask;
      }
      else
      {
        itc->second.set( it.first );
      }
    }
  }

  /* print cubes */
  for ( const auto& p : cube_map )
  {
    os << p.first.to_string() << " ";
    for ( unsigned i = 0u; i < p.second.size(); ++i )
    {
      os << ( p.second[i] ? "1" : "0" );
    }
    os << std::endl;
  }

  os.close();
}

void common_pla_print( const cube_vec_t& cubes, std::ostream& os )
{
  for ( const auto& c : cubes )
  {
    os << c.to_string() << std::endl;
  }
}

cube_vec_t common_pla_espresso( const cube_vec_t& cubes )
{
  common_pla_write_single( cubes, "/tmp/test.pla" );
  system( "espresso -t /tmp/test.pla > /tmp/test2.pla" );
  return common_pla_read_single( "/tmp/test2.pla" );
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
