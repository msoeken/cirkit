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

#include "zdd.hpp"

#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/dd/zdd_to_sets.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

enum class zdd_operation { diff, _union, intersection, symmetric_difference, join, meet, delta, nonsub, nonsup, minhit };

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

zdd_manager::zdd_manager( unsigned nvars, unsigned log_max_objs, bool verbose )
  : dd_manager( nvars, log_max_objs, verbose ) {}

zdd_manager::~zdd_manager() {}

unsigned zdd_manager::zdd_diff( unsigned z1, unsigned z2 )
{
  /* terminating cases */
  if ( z1 == 0u ) { return 0u; }
  if ( z2 == 0u ) { return z1; }
  if ( z1 == z2 ) { return 0u; }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::diff );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );
  unsigned rlow, rhigh, idx;
  if ( node1.var < node2.var )
  {
    rlow = zdd_diff( node1.low, z2 );
    idx = unique_create( node1.var, node1.high, rlow );
  }
  else if ( node1.var > node2.var )
  {
    idx = zdd_diff( z1, node2.low );
  }
  else
  {
    rlow = zdd_diff( node1.low, node2.low );
    rhigh = zdd_diff( node1.high, node2.high );
    idx = unique_create( node1.var, rhigh, rlow );
  }
  return cache.insert( z1, z2, (unsigned)zdd_operation::diff, idx );
}

unsigned zdd_manager::zdd_union( unsigned z1, unsigned z2 )
{
  /* terminating cases */
  if ( z1 == 0u ) { return z2; }
  if ( z2 == 0u ) { return z1; }
  if ( z1 == z2 ) { return z1; }

  /* commutativity */
  if ( z1 > z2 ) { return zdd_union( z2, z1 ); }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::_union );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );
  unsigned rlow, rhigh;
  if ( node1.var < node2.var )
  {
    rlow = zdd_union( node1.low, z2 );
    rhigh = node1.high;
  }
  else if ( node1.var > node2.var )
  {
    rlow = zdd_union( z1, node2.low );
    rhigh = node2.high;
  }
  else
  {
    rlow = zdd_union( node1.low, node2.low );
    rhigh = zdd_union( node1.high, node2.high );
  }
  const auto idx = unique_create( std::min( node1.var, node2.var ), rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::_union, idx );
}

unsigned zdd_manager::zdd_intersection( unsigned z1, unsigned z2 )
{
  /* terminating cases */
  if ( z1 == 0u ) { return 0u; }
  if ( z2 == 0u ) { return 0u; }
  if ( z1 == z2 ) { return z1; }

  /* commutativity */
  if ( z1 > z2 ) { return zdd_intersection( z2, z1 ); }

  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );
  if ( node1.var < node2.var )
  {
    return zdd_intersection( node1.low, z2 );
  }
  if ( node1.var > node2.var )
  {
    return zdd_intersection( z1, node2.low );
  }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::intersection );
  if ( r >= 0 ) { return r; }

  auto rlow = zdd_intersection( node1.low, node2.low );
  auto rhigh = zdd_intersection( node1.high, node2.high );

  const auto idx = unique_create( node1.var, rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::intersection, idx );
}

unsigned zdd_manager::zdd_symmetric_difference( unsigned z1, unsigned z2 )
{
  /* terminating cases */
  if ( z1 == 0u ) { return z2; }
  if ( z2 == 0u ) { return z1; }
  if ( z1 == z2 ) { return 0u; }

  /* commutativity */
  if ( z1 > z2 ) { return zdd_symmetric_difference( z2, z1 ); }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::symmetric_difference );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );
  unsigned rlow, rhigh;
  if ( node1.var < node2.var )
  {
    rlow = zdd_symmetric_difference( node1.low, z2 );
    rhigh = node1.high;
  }
  else if ( node1.var > node2.var )
  {
    rlow = zdd_symmetric_difference( z1, node2.low );
    rhigh = node2.high;
  }
  else
  {
    rlow = zdd_symmetric_difference( node1.low, node2.low );
    rhigh = zdd_symmetric_difference( node1.high, node2.high );
  }
  const auto idx = unique_create( std::min( node1.var, node2.var ), rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::symmetric_difference, idx );
}

