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
 * @since  2.1
 */

#ifndef RANGE_UTILS_HPP
#define RANGE_UTILS_HPP

#include <functional>

#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

namespace cirkit
{

/* The code for indexer comes from: http://stackoverflow.com/questions/10962290/find-position-of-element-in-c11-range-based-for-loop */

template<typename T>
struct iterator_extractor
{
  using type = typename T::iterator;
};

template<typename T>
struct iterator_extractor<T const>
{
  using type = typename T::const_iterator;
};

template<typename T>
class Indexer
{
public:
  class iterator
  {
    using inner_iterator  = typename iterator_extractor<T>::type;
    using inner_reference = typename std::iterator_traits<inner_iterator>::reference;

  public:
    using reference = std::pair<size_t, inner_reference>;
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

template<typename C>
std::string any_join( const C& container, const std::string& delim )
{
  using boost::adaptors::transformed;
  return boost::join( container | transformed( []( const typename C::value_type& v ) { return boost::lexical_cast<std::string>( v ); } ), delim );
}

void ntimes( unsigned n, std::function<void()> f );

template<typename P>
std::function<bool(const P&)> first_matches( const typename P::first_type& v )
{
  return [&v]( const P& p ) { return p.first == v; };
}

template<typename T>
std::vector<T> generate_vector( unsigned size, const std::function<T()>& generator )
{
  std::vector<T> v( size );
  boost::generate( v, generator );
  return v;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
