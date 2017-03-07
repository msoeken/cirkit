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

#include "compact_dsop.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

connected_cube_list::connected_cube_list() {}

connected_cube_list::connected_cube_list( const cube_vec_t& new_cubes )
{
  for ( const auto& c : new_cubes )
  {
    add( c );
  }
}

void connected_cube_list::add( const cube& c )
{
  /* check if cube already exists */
  auto it = _connected_cubes.find( c );
  if ( it != _connected_cubes.end() ) return;

  /* find matching cubes */
  cube_common_vec_t commons;
  int weight = 0;
  for ( const auto& other : _cubes )
  {
    int m = other.match_intersect( c );
    if ( m != -1 )
    {
      commons += std::make_pair( other, (unsigned)m );
      weight += c.dimension() - m - 1;
      _connected_cubes[other] += std::make_pair( c, (unsigned)m );
      cube_weights[other] += other.dimension() - m - 1;
    }
  }

  /* add cube */
  _cubes += c;
  _connected_cubes[c] = commons;
  cube_weights[c] = weight;
}

void connected_cube_list::remove( const cube& c )
{
  /* check if cube exists */
  auto it = _connected_cubes.find( c );
  if ( it == _connected_cubes.end() ) return;

  /* remove from others */
  for ( const auto& p : it->second )
  {
    auto it2 = boost::find( _connected_cubes[p.first], p );
    if ( it2 != _connected_cubes[p.first].end() )
    {
      cube_weights[p.first] -= p.first.dimension() - p.second - 1;
      _connected_cubes[p.first].erase( it2 );
    }
  }

  /* remove cube */
  _connected_cubes.erase( c );
  cube_weights.erase( c );
  _cubes.erase( boost::find( _cubes, c ) );
}

cube_vec_t connected_cube_list::remove_disjoint_cubes()
{
  cube_vec_t dis;

  /* find disjoint cubes */
  for ( const auto& c : _cubes )
  {
    if ( _connected_cubes[c].empty() )
    {
      dis += c;
      _connected_cubes.erase( c );
      cube_weights.erase( c );
    }
  }

  /* remove disjoint cubes */
  for ( const auto& d : dis )
  {
    _cubes.erase( boost::find( _cubes, d ) );
  }

  return dis;
}

cube_vec_t connected_cube_list::connected_cubes( const cube& c ) const
{
  cube_vec_t connected;

  for ( const auto& p : _connected_cubes.find( c )->second )
  {
    connected += p.first;
  }

  return connected;
}

void connected_cube_list::sort( const sort_cube_meta_func_t& sortfunc )
{
  boost::sort( _cubes, sortfunc( cube_weights ) );
}