unsigned zdd_manager::zdd_join( unsigned z1, unsigned z2 )
{
  /* swapping */
  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );

  /* commutativity */
  if ( node1.var < node2.var || ( ( node1.var == node2.var ) && ( z1 > z2 ) ) ) { return zdd_join( z2, z1 ); }

  /* terminating cases */
  if ( z1 == 0u ) { return 0u; }
  if ( z1 == 1u ) { return z2; }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::join );
  if ( r >= 0 ) { return r; }

  unsigned rlow, rhigh;
  if ( node1.var > node2.var )
  {
    rlow = zdd_join( z1, node2.low );
    rhigh = zdd_join( z1, node2.high );
  }
  else
  {
    rlow = zdd_join( node1.low, node2.low );
    auto r1 = zdd_join( node1.low, node2.high );
    auto r2 = zdd_join( node1.high, node2.low );
    auto r3 = zdd_join( node1.high, node2.high );
    rhigh = zdd_union( zdd_union( r1, r2 ), r3 );
  }

  const auto idx = unique_create( node2.var, rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::join, idx );
}

unsigned zdd_manager::zdd_meet( unsigned z1, unsigned z2 )
{
  /* swapping */
  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );

  /* commutativity */
  if ( node1.var < node2.var || ( ( node1.var == node2.var ) && ( z1 > z2 ) ) ) { return zdd_join( z2, z1 ); }

  /* terminating cases */
  if ( z1 <= 1u ) { return z1; }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::meet );
  if ( r >= 0 ) { return r; }

  if ( node1.var > node2.var )
  {
    auto idx = zdd_meet( z1, zdd_union( node2.low, node2.high ) );
    return cache.insert( z1, z2, (unsigned)zdd_operation::join, idx );
  }
  else
  {
    auto rhigh = zdd_join( node1.high, node2.high );
    auto r1 = zdd_join( node1.low, node2.high );
    auto r2 = zdd_join( node1.high, node2.low );
    auto r3 = zdd_join( node1.low, node2.low );
    auto rlow = zdd_union( zdd_union( r1, r2 ), r3 );

    const auto idx = unique_create( node2.var, rhigh, rlow );
    return cache.insert( z1, z2, (unsigned)zdd_operation::join, idx );
  }
}

unsigned zdd_manager::zdd_delta( unsigned z1, unsigned z2 )
{
  /* swapping */
  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );

  /* commutativity */
  if ( node1.var < node2.var || ( ( node1.var == node2.var ) && ( z1 > z2 ) ) ) { return zdd_delta( z2, z1 ); }

  /* terminating cases */
  if ( z1 == 0u ) { return 0u; }
  if ( z1 == 1u ) { return z2; }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::delta );
  if ( r >= 0 ) { return r; }

  unsigned rlow, rhigh;
  if ( node1.var > node2.var )
  {
    rlow = zdd_delta( z1, node2.low );
    rhigh = zdd_delta( z1, node2.high );
  }
  else
  {
    rlow  = zdd_union( zdd_delta( node1.low, node2.low ), zdd_delta( node1.high, node2.high ) );
    rhigh = zdd_union( zdd_delta( node1.low, node2.high ), zdd_delta( node1.high, node2.low ) );
  }

  const auto idx = unique_create( node2.var, rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::delta, idx );
}

unsigned zdd_manager::zdd_nonsub( unsigned z1, unsigned z2 )
{
  /* terminating cases */
  if ( z1 == 0u ) { return 0u; }
  if ( z1 == 1u ) { return 0u; }
  if ( z2 == 0u ) { return z1; }
  if ( z1 == z2 ) { return 0u; }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::nonsub );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );

  unsigned rlow, rhigh;

  if ( node1.var > node2.var )
  {
    rlow = zdd_nonsub( node1.low, z2 );
    rhigh = node1.high;
  }
  else
  {
    auto r1 = zdd_nonsub( node1.low, node2.low );
    auto r2 = zdd_nonsub( node1.low, node2.high );
    rlow = zdd_intersection( r1, r2 );
    rhigh = zdd_nonsub( node1.high, node2.high );
  }

  const auto idx = unique_create( node1.var, rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::nonsub, idx );
}

