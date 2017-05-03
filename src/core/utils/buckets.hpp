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
 * @file buckets.hpp
 *
 * @brief Buckets data structure with O(1) remove operation
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef BUCKETS_HPP
#define BUCKETS_HPP

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace cirkit
{

template<typename T>
void swap_remove( std::vector<T>& v, typename std::vector<T>::size_type index )
{
  v[index] = std::move( v.back() );
  v.pop_back();
}

template<typename T>
void swap_remove( std::vector<T>& v, typename std::vector<T>::iterator it )
{
  v[std::distance( v.begin(), it )] = std::move( v.back() );
  v.pop_back();
}

template<typename T>
class buckets
{
public:
  buckets( unsigned num_buckets ) : vecs( num_buckets ) {}

  inline void add( unsigned bucket, const T& v )
  {
    vecs[bucket].push_back( v );
    ++num_elements;
  }

  inline void remove_at( unsigned bucket, unsigned index )
  {
    swap_remove( vecs[bucket], index );
    --num_elements;
  }

  inline int find( unsigned bucket, const T& v ) const
  {
    const auto it = std::find( vecs[bucket].begin(), vecs[bucket].end(), v );
    return ( it == vecs[bucket].end() ) ? -1 : static_cast<int>( std::distance( vecs[bucket].begin(), it ) );
  }

  inline const T& get( unsigned bucket, unsigned index ) const
  {
    return vecs[bucket][index];
  }

  inline void clear( unsigned index )
  {
    num_elements -= vecs[index].size();
    vecs[index].clear();
  }

  inline typename std::vector<T>::size_type size( unsigned index ) const
  {
    return vecs[index].size();
  }

  inline typename std::vector<T>::const_iterator begin( unsigned index ) const
  {
    return vecs[index].begin();
  }

  inline typename std::vector<T>::const_iterator end( unsigned index ) const
  {
    return vecs[index].end();
  }

private:
  std::vector<std::vector<T>> vecs;
  unsigned num_elements = 0u;
};

template<typename T>
class hash_buckets
{
public:
  hash_buckets( unsigned num_buckets ) : vecs( num_buckets ), hash( num_buckets ) {}

  inline void add( unsigned bucket, const T& v )
  {
    hash[bucket][v] = vecs[bucket].size();
    vecs[bucket].push_back( v );
    ++num_elements;
  }

  inline void remove_at( unsigned bucket, unsigned index )
  {
    const auto v = vecs[bucket][index];
    const auto v2 = vecs[bucket].back();
    swap_remove( vecs[bucket], index );
    hash[bucket].erase( v );
    if ( v != v2 )
    {
      hash[bucket][v2] = index;
    }
    --num_elements;
  }

  inline int find( unsigned bucket, const T& v ) const
  {
    const auto it = hash[bucket].find( v );
    return ( it == hash[bucket].end() ) ? -1 : static_cast<int>( it->second );
  }

  inline const T& get( unsigned bucket, unsigned index ) const
  {
    return vecs[bucket][index];
  }

  inline void clear( unsigned index )
  {
    num_elements -= vecs[index].size();
    vecs[index].clear();
    hash[index].clear();
  }

  inline typename std::vector<T>::size_type size( unsigned index ) const
  {
    return vecs[index].size();
  }

  inline typename std::vector<T>::const_iterator begin( unsigned index ) const
  {
    return vecs[index].begin();
  }

  inline typename std::vector<T>::const_iterator end( unsigned index ) const
  {
    return vecs[index].end();
  }

private:
  std::vector<std::vector<T>> vecs;
  std::vector<std::unordered_map<T, typename std::vector<T>::size_type>> hash;
  unsigned num_elements = 0u;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
