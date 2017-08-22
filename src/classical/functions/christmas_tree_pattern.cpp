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

#include "christmas_tree_pattern.hpp"

#include <string>
#include <vector>

#include <boost/algorithm/string/join.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::vector<std::vector<std::string>> get_order0_tree()
{
  return {{""}};
}

std::vector<std::vector<std::string>> increase_order( const std::vector<std::vector<std::string>>& s )
{
  std::vector<std::vector<std::string>> result;

  for ( const auto& row : s )
  {
    assert( !row.empty() );

    if ( row.size() != 1u )
    {
      result.push_back( std::vector<std::string>() );
      for ( auto i = 1u; i < row.size(); ++i )
      {
        result.back().push_back( row[i] + "0" );
      }
    }

    result.push_back( std::vector<std::string>() );
    result.back().push_back( row.front() + "0" );
    for ( auto i = 0u; i < row.size(); ++i )
    {
      result.back().push_back( row[i] + "1" );
    }
  }

  return result;
}

void print_row( const std::vector<std::string>& row, const std::function<unsigned(const boost::dynamic_bitset<>&)>& mark, std::ostream& os )
{
  for ( auto i = 0u; i < row.size(); ++i )
  {
    boost::dynamic_bitset<> bs( row[i] );

    const auto mc = mark( bs );
    if ( mc != 0u )
    {
      os << "\033[1;3" << mc << "m";
    }
    os << bs;
    if ( mc != 0u )
    {
      os << "\033[0m";
    }

    if ( i != row.size() - 1u )
    {
      os << " ";
    }
  }
  os << std::endl;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned print_christmas_tree_pattern_no_mark( const boost::dynamic_bitset<>& bs )
{
  return 0;
}

void print_christmas_tree_pattern( unsigned n, const std::function<unsigned(const boost::dynamic_bitset<>&)>& mark, std::ostream& os )
{
  auto rows = get_order0_tree();
  for ( auto i = 0u; i < n; ++i )
  {
    rows = increase_order( rows );
  }

  const auto numcols = n + 1;

  for ( const auto& row : rows )
  {
    const auto offset = ( numcols - row.size() ) / 2;

    os << std::string( offset * n + offset, ' ' );
    print_row( row, mark, os );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