unsigned zdd_manager::zdd_nonsup( unsigned z1, unsigned z2 )
{
  /* terminating cases */
  if ( z1 == 0u ) { return 0u; }
  if ( z2 == 1u ) { return 0u; }
  if ( z2 == 0u ) { return z1; }
  if ( z1 == z2 ) { return 0u; }

  const auto& node1 = nodes.at( z1 );
  const auto& node2 = nodes.at( z2 );

  if ( node1.var > node2.var )
  {
    return zdd_nonsup( z1, node2.low );
  }

  const auto r = cache.lookup( z1, z2, (unsigned)zdd_operation::nonsup );
  if ( r >= 0 ) { return r; }

  unsigned rlow, rhigh;

  if ( node1.var < node2.var )
  {
    rlow = zdd_nonsup( node1.low, z2 );
    rhigh = zdd_nonsup( node1.high, z2 );
  }
  else
  {
    rlow = zdd_nonsup( node1.high, node2.high );
    auto rtmp = zdd_nonsup( node1.high, node2.low );
    rhigh = zdd_intersection( rtmp, rlow );
    rlow = zdd_nonsup( node1.low, node2.low );
  }
  const auto idx = unique_create( node1.var, rhigh, rlow );
  return cache.insert( z1, z2, (unsigned)zdd_operation::nonsup, idx );
}

unsigned zdd_manager::zdd_minhit( unsigned z )
{
  /* terminating cases */
  if ( z == 0u ) { return 1u; }
  if ( z == 1u ) { return 0u; }

  const auto r = cache.lookup( z, z, (unsigned)zdd_operation::minhit );
  if ( r >= 0 ) { return r; }

  const auto& node = nodes.at( z );
  auto rtmp = zdd_union( node.low, node.high );
  auto rlow = zdd_minhit( rtmp );
  rtmp = zdd_minhit( node.low );
  auto rhigh = zdd_nonsup( rtmp, rlow );

  const auto idx = unique_create( node.var, rhigh, rlow );
  return cache.insert( z, z, (unsigned)zdd_operation::minhit, idx );
}

unsigned zdd_manager::unique_create( unsigned var, unsigned high, unsigned low )
{
  if ( verbose )
  {
    std::cout << boost::format( "[i] attempt to create (%d, %d, %d)" ) % var % high % low << std::endl;
  }
  assert( var < nvars );
  assert( var < nodes[high].var );
  assert( var < nodes[low].var );

  if ( high == 0u ) { return low; }

  return unique_lookup( var, high, low );
}

std::ostream& operator<<( std::ostream& os, const zdd_manager& mgr )
{
  for ( auto node : index( mgr.nodes ) )
  {
    os << node.index << ": " << node.value << std::endl;
  }
  return os;
}

zdd& zdd::operator=( const zdd& other )
{
  if ( this == &other ) { return *this; }
  assert( !manager || manager == other.manager );
  manager = other.manager;
  index   = other.index;
  return *this;
}

unsigned zdd::var() const
{
  return manager->get_var( index );
}

zdd zdd::high() const
{
  return zdd( manager, manager->get_high( index ) );
}

zdd zdd::low() const
{
  return zdd( manager, manager->get_low( index ) );
}

zdd zdd::operator-( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_diff( index, other.index ) );
}

zdd zdd::operator||( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_union( index, other.index ) );
}

zdd zdd::operator&&( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_intersection( index, other.index ) );
}

zdd zdd::operator^( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_symmetric_difference( index, other.index ) );
}

zdd zdd::operator+( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_join( index, other.index ) );
}

zdd zdd::operator*( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_meet( index, other.index ) );
}

zdd zdd::delta( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_delta( index, other.index ) );
}

zdd zdd::nonsub( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_nonsub( index, other.index ) );
}

zdd zdd::nonsup( const zdd& other ) const
{
  assert( manager == other.manager );
  return zdd( manager, manager->zdd_nonsup( index, other.index ) );
}

zdd zdd::minhit() const
{
  return zdd( manager, manager->zdd_minhit( index ) );
}

bool zdd::equals( const zdd& other ) const
{
  assert( manager == other.manager );
  return index == other.index;
}

std::ostream& operator <<( std::ostream& os, const cirkit::zdd& z )
{
  auto s = cirkit::zdd_to_sets( z );
  for ( const auto& e : s )
  {
    cirkit::print_as_set( os, e ) << std::endl;
  }
  return os;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
