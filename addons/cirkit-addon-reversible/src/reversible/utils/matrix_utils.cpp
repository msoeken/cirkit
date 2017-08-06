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

#include "matrix_utils.hpp"

#include <cmath>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <xtensor/xio.hpp>
#include <xtensor-blas/xlinalg.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/pauli_tags.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/simulation/simple_simulation.hpp>

namespace cirkit
{

using namespace std::complex_literals;

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

xt::xarray<complex_t> get_2by2_matrix( complex_t a, complex_t b, complex_t c, complex_t d )
{
  xt::xarray<complex_t> matrix(std::vector<size_t>{2, 2});
  matrix[{0,0}] = a;
  matrix[{0,1}] = b;
  matrix[{1,0}] = c;
  matrix[{1,1}] = d;
  return matrix;
}

xt::xarray<complex_t> kron( const std::vector<xt::xarray<complex_t>>& ms )
{
  assert( !ms.empty() );

  auto r = ms[0u];

  for ( auto i = 1u; i < ms.size(); ++i )
  {
    r = xt::linalg::kron( r, ms[i] );
  }

  return r;
}

xt::xarray<complex_t> identity( unsigned dimension )
{
  xt::xarray<complex_t> matrix(std::vector<size_t>{dimension, dimension});
  for ( auto i = 0u; i < dimension; ++i )
  {
    matrix[{i, i}] = 1.0;
  }
  return matrix;
}

xt::xarray<complex_t> identity_padding( const xt::xarray<complex_t>& matrix, unsigned from, unsigned to, unsigned lines )
{
  std::vector<xt::xarray<complex_t>> ms;

  if ( to + 1u < lines )
  {
    ms.push_back( identity( 1 << ( lines - to - 1u ) ) );
  }

  ms.push_back( matrix );

  if ( from > 0u )
  {
    ms.push_back( identity( 1 << from ) );
  }

  return kron( ms );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

xt::xarray<complex_t> matrix_from_clifford_t_circuit( const circuit& circ )
{
  xt::xarray<complex_t> matrix_X = get_2by2_matrix( 0.0, 1.0, 1.0, 0.0 );
  xt::xarray<complex_t> matrix_H = 1.0 / sqrt( 2.0 ) * get_2by2_matrix( 1.0, 1.0, 1.0, -1.0 );
  xt::xarray<complex_t> matrix_T = get_2by2_matrix( 1.0, 0.0, 0.0, exp( 1i * M_PI / 4.0 ) );
  xt::xarray<complex_t> matrix_Tdag = get_2by2_matrix( 1.0, 0.0, 0.0, exp( -1i * M_PI / 4.0 ) );
  xt::xarray<complex_t> matrix_zc = get_2by2_matrix( 1.0, 0.0, 0.0, 0.0 );
  xt::xarray<complex_t> matrix_oc = get_2by2_matrix( 0.0, 0.0, 0.0, 1.0 );

  const auto n = circ.lines();

  std::vector<xt::xarray<complex_t>> gates;

  for ( const auto& g : circ )
  {
    const auto target = g.targets().front();

    if ( is_toffoli( g ) && g.controls().size() == 1u && g.controls().front().polarity() )
    {
      const auto control = g.controls().front().line();

      if ( control < target )
      {
        const auto act = target - control;
        const auto gate = xt::linalg::kron( identity( 1 << act ), matrix_zc ) + kron( {matrix_X, identity( 1 << ( act - 1 ) ), matrix_oc} );
        gates.push_back( identity_padding( gate, control, target, n ) );
      }
      else
      {
        const auto act = control - target;
        const auto gate = xt::linalg::kron( matrix_zc, identity( 1 << act ) ) + kron( {matrix_oc, identity( 1 << ( act - 1 ) ), matrix_X} );
        gates.push_back( identity_padding( gate, target, control, n ) );
      }
    }
    else if ( is_hadamard( g ) )
    {
      gates.push_back( identity_padding( matrix_H, target, target, n ) );
    }
    else if ( is_pauli( g ) )
    {
      const auto pauli = boost::any_cast<pauli_tag>( g.type() );
      switch ( pauli.axis )
      {
      case pauli_axis::Z:
        if ( pauli.root == 4u )
        {
          gates.push_back( identity_padding( pauli.adjoint ? matrix_Tdag : matrix_T, target, target, n ) );
        }
        else
        {
          std::cout << "[w] unsupported gate" << std::endl;
        }
        break;
      default:
        std::cout << "[w] unsupported gate" << std::endl;
        break;
      }
    }
    else
    {
      std::cout << "[w] unsupported gate" << std::endl;
    }
  }

  if ( gates.empty() )
  {
    return identity( 1 << n );
  }
  else
  {
    auto r = gates.front();
    for ( auto i = 1u; i < gates.size(); ++i )
    {
      r = xt::linalg::dot( r, gates[i] );
    }

    return r;
  }
}

xt::xarray<complex_t> matrix_from_reversible_circuit( const circuit& circ )
{
  const size_t N = 1 << circ.lines();

  xt::xarray<complex_t> matrix(std::vector<size_t>{N, N});
  foreach_bitset( circ.lines(), [&circ, &matrix]( const boost::dynamic_bitset<>& input ) {
      boost::dynamic_bitset<> output;
      simple_simulation( output, circ, input );
      matrix[{input.to_ulong(), output.to_ulong()}] = 1.0;
    } );

  return matrix;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
