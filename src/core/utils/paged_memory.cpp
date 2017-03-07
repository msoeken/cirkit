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

#include "paged_memory.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/numeric.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * paged_memory::set                                                          *
 ******************************************************************************/

paged_memory::set::set( unsigned address, std::vector<unsigned>& data, unsigned additional )
  : _address( address ),
    data( data ),
    additional( additional )
{
}

std::size_t paged_memory::set::size() const
{
  return data[_address];
}

paged_memory::set::iterator paged_memory::set::begin() const
{
  return data.begin() + _address + 1 + additional;
}

paged_memory::set::iterator paged_memory::set::end() const
{
  return data.begin() + _address + 1 + additional + size();
}

boost::iterator_range<paged_memory::set::iterator> paged_memory::set::range() const
{
  return boost::make_iterator_range( begin(), end() );
}

paged_memory::set::value_type paged_memory::set::extra( unsigned i ) const
{
  return data[_address + 1 + i];
}

void paged_memory::set::set_extra( unsigned i, value_type v )
{
  data[_address + 1 + i] = v;
}

unsigned paged_memory::set::address() const
{
  return _address;
}

/******************************************************************************
 * paged_memory::iterator                                                     *
 ******************************************************************************/

paged_memory::iterator::iterator( unsigned index, unsigned address, std::vector<unsigned>& data, unsigned additional )
  : index( index ),
    address( address ),
    data( data ),
    additional( additional )
{
}

paged_memory::iterator& paged_memory::iterator::operator=( const iterator& other )
{
  if ( this != &other )
  {
    index = other.index;
    address = other.address;
    data = other.data;
    additional = other.additional;
  }
  return *this;
}

paged_memory::iterator::reference paged_memory::iterator::operator*() const
{
  return set( address, data, additional );
}

paged_memory::iterator& paged_memory::iterator::operator++()
{
  address += data[address] + 1u + additional;
  ++index;
  return *this;
}

paged_memory::iterator paged_memory::iterator::operator++(int)
{
  iterator tmp( *this );
  ++*this;
  return tmp;
}

bool paged_memory::iterator::operator==( iterator const& it ) const
{
  return index == it.index;
}

bool paged_memory::iterator::operator!=( iterator const& it ) const
{
  return !( *this == it );
}

/******************************************************************************
 * paged_memory                                                               *
 ******************************************************************************/

paged_memory::paged_memory( unsigned n, unsigned k )
  : _additional( k ),
    _offset( n ),
    _count( n, 0u )
{
  _data.reserve( n << 1u );
}

unsigned paged_memory::count( unsigned index ) const
{
  return _count[index];
}

unsigned paged_memory::memory() const
{
  return sizeof( unsigned ) * ( _data.size() + _offset.size() + _count.size() + 2u ) + sizeof( double );
}

boost::iterator_range<paged_memory::iterator> paged_memory::sets( unsigned index )
{
  return boost::make_iterator_range( iterator( 0u, _offset[index], _data, _additional ),
                                     iterator( _count[index], 0, _data, _additional ) );
}

unsigned paged_memory::sets_count() const
{
  return boost::accumulate( _count, 0u );
}

unsigned paged_memory::index( const set& s ) const
{
  return std::distance( _offset.begin(), boost::lower_bound( _offset, s._address ) );
}

paged_memory::set paged_memory::from_address( unsigned address )
{
  return set( address, _data, _additional );
}

paged_memory::set paged_memory::from_index( unsigned index )
{
  return set( _offset[index], _data, _additional );
}

void paged_memory::assign_empty( unsigned index, const std::vector<unsigned>& extra )
{
  _offset[index] = _data.size();
  _count[index] = 1u;
  _data += 0u;
  boost::push_back( _data, extra );
}

void paged_memory::assign_singleton( unsigned index, unsigned value, const std::vector<unsigned>& extra )
{
  _offset[index] = _data.size();
  _count[index] = 1u;
  _data += 1u;
  boost::push_back( _data, extra );
  _data += value;
}

void paged_memory::append_begin( unsigned index )
{
  _offset[index] = _data.size();
}

void paged_memory::append_singleton( unsigned index, unsigned value, const std::vector<unsigned>& extra )
{
  _count[index]++;
  _data += 1u;
  boost::push_back( _data, extra );
  _data += value;
}

void paged_memory::append_set( unsigned index, const std::vector<unsigned>& values, const std::vector<unsigned>& extra )
{
  _count[index]++;
  _data += values.size();
  boost::push_back( _data, extra );
  boost::push_back( _data, values );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
