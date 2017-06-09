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

#include "spectral_canonization.hpp"

#include <iostream>
#include <numeric>

#include <boost/format.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

tt make_sum( const boost::dynamic_bitset<>& mask )
{
  boost::dynamic_bitset<> sum( std::max<unsigned>( 1 << mask.size(), 64u ) );
  foreach_bit( mask, [&sum]( unsigned pos ) {
      sum ^= tt_nth_var( pos );
    } );
  if ( mask.size() < 6u )
  {
    tt_shrink( sum, mask.size() );
  }

  return sum;
}

std::vector<int> rademacher_walsh_spectrum( const tt& func )
{
  const auto n = tt_num_vars( func );
  std::vector<int> spectrum;

  foreach_bitset( n, [n, &func, &spectrum]( const boost::dynamic_bitset<>& bs ) {
      const auto row = make_sum( bs );
      spectrum.push_back( ( 1 << n ) - 2 * ( row ^ func ).count() );
    } );

  return spectrum;
}

std::vector<int> autocorrelation_spectrum( const tt& func )
{
  const auto n = tt_num_vars( func );
  std::vector<int> spectrum;

  foreach_bitset( n, [n, &func, &spectrum]( const boost::dynamic_bitset<>& bs ) {
      if ( bs.none() )
      {
        spectrum.push_back( 1 << n );
      }
      else
      {
        boost::dynamic_bitset<> fs( func.size() );
        foreach_bitset( n, [&bs, &fs, &func]( const boost::dynamic_bitset<>& bs2 ) {
            fs.set( bs2.to_ulong(), func.test( ( bs2 ^ bs ).to_ulong() ) );
          } );
        spectrum.push_back( ( 1 << n ) - 2 * ( fs ^ func ).count() );
      }
    } );
  return spectrum;
}

void print_spectrum( const std::vector<int>& spectrum, unsigned nvars )
{
  for ( auto i = 0u; i < spectrum.size(); ++i )
  {
    std::cout << boost::dynamic_bitset<>( nvars, i ) << boost::format( " %5d" ) % spectrum[i] << std::endl;
  }
}

void print_spectrum_ordered( const std::vector<int>& spectrum, unsigned nvars )
{
  if ( nvars == 4u )
  {
    for ( auto i : {0u,16u,1u,2u,4u,8u,16u,3u,5u,9u,6u,10u,12u,16u,7u,11u,13u,14u,16u,15u} )
    {
      if ( i == 16u )
      {
        std::cout << " |";
      }
      else
      {
        std::cout << " " << spectrum[i];
      }
    }
    std::cout << std::endl;
  }
  else
  {
    std::cout << any_join( spectrum, " " ) << std::endl;
  }
}

