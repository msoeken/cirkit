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
 * @file cube.hpp
 *
 * @brief Data structure for cube representation
 *
 * @author Mathias Soeken
 * @since  2.1
 */

#ifndef CUBE_HPP
#define CUBE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>

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

using cube_vec_t     = std::vector<cube>;
using cube_vec_vec_t = std::vector<cube_vec_t>;

cube_vec_t     common_pla_read_single( const std::string& filename, unsigned output = 0u );
cube_vec_vec_t common_pla_read( const std::string& filename );
void           common_pla_write_single( const cube_vec_t& cubes, const std::string& filename );
void           common_pla_write( const cube_vec_vec_t& cubes, const std::string& filename,
                                 const properties::ptr& settings = properties::ptr(),
                                 const properties::ptr& statistics = properties::ptr() );
void           common_pla_print( const cube_vec_t& cubes, std::ostream& os = std::cout );

cube_vec_t common_pla_espresso( const cube_vec_t& cubes );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
