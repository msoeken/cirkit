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

#include "spectral_canonization2.hpp"

#include <cmath>
#include <iostream>

#include <boost/format.hpp>
#include <boost/pending/integer_log2.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * trans_t                                                                    *
 ******************************************************************************/

trans_t::trans_t()
  : data( 0 )
{
}

trans_t::trans_t( uint16_t kind, uint16_t var1, uint16_t var2 )
    : kind( kind ), var1( var1 ), var2( var2 )
{
}

/******************************************************************************
 * spectrum                                                                   *
 ******************************************************************************/

spectrum::spectrum( const tt& func )
  : _s( func.size(), 1 )
{
  /* translate 0 -> 1, 1 -> -1 */
  foreach_bit( func, [this]( int pos ) { _s[pos] = -1; } );
  /* apply fast Hadamard transform */
  fast_hadamard_transform();
}

spectrum::spectrum( const spectrum& other )
  : _s( other._s.begin(), other._s.end() )
{
}

spectrum& spectrum::operator=( const spectrum& other )
{
  if ( this != &other )
  {
    _s = other._s;
  }
  return *this;
}

tt spectrum::to_function()
{
  fast_hadamard_transform( true );

  tt func( _s.size() );
  for ( auto i = 0u; i < _s.size(); ++i )
  {
    if ( _s[i] == -1 )
    {
      func.set( i );
    }
  }
  return func;
}

void spectrum::fast_hadamard_transform( bool reverse )
{
  unsigned k{};
  int t{};

  for ( auto m = 1u; m < _s.size(); m <<= 1u )
  {
    for ( auto i = 0u; i < _s.size(); i += ( m << 1u ) )
    {
      for ( auto j = i, p = k = i + m; j < p; ++j, ++k )
      {
        t = _s[j];
        _s[j] += _s[k];
        _s[k] = t - _s[k];
      }
    }
  }

  if ( reverse )
  {
    for ( auto i = 0u; i < _s.size(); ++i )
    {
      _s[i] /= static_cast<int>( _s.size() );
    }
  }
}

void spectrum::apply( const trans_t& trans )
{
  switch ( trans.kind )
  {
  default: assert( false ); break;
  case 1: trans1( trans.var1, trans.var2 ); break;
  case 2: trans2( trans.var1 ); break;
  case 3: trans3(); break;
  case 4: trans4( trans.var1, trans.var2 ); break;
  case 5: trans5( trans.var1 ); break;
  }
}

trans_t spectrum::trans1( unsigned i, unsigned j )
{
  trans_t t( 1, i, j );
  i = 1 << i;
  j = 1 << j;
  for ( auto k = 0u; k < _s.size(); ++k )
  {
    if ( ( k & i ) > 0 && ( k & j ) == 0 )
    {
      std::swap( _s[k], _s[k - i + j] );
    }
  }

  return t;
}

trans_t spectrum::trans2( unsigned i )
{
  trans_t t( 2, i );
  i = 1 << i;
  for ( auto k = 0u; k < _s.size(); ++k )
  {
    if ( ( k & i ) > 0 )
    {
      _s[k] = -_s[k];
    }
  }

  return t;
}

trans_t spectrum::trans3()
{
  for ( auto k = 0u; k < _s.size(); ++k )
  {
    _s[k] = -_s[k];
  }

  return trans_t( 3 );
}

trans_t spectrum::trans4( unsigned i, unsigned j )
{
  trans_t t( 4, i, j );

  i = 1 << i;
  j = 1 << j;

  for ( auto k = 0u; k < _s.size(); ++k )
  {
    if ( ( k & i ) > 0 && ( k & j ) == 0 )
    {
      std::swap( _s[k], _s[k + j] );
    }
  }

  return t;
}

trans_t spectrum::trans5( unsigned i )
{
  trans_t t( 5, i );

  i = 1 << i;

  for ( auto k = 0u; k < _s.size(); ++k )
  {
    if ( ( k & i ) > 0 )
    {
      std::swap( _s[k], _s[k - i] );
    }
  }

  return t;
}

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

struct spectral_normalization_impl
{
public:
  spectral_normalization_impl( const tt& func, const spectral_normalization_params& params, spectral_normalization_stats& stats )
    : func( func ),
      params( params ),
      stats( stats ),
      num_vars( tt_num_vars( func ) ),
      spec( func ),
      best_spec( spec ),
      transforms( 100u ),
      transform_index( 0u )
  {
  }

  tt run()
  {
    reference_timer t( &stats.runtime );

    order = get_initial_coeffecient_order();

    if ( params.verbose )
    {
      print_spectrum( std::cout );
    }

    normalize();

    if ( params.verbose )
    {
      print_spectrum( std::cout );
    }

    stats.transforms = best_transforms;

    const auto res_tt = spec.to_function();

    if ( params.verify )
    {
      verify( res_tt );
    }

    return res_tt;
  }

private:
  std::vector<unsigned> get_initial_coeffecient_order()
  {
    std::vector<unsigned> map( spec.size(), 0u );
    auto p = std::begin( map ) + 1;

    for ( auto i = 1u; i <= num_vars; ++i )
    {
      for ( auto j = 1u; j < spec.size(); ++j )
      {
        if ( __builtin_popcount( j ) == static_cast<int>( i ) )
        {
          *p++ = j;
        }
      }
    }

    return map;
  }

