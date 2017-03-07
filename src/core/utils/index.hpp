/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 * Copyright (C) 2015  The Regents of the University of California
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
 * @file index.hpp
 *
 * @brief Base indexes, maps, and sets
 *
 * @author Baruch Sterin
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CORE_UTILS_INDEX_HPP
#define CORE_UTILS_INDEX_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <stack>

#include <boost/range/algorithm.hpp>
#include <boost/dynamic_bitset.hpp>

namespace cirkit
{

/******************************************************************************
 * base_index                                                                 *
 ******************************************************************************/

template<typename Tag>
class base_index
{
public:
  /**
   * @brief Empty constructor for empty index
   *
   * 0 is considered the null/empty index
   */
  base_index() : i( 0u ) {}

  static base_index from_index( unsigned i ) { return base_index( i ); }

  /**
   * @brief Returns integer value
   */
  unsigned index() const { return i; }

  /**
   * @brief Create empty index
   */
  static const base_index null() { return base_index(); }

  bool operator==( const base_index& other ) const { return i == other.i; }
  bool operator!=( const base_index& other ) const { return i != other.i; }
  bool operator>( const base_index& other ) const { return i > other.i; }
  bool operator<( const base_index& other ) const { return i < other.i; }

  /**
   * @brief Checks for nullness/emptiness
   */
  operator bool() const { return i != 0u; }

protected:
  explicit base_index( unsigned i ) : i( i ) {}

private:
  unsigned i;
};

template<typename T>
std::ostream& operator<<( std::ostream& os, const base_index<T>& index )
{
  return os << index.index();
}

/******************************************************************************
 * value_traits                                                               *
 ******************************************************************************/

template<typename T>
struct value_traits
{
  static const T null_value;
};

template<typename T>
const T value_traits<T>::null_value = T();

template<>
struct value_traits<unsigned>
{
  static const unsigned null_value = 0u;
};

/******************************************************************************
 * index_map                                                                  *
 ******************************************************************************/

template<typename IndexType, typename ValueType>
class index_map
{
public:
  static const ValueType null_value/* = value_traits<ValueType>::null_value*/;

  bool has_index( IndexType index ) const
  {
    return index.index() < values.size();
  }

  bool has( IndexType index ) const
  {
    return has_index(index) && values[index.index()] != null_value;
  }

  /**
   * @return true, if previous value was not null
   */
  bool insert( IndexType index, const ValueType& value )
  {
    reserve( index );
    const auto ret_val = has( index );
    values[index.index()] = value;
    return ret_val;
  }

  /**
   * @return true, if previous value was not null
   */
  bool remove( IndexType index )
  {
    if( ! has_index( index ) )
    {
      return false;
    }

    return insert( index, null_value );
  }

  void clear()
  {
    values.clear();
  }

  ValueType& operator[]( IndexType index )
  {
    reserve( index );
    return values[index.index()];
  }

  const ValueType& operator[]( IndexType index ) const
  {
    assert( has_index(index) );
    return values[index.index()];
  }

  friend std::ostream& operator<<( std::ostream& os, const index_map<IndexType, ValueType>& map )
  {
    for ( auto i = 0u; i < map.values.size(); ++i )
    {
      if ( map.values[i] != index_map<IndexType, ValueType>::null_value )
      {
        os << i << ": " << map.values[i] << std::endl;
      }
    }

    return os;
  }

  void reserve( IndexType index )
  {
    if ( ! has_index(index) )
    {
      values.resize( index.index() + 1u, null_value );
    }
  }

private:
  std::vector<ValueType> values;
};

template<typename IndexType, typename ValueType>
const ValueType index_map<IndexType, ValueType>::null_value = value_traits<ValueType>::null_value;

/******************************************************************************
 * index_set                                                                  *
 ******************************************************************************/

template<typename IndexType>
class index_set
{
public:
  using const_iterator = typename std::vector<IndexType>::const_iterator;

  bool insert( IndexType index )
  {
    if ( positions.has( index ) )
    {
      return true;
    }

    values.push_back( index );
    positions.insert( index, values.size() );

    return false;
  }

