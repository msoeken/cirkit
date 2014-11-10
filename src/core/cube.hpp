/* Copyright (C) 2009-2014  University of Bremen
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
 * @file cube.hpp
 *
 * @brief Data structure for cube representation
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef CUBE_HPP
#define CUBE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

class cube;

class cube_assigner
{
public:
  cube_assigner( cube& c, unsigned index );

  void operator=( char v );

private:
  cube& c;
  unsigned index;
};

class cube
{
public:
  friend class cube_assigner;

  cube();
  explicit cube( unsigned n );
  cube( const boost::dynamic_bitset<>& bits, const boost::dynamic_bitset<>& care );
  explicit cube( const std::string& s );

  inline boost::dynamic_bitset<> bits() const { return data.first; }
  inline boost::dynamic_bitset<> care() const { return data.second; }
  inline unsigned length() const { return data.first.size(); }
  inline unsigned dimension() const { return data.second.count(); }
  unsigned match( const cube& other ) const;
  int match_intersect( const cube& other ) const;
  std::vector<cube> disjoint_sharp( const cube& other ) const;

  cube_assigner operator[]( unsigned index );
  std::string to_string() const;

  bool operator==( const cube& other ) const;
  bool operator!=( const cube& other ) const;
  bool operator<( const cube& other ) const;
private:
  std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>> data;
};

typedef std::vector<cube>       cube_vec_t;
typedef std::vector<cube_vec_t> cube_vec_vec_t;

cube_vec_t     common_pla_read_single( const std::string& filename, unsigned output = 0u );
cube_vec_vec_t common_pla_read( const std::string& filename );
void           common_pla_write_single( const cube_vec_t& cubes, const std::string& filename );
void           common_pla_write( const cube_vec_vec_t& cubes, const std::string& filename );
void           common_pla_print( const cube_vec_t& cubes, std::ostream& os = std::cout );

cube_vec_t common_pla_espresso( const cube_vec_t& cubes );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