int compare_abs( int a, int b )
{
  if ( abs( a ) < abs( b ) )
  {
    return -1;
  }
  else if ( abs( a ) > abs( b ) )
  {
    return 1;
  }
  else if ( a < b )
  {
    return -1;
  }
  else if ( a > b )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/******************************************************************************
 * Spectral operations                                                        *
 ******************************************************************************/

void operation1( std::vector<int>& spectrum, tt& func, const std::vector<unsigned>& perm )
{
  std::vector<int> spectrum_p( spectrum.size() );
  tt func_p = func;

  for ( auto row = 0u; row < spectrum.size(); ++row )
  {
    boost::dynamic_bitset<> idx( perm.size(), row ), idx_p( perm.size() );
    for ( auto i = 0u; i < perm.size(); ++i )
    {
      idx_p[i] = idx[perm[i]];
    }

    spectrum_p[idx_p.to_ulong()] = spectrum[row];
    func_p.set( idx_p.to_ulong(), func.test( row ) );
  }

  spectrum = spectrum_p;
  func = func_p;
}

void operation2( std::vector<int>& spectrum, tt& func, unsigned var )
{
  func = tt_flip( func, var );

  for ( auto row = 1u; row < spectrum.size(); ++row )
  {
    if ( ( row >> var ) & 1 )
    {
      spectrum[row] = -spectrum[row];
    }
  }
}

void operation3( std::vector<int>& spectrum, tt& func )
{
  func.flip();

  std::transform( spectrum.begin(), spectrum.end(), spectrum.begin(), std::negate<int>() );
}

void operation4( std::vector<int>& spectrum, tt& func, unsigned var, unsigned diff )
{
  for ( auto row = 0u; row < spectrum.size(); ++row )
  {
    if ( ( row >> var ) & 1 )
    {
      auto row2 = row ^ diff;
      if ( row < row2 )
      {
        std::swap( spectrum[row], spectrum[row2] );
      }
    }
  }

  const auto nvars = tt_num_vars( func );
  boost::dynamic_bitset<> mask( nvars, diff );
  const auto varbs = onehot_bitset( nvars, var );
  const auto inc_mask = ~( varbs | mask );

  //std::cout << "var = " << var << ", mask = " << mask << ", diff = " << diff << std::endl;
  //std::cout << "func b " << func << " " << func[15] << " " << func[14] << std::endl;

  auto start = varbs;
  do {
    //std::cout << start << std::endl;

    auto pattern = start;
    do {
      //std::cout << " " << pattern;
      if ( ( pattern & mask ).count() % 2 )
      {
        const auto r1 = pattern.to_ulong();
        const auto r2 = ( pattern ^ varbs ).to_ulong();

        const auto tmp = func.test( r1 );
        func.set( r1, func.test( r2 ) );
        func.set( r2, tmp );
        //std::cout << " *, swap with " << ( pattern ^ varbs ) << " (" << r1 << " with " << r2 << ")";
      }
      //std::cout << std::endl;
      inc_pos( pattern, mask );
    } while ( ( pattern & mask ).any() );

    inc_pos( start, inc_mask );
  } while ( ( start & inc_mask ).any() );

  //std::cout << "func a " << func << std::endl;
}

void operation5( std::vector<int>& spectrum, tt& func, unsigned row )
{
  const auto nvars = tt_num_vars( func );
  boost::dynamic_bitset<> mask( nvars, row );

  func ^= make_sum( mask );

  for ( auto i = 0u; i < spectrum.size(); ++i )
  {
    const auto j = i ^ row;
    if ( i < j )
    {
      std::swap( spectrum[i], spectrum[j] );
    }
  }
}

/******************************************************************************
 * Canonization operations                                                    *
 ******************************************************************************/

void maximize_zero_coefficient( std::vector<int>& spectrum, tt& func )
{
  const auto max_coeff = std::max_element( spectrum.begin(), spectrum.end(), []( int a, int b ) { return compare_abs( a, b ) == -1; } );

  if ( *max_coeff < 0 )
  {
    operation3( spectrum, func );
  }

  const auto pos = std::distance( spectrum.begin(), max_coeff );

  if ( pos != 0 )
  {
    operation5( spectrum, func, pos );
  }
}

void minimize_order( std::vector<int>& spectrum, tt& func )
{
  const auto n = tt_num_vars( func );

  for ( auto var = 0u; var < n; ++var )
  {
    const auto row = 1u << var;
    /* sweep through the other ones */
    auto best_row = row;
    auto best_val = spectrum[row];
    for ( auto row2 = row + 1u; row2 < spectrum.size(); ++row2 )
    {
      if ( ( row & row2 ) != row ) continue;

      if ( compare_abs( spectrum[row2], best_val ) == 1 )
      {
        best_val = spectrum[row2];
        best_row = row2;
      }
    }

    /* update values */
    const auto diff = row ^ best_row;
    if ( diff )
    {
      operation4( spectrum, func, var, diff );
    }
  }
}

void sort_inputs( std::vector<int>& spectrum, tt& func )
{
  auto cmp = [&spectrum]( unsigned i1, unsigned i2 ) {
    switch ( compare_abs( spectrum[1 << i1], spectrum[1 << i2] ) )
    {
    case -1: return false;
    case 1: return true;
    default:
    return false;
    }
  };

  auto cmp4 = [&spectrum]( unsigned i1, unsigned i2 ) {
    switch ( compare_abs( spectrum[1 << i1], spectrum[1 << i2] ) )
    {
    case -1: return false;
    case 1: return true;
    default:
    {
      for ( auto row : {3, 5, 9, 6, 10, 12} )
      {
        if ( ( ( row >> i1 ) & 1 ) && !( ( row >> i2 ) & 1 ) )
        {
          auto row2 = ( row ^ ( 1 << i1 ) ) | ( 1 << i2 );
          switch ( compare_abs( spectrum[row], spectrum[row2] ) )
          {
          case -1: return true;
          case 1: return false;
          }
        }
        else if ( !( ( row >> i1 ) & 1 ) && ( ( row >> i2 ) & 1 ) )
        {
          auto row2 = ( row ^ ( 1 << i2 ) ) | ( 1 << i1 );
          switch ( compare_abs( spectrum[row], spectrum[row2] ) )
          {
          case -1: return true;
          case 1: return false;
          }
        }
      }
      return false;
    }
    }
  };

  const auto nvars = tt_num_vars( func );
  std::vector<unsigned> proj( nvars );
  std::iota( proj.begin(), proj.end(), 0u );

  if ( nvars <= 3 )
  {
    std::sort( proj.begin(), proj.end(), cmp );
  }
  else
  {
    std::sort( proj.begin(), proj.end(), cmp4 );
  }

  operation1( spectrum, func, proj );
}

void invert_inputs( std::vector<int>& spectrum, tt& func )
{
  const auto nvars = tt_num_vars( func );

  for ( auto i = 0u; i < nvars; ++i )
  {
    if ( spectrum[1 << i] < 0 )
    {
      operation2( spectrum, func, i );
    }
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tt spectral_canonization( const tt& func, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto verbose = get( settings, "verbose", false );
  const auto very_verbose = get( settings, "very_verbose", false );

  properties_timer t( statistics );

  const auto nvars = tt_num_vars( func );
  auto spectrum = rademacher_walsh_spectrum( func );
  set( statistics, "spectrum_init", spectrum );
  auto cfunc = func;

  if ( verbose )
  {
    std::cout << "AC " << any_join( autocorrelation_spectrum( func ), " " ) << std::endl;
    std::cout << "before" << std::endl;
    if ( very_verbose ) print_spectrum( spectrum, nvars );
    print_spectrum_ordered( spectrum, nvars );
  }

  maximize_zero_coefficient( spectrum, cfunc );

  if ( verbose )
  {
    std::cout << "after step 1" << std::endl;
    if ( very_verbose ) print_spectrum( spectrum, nvars );
    print_spectrum_ordered( spectrum, nvars );
    std::cout << cfunc << std::endl;
  }

  minimize_order( spectrum, cfunc );

  if ( verbose )
  {
    std::cout << "after step 2" << std::endl;
    if ( very_verbose ) print_spectrum( spectrum, nvars );
    print_spectrum_ordered( spectrum, nvars );
    std::cout << cfunc << std::endl;
  }

  invert_inputs( spectrum, cfunc );

  if ( verbose )
  {
    std::cout << "after step 3" << std::endl;
    if ( very_verbose ) print_spectrum( spectrum, nvars );
    print_spectrum_ordered( spectrum, nvars );
    std::cout << cfunc << std::endl;
  }

  sort_inputs( spectrum, cfunc );

  if ( verbose )
  {
    std::cout << "after step 4" << std::endl;
    if ( very_verbose ) print_spectrum( spectrum, nvars );
    print_spectrum_ordered( spectrum, nvars );
    std::cout << cfunc << std::endl;
  }

  set( statistics, "spectrum_final", spectrum );
  set( statistics, "class", get_spectral_class( func ) );

  // if ( !( ( spectrum == std::vector<int>( {16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ) ) ||
  //         ( spectrum == std::vector<int>( {14, 2, 2, -2, 2, -2, -2, 2, 2, -2, -2, 2, -2, 2, 2, -2} ) ) ||
  //         ( spectrum == std::vector<int>( {12, 4, 4, -4, 4, -4, -4, 4, 0, 0, 0, 0, 0, 0, 0, 0} ) ) ||
  //         ( spectrum == std::vector<int>( {10, 6, 6, -6, 2, -2, -2, 2, 2, -2, -2, 2, 2, -2, -2, 2} ) ) ||
  //         ( spectrum == std::vector<int>( {8, 8, 8, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ) ) ||
  //         ( spectrum == std::vector<int>( {8, 8, 4, -4, 4, -4, 0, 0, 4, -4, 0, 0, 0, 0, -4, 4} ) ) ||
  //         ( spectrum == std::vector<int>( {6, 6, 6, -2, 6, -2, -2, -2, 6, -2, -2, -2, -2, -2, -2, 6} ) ) ||
  //         ( spectrum == std::vector<int>( {4, 4, 4, 4, 4, 4, -4, -4, 4, -4, 4, -4, -4, 4, 4, -4} ) ) ) )
  // {
  //   std::cout << "[w] wrongly classified " << func << std::endl;
  //   print_spectrum_ordered( spectrum );
  //   print_spectrum( spectrum, nvars );
  // }

  return cfunc;
}

unsigned get_spectral_class( const tt& func )
{
  const auto nvars = tt_num_vars( func );

  assert( nvars >= 2u && nvars <= 5u );

  auto spectrum = rademacher_walsh_spectrum( func );
  std::transform( spectrum.begin(), spectrum.end(), spectrum.begin(), []( int i ) { return abs( i ); } );
  std::stable_sort( spectrum.begin(), spectrum.end(), std::not2( std::less<int>() ) );

  switch ( nvars )
  {
  case 2u:
    return spectrum.front() == 4 ? 0u : 1u;
  case 3u:
    switch ( spectrum.front() )
    {
    case 8: return 0u;
    case 6: return 1u;
    case 4: return 2u;
    } break;
  case 4u:
    switch ( spectrum.front() )
    {
    case 16: return 0u;
    case 14: return 1u;
    case 12: return 2u;
    case 10: return 3u;
    case 8: return spectrum[2u] == 8 ? 4u : 5u;
    case 6: return 6u;
    case 4: return 7u;
    } break;
  case 5u:
    switch ( spectrum.front() )
    {
    case 32: return 0u;
    case 30: return 1u;
    case 28: return 2u;
    case 26: return 3u;
    case 24: return spectrum[4u] == 8 ? 4u : 5u;
    case 22: return spectrum[2u] == 6 ? 6u : 7u;
    case 20:
      if ( spectrum[1u] == 12 && spectrum[2u] == 4 ) return 8u;
      else if ( spectrum[1u] == 8 ) return 9u;
      else if ( spectrum[1u] == 12 && spectrum[2u] == 8 ) return 10u;
      else if ( spectrum[1u] == 12 && spectrum[2u] == 12 ) return 11u;
      else assert( false ); break;
    case 18:
      if ( spectrum[1u] == 10 && spectrum[2u] == 6 ) return 12u;
      else if ( spectrum[1u] == 14 && spectrum[2u] == 6 ) return 13u;
      else if ( spectrum[1u] == 10 && spectrum[2u] == 10 ) return 14u;
      else if ( spectrum[1u] == 14 && spectrum[2u] == 10 ) return 15u;
      else if ( spectrum[1u] == 14 && spectrum[2u] == 14 ) return 16u;
      else assert( false ); break;
    case 16:
      if ( spectrum[1u] == 16 && spectrum[2u] == 16 ) return 17u;
      else if ( spectrum[1u] == 8 && spectrum[9u] == 8 ) return 18u;
      else if ( spectrum[1u] == 16 && spectrum[2u] == 8 && spectrum[6] == 8) return 19u;
      else if ( spectrum[1u] == 8 && spectrum[9u] == 4 ) return 20u;
      else if ( spectrum[1u] == 12 && spectrum[2u] == 8 ) return 21u;
      else if ( spectrum[1u] == 16 && spectrum[2u] == 8 && spectrum[6] == 4 ) return 22u;
      else if ( spectrum[1u] == 12 && spectrum[2u] == 12 ) return 23u;
      else if ( spectrum[1u] == 16 && spectrum[2u] == 12 ) return 24u;
      else assert( false ); break;
    case 14:
      if ( spectrum[1u] == 10 && spectrum[2u] == 10 && spectrum[5u] == 6 )
      {
        auto ac = autocorrelation_spectrum( func );
        std::transform( ac.begin(), ac.end(), ac.begin(), []( int i ) { return abs( i ); } );
        std::stable_sort( ac.begin(), ac.end(), std::not2( std::less<int>() ) );
        return ac[1u] == 12 ? 25u : 26u;
      }
      else if ( spectrum[1u] == 14 && spectrum[2u] == 10 && spectrum[4u] == 6 ) return 27u;
      else if ( spectrum[1u] == 10 && spectrum[2u] == 10 && spectrum[5u] == 10 ) return 28u;
      else if ( spectrum[1u] == 14 && spectrum[2u] == 14 ) return 29u;
      else if ( spectrum[1u] == 14 && spectrum[2u] == 10 && spectrum[4u] == 10 ) return 30;
      else assert( false ); break;
    case 12:
      if ( spectrum[1u] == 8 ) return 33u;
      else if ( spectrum[1u] == 12 && spectrum[2u] == 8 )
      {
        auto ac = autocorrelation_spectrum( func );
        std::transform( ac.begin(), ac.end(), ac.begin(), []( int i ) { return abs( i ); } );
        std::stable_sort( ac.begin(), ac.end(), std::not2( std::less<int>() ) );
        return ac[1u] == 16 ? 34u : 35u;
      }
      else if ( spectrum[1u] == 12 && spectrum[2u] == 12 && spectrum[3u] == 8 ) return 36;
      else if ( spectrum[4u] == 4 )
      {
        auto ac = autocorrelation_spectrum( func );
        std::transform( ac.begin(), ac.end(), ac.begin(), []( int i ) { return abs( i ); } );
        std::stable_sort( ac.begin(), ac.end(), std::not2( std::less<int>() ) );
        return ac[4u] == 8 ? 31u : 32u;
      }
      else if ( spectrum[4u] == 8 )
      {
        auto ac = autocorrelation_spectrum( func );
        std::transform( ac.begin(), ac.end(), ac.begin(), []( int i ) { return abs( i ); } );
        std::stable_sort( ac.begin(), ac.end(), std::not2( std::less<int>() ) );
        return ac[1u] == 16 ? 37u : 38u;
      }
      else if ( spectrum[4u] == 12 ) return 39u;
      else assert( false ); break;
    case 10:
      if ( spectrum[4u] == 6 ) return 40u;
      else if ( spectrum[4u] == 10 )
      {
        auto ac = autocorrelation_spectrum( func );
        std::transform( ac.begin(), ac.end(), ac.begin(), []( int i ) { return abs( i ); } );
        std::stable_sort( ac.begin(), ac.end(), std::not2( std::less<int>() ) );
        return ac[1u] == 12 ? 41u : ( ac[1u] == 20 ? 42u : 43u );
      }
      else assert( false ); break;
    case 8:
      if ( spectrum[12u] == 8 )
      {
        auto ac = autocorrelation_spectrum( func );
        std::transform( ac.begin(), ac.end(), ac.begin(), []( int i ) { return abs( i ); } );
        std::stable_sort( ac.begin(), ac.end(), std::not2( std::less<int>() ) );
        return ac[1u] == 32 ? 44u : ( ac[1u] == 8 ? 45u : 46u );
      }
      else if ( spectrum[12u] == 4 ) return 47u;
      else assert( false ); break;
    } break;
  }

  return 0u;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