  bool remove( IndexType index )
  {
    if ( !positions.has( index ) )
    {
      return false;
    }

    const auto pos = positions[index];
    positions[values.back()] = pos;
    values[pos - 1] = values.back();
    positions.remove( index );
    values.pop_back();

    return true;
  }

  void clear()
  {
    values.clear();
    positions.clear();
  }

  bool has( IndexType index ) const
  {
    return positions.has( index );
  }

  unsigned size() const
  {
    return values.size();
  }

  bool empty() const
  {
    return values.empty();
  }

  IndexType front() const
  {
    assert( !empty() );
    return values.front();
  }

  IndexType back() const
  {
    assert( !empty() );
    return values.back();
  }

  /* iterators */
  const_iterator begin() const { return values.begin(); }
  const_iterator end()   const { return values.end();   }

  bool operator==( const index_set& other ) const
  {
    if( size() != other.size() )
    {
      return false;
    }

    for( auto index : other )
    {
      if( !has(index) )
      {
        return false;
      }
    }

    return true;
  }

  bool operator!=(const index_set& other) const
  {
    return ! operator==(other);
  }

private:
  std::vector<IndexType>         values;
  index_map<IndexType, unsigned> positions;
};

template<typename T>
std::ostream& operator<<( std::ostream& os, const index_set<T>& set )
{
  os << "[";
  if ( !set.empty() )
  {
    std::copy( set.begin(), set.end() - 1, std::ostream_iterator<T>( os, ", " ) );
    std::copy( set.end() - 1, set.end(), std::ostream_iterator<T>( os, "" ) );
  }
  os << "]";
  return os;
}

/*
 * @class index_backtracking_set
 *
 * A data structure for maintaining a set of indices that allows O(1) removal and O(1) backtracking to the previous state.
 * It supports the following operations
 *
 * insert() : O(1) - insert an element at the end of the set.
 * remove() : O(1) - remove an element from the set
 * has() : O(1) - check if an element is in the set
 * save_state() : O(1) - save the current state of the set
 * restore_state() : O(1) - restore the state to its previously stored state (undoing any removals performed)
 *
 * insertions are only allowed before any removal, or after removals have been undone.
 */

template<typename IndexType>
class index_backtracking_set
{
public:

  using const_iterator = typename std::vector<IndexType>::const_iterator;

  index_backtracking_set() :
    real_size(0)
  {
  }

  void save_state()
  {
    if( !state_stack.empty() && real_size==state_stack.top().size )
    {
      ++state_stack.top().n;
    }
    else
    {
      state_stack.push(state{real_size, 1});
    }
  }

  void restore_state()
  {
    assert( !state_stack.empty() );

    real_size = state_stack.top().size;
    --state_stack.top().n;

    if( state_stack.top().n == 0)
    {
      state_stack.pop();
    }
  }

  bool has( IndexType index ) const
  {
    return positions.has( index ) && positions[index] <= size();
  }

  bool insert( IndexType index )
  {
    assert( state_stack.empty() );
    assert( size() == values.size() );

    if ( has(index) )
    {
      return true;
    }

    values.push_back( index );
    positions.insert( index, values.size() );

    ++real_size;

    return false;
  }

  bool remove( IndexType index )
  {
    assert ( has( index ) );

    unsigned pos = positions[index]-1;

    std::swap(positions[index], positions[back()]);
    std::swap(values[pos], values[size()-1]);

    --real_size;

    return true;
  }

  unsigned size() const
  {
    return real_size;
  }

  bool empty() const
  {
    return size()==0;
  }

  IndexType front() const
  {
    assert( !empty() );
    return values.front();
  }

  IndexType back() const
  {
    assert( !empty() );
    return values[size()-1];
  }

  /* iterators */
  const_iterator begin() const { return values.begin(); }
  const_iterator end()   const { return values.begin() + size(); }

  bool operator==( const index_backtracking_set& other ) const
  {
    if( size() != other.size() )
    {
      return false;
    }

    for( auto index : other )
    {
      if( !has(index) )
      {
        return false;
      }
    }

    return true;
  }

