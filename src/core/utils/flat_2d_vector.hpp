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
 * @file flat_2d_vector.hpp
 *
 * @brief Flat 2d-access vector
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef FLAT_2D_VECTOR_HPP
#define FLAT_2D_VECTOR_HPP

namespace cirkit
{

template<typename T>
class flat_2d_vector
{
public:
  using vec_t = std::vector<T>;
  using size_type = typename vec_t::size_type;
  using size_vec_t = std::vector<size_type>;

public:
  flat_2d_vector( unsigned num_elements )
    : indexes( num_elements ),
      sizes( num_elements )
  {
  }

  void append_empty( unsigned index )
  {
    indexes[index] = vec.size();
    sizes[index] = 0u;
  }

  void append_singleton( unsigned index, const T& e )
  {
    indexes[index] = vec.size();
    sizes[index] = 1u;
    vec.push_back( e );
  }

  void append_vector( unsigned index, const vec_t& v )
  {
    indexes[index] = vec.size();
    sizes[index] = v.size();
    std::copy( v.begin(), v.end(), std::back_inserter( vec ) );
  }

  void copy_to( unsigned index, vec_t& v, size_type offset = 0 ) const
  {
    const auto it = vec.begin() + indexes[index];
    std::copy( it + offset, it + sizes[index], std::back_inserter( v ) );
  }

  size_type size( unsigned index ) const
  {
    return sizes[index];
  }

  const T& at( unsigned index, unsigned element ) const
  {
    return vec[indexes[index] + element];
  }

private:
  vec_t vec;
  size_vec_t indexes;
  size_vec_t sizes;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
