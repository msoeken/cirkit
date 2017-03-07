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

#include "cut_enumeration.hpp"

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * structural_cut                                                             *
 ******************************************************************************/

template<>
boost::dynamic_bitset<> cut_from_index( unsigned index, unsigned n )
{
  return onehot_bitset( n, index );
}

template<>
boost::dynamic_bitset<> empty_cut( unsigned n )
{
  return boost::dynamic_bitset<>( n );
}

template<>
unsigned cut_size<>( const boost::dynamic_bitset<>& cut )
{
  return cut.count();
}

template<>
boost::dynamic_bitset<> cut_union<>( const boost::dynamic_bitset<>& cut1, const boost::dynamic_bitset<>& cut2 )
{
  return cut1 | cut2;
}

template<>
boost::dynamic_bitset<>& cut_insert<>( boost::dynamic_bitset<>& cut, unsigned index )
{
  cut.set( index );
  return cut;
}

template<>
void foreach_node_in_cut<>( const boost::dynamic_bitset<>& cut, const std::function<void(unsigned)>&& f )
{
  auto pos = cut.find_first();

  while ( pos != boost::dynamic_bitset<>::npos )
  {
    f( pos );
    pos = cut.find_next( pos );
  }
}

template<>
std::ostream& print_cut<>( std::ostream& os, const boost::dynamic_bitset<>& cut )
{
  print_as_set( os, cut );
  return os;
}

/******************************************************************************
 * std::vector<std::set<unsigned>>                                            *
 ******************************************************************************/

template<>
std::set<unsigned> cut_from_index( unsigned index, unsigned n )
{
  return {index};
}

template<>
std::set<unsigned> empty_cut( unsigned n )
{
  return std::set<unsigned>{};
}

template<>
unsigned cut_size<>( const std::set<unsigned>& cut )
{
  return cut.size();
}

template<>
std::set<unsigned> cut_union<>( const std::set<unsigned>& cut1, const std::set<unsigned>& cut2 )
{
  std::set<unsigned> cut;
  std::set_union( cut1.begin(), cut1.end(), cut2.begin(), cut2.end(), std::insert_iterator<std::set<unsigned>>( cut, cut.begin() ) );
  return cut;
}

template<>
std::set<unsigned>& cut_insert<>( std::set<unsigned>& cut, unsigned index )
{
  cut.insert( index );
  return cut;
}

template<>
void foreach_node_in_cut<>( const std::set<unsigned>& cut, const std::function<void(unsigned)>&& f )
{
  for ( unsigned i : cut ) { f( i ); }
}

template<>
std::ostream& print_cut<>( std::ostream& os, const std::set<unsigned>& cut )
{
  if ( cut.empty() )
  {
    return os << "{ }";
  }
  else
  {
    return os << "{ " << any_join( cut, " " ) << " }";
  }
}

/******************************************************************************
 * std::vector<std::vector<unsigned>>                                         *
 ******************************************************************************/

template<>
std::vector<unsigned> cut_from_index( unsigned index, unsigned n )
{
  return {index};
}

template<>
std::vector<unsigned> empty_cut( unsigned n )
{
  return {};
}

template<>
unsigned cut_size<>( const std::vector<unsigned>& cut )
{
  return cut.size();
}

template<>
std::vector<unsigned> cut_union<>( const std::vector<unsigned>& cut1, const std::vector<unsigned>& cut2 )
{
  std::vector<unsigned> cut;
  std::set_union( cut1.begin(), cut1.end(), cut2.begin(), cut2.end(), std::back_inserter( cut ) );
  return cut;
}

template<>
std::vector<unsigned>& cut_insert<>( std::vector<unsigned>& cut, unsigned index )
{
  const auto it = std::lower_bound( cut.begin(), cut.end(), index );
  if ( it != cut.end() && !( index < *it ) )
  {
    cut.insert( it, index );
  }
  return cut;
}

template<>
void foreach_node_in_cut<>( const std::vector<unsigned>& cut, const std::function<void(unsigned)>&& f )
{
  for ( unsigned i : cut ) { f( i ); }
}

template<>
std::ostream& print_cut<>( std::ostream& os, const std::vector<unsigned>& cut )
{
  if ( cut.empty() )
  {
    return os << "{ }";
  }
  else
  {
    return os << "{ " << any_join( cut, " " ) << " }";
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