cube_vec_t compute_dsop( cube_vec_t& c, const sort_cube_meta_func_t& sortfunc, const opt_cube_func_t& optfunc, bool verbose )
{
  cube_vec_t d;

  if ( verbose )
  {
    std::cout << "[i] initial cover:" << std::endl;
    common_pla_print( c );
  }

  while ( !c.empty() )
  {
    /* BUILD-SOP(C, P) */
    connected_cube_list p( common_pla_espresso( c ) );
    if ( verbose )
    {
      std::cout << "[i] after espresso:" << std::endl;
      common_pla_print( p.cubes() );
    }
    /* A = ..., D = D \cup A, P = P \ A, WEIGHT(P) */
    boost::push_back( d, p.remove_disjoint_cubes() );
    if ( verbose )
    {
      std::cout << "[i] current d:" << std::endl;
      common_pla_print( d );
    }
    /* SORT(P) */
    p.sort( sortfunc );
    if ( verbose )
    {
      std::cout << "[i] after sorting:" << std::endl;
      common_pla_print( p.cubes() );
    }
    /* B = {} */
    cube_vec_t b;

    while ( !p.empty() )
    {
      /* let p be the first element of P
         (note: the sort function is inversed so
         we can take the last element instead) */
      auto cube = p.cubes().back();
      auto connected = p.connected_cubes( cube );
      /* P = P \ {p} */
      p -= cube;
      /* D = D \cup {p} */
      d += cube;

      if ( verbose )
      {
        std::cout << "[i] picked p = " << cube.to_string() << std::endl;
      }

      for ( const auto& cubeq : connected )
      {
        auto it = boost::find( p.cubes(), cubeq );
        if ( it == p.cubes().end() )
        {
          continue;
        }

        if ( verbose )
        {
          std::cout << "[i] picked q = " << cubeq.to_string() << std::endl;
        }

        p -= cubeq;
        auto q = cubeq.disjoint_sharp( cube );
        // CHECK??
        if ( q.empty() )
        {
          continue;
        }
        if ( verbose )
        {
          std::cout << "[i] q (#) p =" << std::endl;
          common_pla_print( q );
        }

        optfunc( cubeq, q, p, b, sortfunc );
      }

      if ( verbose )
      {
        std::cout << "current b:" << std::endl;
        common_pla_print( b );
      }

      cube_vec_t b2;
      for ( const auto& cuber : b )
      {
        if ( cuber.match_intersect( cube ) == -1 )
        {
          b2 += cuber;
          continue;
        }
        auto q = cuber.disjoint_sharp( cube );
        boost::push_back( b2, q );
      }
      b = b2;
    }

    c = b;
  }

  return d;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

sort_cube_func_t sort_by_dimension_first( const cube_weight_map_t& cube_weights )
{
  return [&cube_weights]( const cube& cube1, const cube& cube2 ) {
    auto c1c = cube1.dimension();
    auto c2c = cube2.dimension();
    if ( c1c == c2c )
    {
      return cube_weights.at( cube1 ) > cube_weights.at( cube2 );
    }
    else
    {
      return c1c < c2c;
    }
  };
}

sort_cube_func_t sort_by_weight_first( const cube_weight_map_t& cube_weights )
{
  return [&cube_weights]( const cube& cube1, const cube& cube2 ) {
    auto c1w = cube_weights.at( cube1 );
    auto c2w = cube_weights.at( cube2 );
    if ( c1w == c2w )
    {
      return cube1.dimension() < cube2.dimension();
    }
    else
    {
      return c1w > c2w;
    }
  };
}

void opt_dsop_1( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc )
{
  boost::push_back( b, q );
}

void opt_dsop_2( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc )
{
  boost::push_back( b, q );
  p.sort( sortfunc );
}

void opt_dsop_3( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc )
{
  cube_vec_t remove;
  boost::push_back( b, q );
  for ( const auto& cube : p.cubes() )
  {
    if ( cube.match_intersect( cubeq ) != -1 )
    {
      b += cube;
      remove += cube;
    }
  }

  for ( const auto& cube : remove )
  {
    p -= cube;
  }
}

void opt_dsop_4( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc )
{
  if ( q.size() == 1u )
  {
    p += q.front();
  }
  else
  {
    boost::push_back( b, q );
  }
  p.sort( sortfunc );
}

void opt_dsop_5( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc )
{
  assert( !q.empty() );
  cube biggest = *boost::max_element( q, []( const cube& c1, const cube& c2 ) { return c1.dimension() < c2.dimension(); } );
  p += biggest;
  for ( const auto& c : q )
  {
    if ( c != biggest )
    {
      b += c;
    }
  }
  p.sort( sortfunc );
}

void compact_dsop( const std::string& destination, const std::string& filename,
                   const properties::ptr& settings,
                   const properties::ptr& statistics )
{
  /* Settings */
  const auto sortfunc = get( settings, "sortfunc", sort_cube_meta_func_t( sort_by_dimension_first ) );
  const auto optfunc  = get( settings, "optfunc",  opt_cube_func_t( opt_dsop_1 ) );
  const auto verbose  = get( settings, "verbose",  false );

  /* Run-time */
  properties_timer t( statistics );

  cube_vec_vec_t cs = common_pla_read( filename );
  cube_vec_vec_t ds;

  for ( auto& c : cs )
  {
    ds += compute_dsop( c, sortfunc, optfunc, verbose );
  }

  auto cpw_settings   = std::make_shared<properties>();
  auto cpw_statistics = std::make_shared<properties>();
  common_pla_write( ds, destination, cpw_settings, cpw_statistics );

  set( statistics, "cube_count", cpw_statistics->get<unsigned>( "cube_count" ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
