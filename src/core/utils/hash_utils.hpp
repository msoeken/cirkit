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
 * @file hash_utils.hpp
 *
 * @brief Hash utils
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef HASH_UTILS_HPP
#define HASH_UTILS_HPP

#include <functional>
#include <vector>

namespace cirkit
{

/* from Leo Goodstadt (http://stackoverflow.com/a/7115547) */
template<typename T>
struct hash
{
  std::size_t operator()( const T& t ) const
  {
    return std::hash<T>()( t );
  }
};

namespace
{

template<typename T>
inline void hash_combine( std::size_t& seed, const T& v )
{
  seed ^= hash<T>()( v ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

// Recursive template code derived from Matthieu M.
template<class Tuple, std::size_t Index = std::tuple_size<Tuple>::value - 1>
struct hash_value_impl
{
  static void apply( std::size_t& seed, const Tuple& tuple )
  {
    hash_value_impl<Tuple, Index - 1>::apply( seed, tuple );
    hash_combine( seed, std::get<Index>( tuple ) );
  }
};

template<class Tuple>
struct hash_value_impl<Tuple, 0>
{
  static void apply( std::size_t& seed, const Tuple& tuple )
  {
    hash_combine( seed, std::get<0>( tuple ) );
  }
};

}

template<typename ... T>
struct hash<std::tuple<T...>>
{
  std::size_t operator()( std::tuple<T...> const& t ) const
  {
    std::size_t seed = 0;
    hash_value_impl<std::tuple<T...>>::apply( seed, t );
    return seed;
  }
};

template<typename T1, typename T2>
struct hash<std::pair<T1, T2>>
{
  std::size_t operator()( std::pair<T1, T2> const& p ) const
  {
    std::size_t seed = 0;
    hash_combine( seed, p.first );
    hash_combine( seed, p.second );
    return seed;
  }
};

template<typename T>
struct hash<std::vector<T>>
{
  std::size_t operator()( std::vector<T> const& c ) const
  {
    std::size_t seed = 0;
    for ( const auto& v : c )
    {
      hash_combine( seed, v );
    }
    return seed;
  }
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