  bool operator!=(const index_backtracking_set& other) const
  {
    return ! operator==(other);
  }

private:

  std::vector<IndexType>         values;
  index_map<IndexType, unsigned> positions;

  struct state
  {
    unsigned size;
    unsigned n;
  };

  std::stack<state> state_stack;
  unsigned real_size;
};

template<typename T>
std::ostream& operator<<( std::ostream& os, const index_backtracking_set<T>& set )
{
  os << "[";
  if ( !set.empty() )
  {
    std::copy( set.begin(), set.end() - 1, std::ostream_iterator<T>( os, ", " ) );
    std::copy( set.end() - 1, set.end(), std::ostream_iterator<T>( os, "" ) );
  }
  os << "]";
  return os;
}
}

/*
 * @class index_bitset
 *
 * A data structure for maintaining a set of indices, that only allows efficient insertion and membership testing, but not efficient iteration.
 * It uses a bitset.
 *
 * insert() : O(1) - insert an element at the end of the set, return if the element was already present.
 * remove() : O(1) - remove an element from the set
 * has() : O(1) - check if an element is in the set
 *
 */

template<typename IndexType>
class index_bitset
{
public:

  bool has( IndexType index ) const
  {
    return has_index( index ) && bitset[ index.index() ];
  }

  bool insert( IndexType index )
  {
    reserve( index );

    bool prev = bitset.test( index.index() );

    bitset.set( index.index() );

    return prev;
  }

  void remove( IndexType index )
  {
    if( has_index(index) )
    {
      bitset.reset( index.index() );
    }
  }

  void reserve( IndexType index)
  {
    if( ! has_index(index) )
    {
      bitset.resize(index.index()+1, false);
    }
  }

  void clear()
  {
    bitset.clear();
  }

  unsigned capacity() const
  {
    return bitset.size();
  }

private:

  bool has_index( IndexType index ) const
  {
    assert( index );

    return index.index() < bitset.size();
  }

  boost::dynamic_bitset<> bitset;
};


/******************************************************************************
 * index_set_monotone                                                                  *
 ******************************************************************************/

template<typename IndexType>
class index_set_monotone
{
public:

  using const_iterator = typename std::vector<IndexType>::const_iterator;

  bool insert( IndexType index )
  {
    if ( present.has( index ) )
    {
      return true;
    }

    values.push_back( index );
    present.insert( index );

    return false;
  }

  void clear()
  {
    if ( values.size() < present.capacity()/32 )
    {
      for( const auto& i : values )
      {
        present.remove(i);
      }

      values.clear();
    }
    else
    {
      values.clear();
      present.clear();
    }
  }

  bool has( IndexType index ) const
  {
    return present.has( index );
  }

  unsigned size() const
  {
    return values.size();
  }

  bool empty() const
  {
    return values.empty();
  }

  IndexType front() const
  {
    assert( !empty() );
    return values.front();
  }

  IndexType back() const
  {
    assert( !empty() );
    return values.back();
  }

  /* iterators */
  const_iterator begin() const { return values.begin(); }
  const_iterator end()   const { return values.end();   }

  bool operator==( const index_set_monotone& other ) const
  {
    if( size() != other.size() )
    {
      return false;
    }

    for( auto index : other )
    {
      if( !has(index) )
      {
        return false;
      }
    }

    return true;
  }

  bool operator!=(const index_set_monotone& other) const
  {
    return ! operator==(other);
  }

private:

  std::vector<IndexType>  values;
  index_bitset<IndexType> present;
};

template<typename T>
std::ostream& operator<<( std::ostream& os, const index_set_monotone<T>& set )
{
  os << "[";
  if ( !set.empty() )
  {
    std::copy( set.begin(), set.end() - 1, std::ostream_iterator<T>( os, ", " ) );
    std::copy( set.end() - 1, set.end(), std::ostream_iterator<T>( os, "" ) );
  }
  os << "]";
  return os;
}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
