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
 * @file matrix_utils.hpp
 *
 * @brief Matrix utilities
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef REVERSIBLE_MATRIX_UTILS_HPP
#define REVERSIBLE_MATRIX_UTILS_HPP

#include <complex>

#include <xtensor/xarray.hpp>

#include <reversible/circuit.hpp>

namespace cirkit
{

template<class T>
struct complex_round_fun
{
  using argument_type = T;
  using result_type = T;
  constexpr T operator()( const T& arg ) const
  {
    return T( round( arg.real() ), round( arg.imag() ) );
  }
};

template<class E>
inline auto complex_round( E&& e ) noexcept -> xt::detail::xfunction_type_t<complex_round_fun, E>
{
  return xt::detail::make_xfunction<complex_round_fun>( std::forward<E>( e ) );
}

template<class T>
struct complex_isclose_fun
{
  using result_type = bool;
  complex_isclose_fun( double rtol, double atol )
    : m_rtol( rtol ), m_atol( atol )
  {
  }

  bool operator()( const T& a, const T& b ) const
  {
    return std::abs( a - b ) <= ( m_atol + m_rtol * std::abs( b ) );
  }

private:
  double m_rtol;
  double m_atol;
};

template<class E1, class E2>
inline auto complex_isclose( E1&& e1, E2&& e2, double rtol = 1e-05, double atol = 1e-08 ) noexcept
{
  return xt::detail::make_xfunction<complex_isclose_fun>( std::make_tuple( rtol, atol ), std::forward<E1>( e1 ), std::forward<E2>( e2 ) );
}

template<class E1, class E2>
inline auto complex_allclose( E1&& e1, E2&& e2, double rtol = 1e-05, double atol = 1e-08 ) noexcept
{
  return xt::all( complex_isclose( std::forward<E1>( e1 ), std::forward<E2>( e2 ), rtol, atol ) );
}

using complex_t = std::complex<double>;

xt::xarray<complex_t> matrix_from_clifford_t_circuit( const circuit& circ );
xt::xarray<complex_t> matrix_from_reversible_circuit( const circuit& circ );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
