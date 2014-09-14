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

/**
 * @file range_utils.hpp
 *
 * @brief Some helper functions for ranges
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef RANGE_UTILS_HPP
#define RANGE_UTILS_HPP

namespace cirkit
{

/* The code for indexer comes from: http://stackoverflow.com/questions/10962290/find-position-of-element-in-c11-range-based-for-loop */

template<typename T>
struct iterator_extractor
{
  typedef typename T::iterator type;
};

template<typename T>
struct iterator_extractor<T const>
{
  typedef typename T::const_iterator type;
};

template<typename T>
class Indexer
{
public:
  class iterator
  {
    typedef typename iterator_extractor<T>::type inner_iterator;
    typedef typename std::iterator_traits<inner_iterator>::reference inner_reference;

  public:
    typedef std::pair<size_t, inner_reference> reference;
    iterator( inner_iterator it ) : _pos( 0 ), _it( it ) {}

    reference operator*() const
    {
      return reference( _pos, *_it );
    }

    iterator& operator++()
    {
      ++_pos;
      ++_it;
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp( *this );
      ++*this;
      return tmp;
    }

    bool operator==( iterator const& it ) const
    {
      return _it == it._it;
    }

    bool operator!=( iterator const& it ) const
    {
      return !( *this == it );
    }

  private:
    size_t _pos;
    inner_iterator _it;
  };

  Indexer( T& t ) : _container( t ) {}

  iterator begin() const
  {
    return iterator( _container.begin() );
  }

  iterator end() const
  {
    return iterator( _container.end() );
  }

private:
  T& _container;
};

template<typename T>
Indexer<T> index( T& t )
{
  return Indexer<T>( t );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
