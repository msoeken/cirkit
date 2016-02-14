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