  void closer( spectrum& lspec )
  {
    for ( auto i = 0u; i < lspec.size(); ++i )
    {
      const auto j = order[i];
      if ( lspec[j] == best_spec[j] ) continue;
      if ( abs( lspec[j] ) > abs( best_spec[j] ) ||
           ( abs( lspec[j] ) == abs( best_spec[j] ) && lspec[j] > best_spec[j] ) )
      {
        update_best( lspec );
        return;
      }

      if ( abs( lspec[j] ) < abs( best_spec[j] ) ||
           ( abs( lspec[j] ) == abs( best_spec[j] ) && lspec[j] < best_spec[j] ) )
      {
        return;
      }
    }
  }

  void normalize_rec( spectrum& lspec, unsigned v )
  {
    if ( v == num_vars ) /* leaf case */
    {
      /* invert function if necessary */
      if ( lspec[0u] < 0 )
      {
        insert( lspec.trans3() );
      }
      /* invert any variable as necessary */
      for ( auto i = 0u; i < num_vars; ++i )
      {
        if ( lspec[1 << i] < 0 )
        {
          insert( lspec.trans2( i ) );
        }
      }

      closer( lspec );
      return;
    }

    const auto locked = ( 1 << v ) - 1;
    auto max = -1;

    for ( auto i = 1u; i < lspec.size(); ++i )
    {
      if ( ( locked & i ) == i ) continue;
      max = std::max( max, abs( lspec[i] ) );
    }

    if ( max == 0 )
    {
      auto spec2 = lspec;
      normalize_rec( spec2, num_vars );
    }
    else
    {
      for ( auto i = 1u; i < lspec.size(); ++i )
      {
        auto j = order[i];
        if ( abs( lspec[j] ) != max ) continue;

        const auto save = transform_index;
        auto spec2 = lspec;
        auto k = 0u;
        for ( ; k < num_vars; ++k )
        {
          if ( ( 1 << k ) & locked ) continue;
          if ( ( 1 << k ) & j ) break;
        }
        if ( k == num_vars ) continue;
        j -= 1 << k;
        while ( j > 0 )
        {
          auto p = 0u;
          for ( ; ( ( 1 << p ) & j ) == 0; ++p );
          j -= 1 << p;
          insert( spec2.trans4( k, p ) );
        }
        if ( k != v )
        {
          insert( spec2.trans1( k, v ) );
        }
        normalize_rec( spec2, v + 1 );
        transform_index = save;
      }
    }
  }

  void normalize()
  {
    /* find maximum absolute element in spectrum (by order) */
    auto max = abs( spec[0u] );
    auto j = 0u;
    for ( auto i = 1u; i < spec.size(); ++i )
    {
      auto p = order[i];
      if ( abs( spec[p] ) > max )
      {
        max = abs( spec[p] );
        j = p;
      }
    }

    /* if max element is not the first element */
    if ( j > 0 )
    {
      auto k = 0u;
      for ( ; ( ( 1 << k ) & j ) == 0; ++k );
      j -= 1 << k;
      while ( j > 0 )
      {
        auto p = k + 1;
        for ( ; ( ( 1 << p ) & j ) == 0; ++p );
        j -= 1 << p;
        insert( spec.trans4( k, p ) );
      }
      insert( spec.trans5( k ) );
    }

    update_best( spec );
    normalize_rec( spec, 0u );
    spec = best_spec;
  }

  void insert( const trans_t& trans )
  {
    assert( transform_index < 100u );
    transforms[transform_index++] = trans;
  }

  void update_best( const spectrum& lbest )
  {
    best_spec = lbest;
    best_transforms.resize( transform_index );
    std::copy( transforms.begin(), transforms.begin() + transform_index, best_transforms.begin() );
  }

  void verify( const tt& res_tt )
  {
    if ( params.verbose )
    {
      std::cout << "[i] verify " << best_transforms.size() << " transformations" << std::endl;
    }

    spectrum lspec( func );
    std::for_each( best_transforms.begin(), best_transforms.end(),
                   [&lspec]( const trans_t& trans ) {
                     lspec.apply( trans );
                   } );
    assert( lspec.to_function() == res_tt );

    spectrum lspec2( res_tt );
    std::for_each( best_transforms.rbegin(), best_transforms.rend(),
                   [&lspec2]( const trans_t& trans ) {
                     lspec2.apply( trans );
                   } );
    assert( lspec2.to_function() == func );
  }

  std::ostream& print_spectrum( std::ostream& os )
  {
    os << "[i]";

    for ( auto i = 0u; i < spec.size(); ++i )
    {
      auto j = order[i];
      if ( j > 0 && __builtin_popcount( order[i - 1] ) < __builtin_popcount( j ) )
      {
        os << " |";
      }
      os << boost::format( " %3d" ) % spec[j];
    }
    return os << std::endl;
  }

private:
  const tt& func;
  const spectral_normalization_params& params;
  spectral_normalization_stats& stats;

  unsigned num_vars;
  spectrum spec;
  spectrum best_spec;

  std::vector<unsigned> order;
  std::vector<trans_t> transforms;
  std::vector<trans_t> best_transforms;
  unsigned transform_index = 0u;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

tt spectral_normalization( const tt& func, const spectral_normalization_params& params, spectral_normalization_stats& stats )
{
  spectral_normalization_impl impl( func, params, stats );
  return impl.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
