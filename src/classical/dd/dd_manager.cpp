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

#include "dd_manager.hpp"

#include <cstring>
#include <iostream>
#include <vector>

#include <boost/format.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

hash_cache::hash_cache( size_type log_size )
  : data( 1 << log_size ),
    mask( ( 1 << log_size ) - 1u )
  , nhit( 0u )
  , nmiss( 0 ) {}

int hash_cache::lookup( unsigned arg0, unsigned arg1, unsigned arg2 )
{
  auto& ent = entry( arg0, arg1, arg2 );
  bool hit = std::get<0>( ent ) == arg0 && std::get<1>( ent ) == arg1 && std::get<2>( ent ) == arg2;

  if ( hit ) {
    ++nhit;
  } else {
    ++nmiss;
  }

  return hit ? std::get<3>( ent ) : -1;
}

int hash_cache::insert( unsigned arg0, unsigned arg1, unsigned arg2, int res )
{
  auto& ent = entry( arg0, arg1, arg2 );
  std::get<0>( ent ) = arg0;
  std::get<1>( ent ) = arg1;
  std::get<2>( ent ) = arg2;
  std::get<3>( ent ) = res;
  return res;
}

std::size_t hash_cache::cache_size() const
{
  return data.size();
}

std::size_t hash_cache::hit() const
{
  return nhit;
}

std::size_t hash_cache::miss() const
{
  return nmiss;
}

std::ostream& operator<<( std::ostream& os, const dd_node& z )
{
  return os << boost::format( "(%d, %d, %d)" ) % z.var % z.high % z.low;
}

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

dd_manager::dd_manager( unsigned nvars, unsigned log_max_objs, bool verbose )
  : nvars( nvars ), cache( log_max_objs ), verbose( verbose )
{
  assert( log_max_objs > 0u );

  const auto _nobjs = 1 << log_max_objs;
  nodes.resize( _nobjs, {-1u, -1u, -1u } );
  mask   = _nobjs - 1u;
  unique = new unsigned[_nobjs];
  nexts  = new unsigned[_nobjs];

  memset( unique, 0, sizeof( unsigned ) * _nobjs );
  memset( nexts,  0, sizeof( unsigned ) * _nobjs );

  /* terminals, value is determined by index */
  nodes[0] = {nvars, -1u, -1u};
  nodes[1] = {nvars, -1u, -1u};

  /* variable nodes */
  for ( auto i = 0u; i < nvars; ++i )
  {
    nodes[i + 2u] = {i, 1u, 0u};
  }

  nnodes = 2u + nvars;
}

dd_manager::~dd_manager()
{
  delete[] unique;
  delete[] nexts;
}

unsigned dd_manager::size() const
{
  return nnodes;
}

unsigned dd_manager::get_var( unsigned z ) const
{
  return nodes.at( z ).var;
}

unsigned dd_manager::get_high( unsigned z ) const
{
  return nodes.at( z ).high;
}

unsigned dd_manager::get_low( unsigned z ) const
{
  return nodes.at( z ).low;
}

void dd_manager::dump_stats(std::ostream &stream) const
{
  stream << boost::format ("-- Variables:   %9d\n") % nvars;
  stream << boost::format ("-- Nodes:       %9d\n") % nnodes;
  stream << boost::format ("-- Cache-size:  %9d\n") % cache.cache_size();
  stream << boost::format ("-- Cache-miss:  %9d\n") % cache.miss();
  stream << boost::format ("-- Cache-hit:   %9d\n") % cache.hit();
}

unsigned dd_manager::unique_lookup( unsigned var, unsigned high, unsigned low )
{
  /* variable node */
  if ( high == 1u && low == 0u )
  {
    return var + 2u;
  }

  const auto hash  = ( 12582917 * (int)var + 4256249 * (int)high + 741457 * (int)low );
  const auto index = hash & mask;
  auto*      q     = unique + index;

  while ( *q )
  {
    if ( nodes.at( *q ).var == var && nodes.at( *q ).high == high && nodes.at( *q ).low == low )
    {
      return *q;
    }
    q = nexts + *q;
  }

  if ( nnodes == nodes.size() )
  {
    std::cerr << "[e] dd capacity exceeded" << std::endl;
    assert( false );
  }

  *q = nnodes++;
  nodes[*q].var  = var;
  nodes[*q].high = high;
  nodes[*q].low  = low;

  if ( verbose )
  {
    // std::cout << boost::format( "[i] created entry (%d, %d, %d) at index %d" ) % var % high % low % *q << std::endl;
  }

  return *q;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
