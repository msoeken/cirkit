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

#include "bdd.hpp"

#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/counting_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/dd/count_solutions.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

enum class bdd_operation {
  _and, _or, _xor, _not, cof0, cof1, exists,
  constrain, restrict, round_down, round_up, round
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bdd_manager::bdd_manager( unsigned nvars, unsigned log_max_objs, bool verbose )
  : dd_manager( nvars, log_max_objs, verbose ) {}

bdd_manager::~bdd_manager() {}

unsigned bdd_manager::bdd_and( unsigned f, unsigned g )
{
  /* terminating cases */
  if ( f == 0u ) { return 0u; }
  if ( g == 0u ) { return 0u; }
  if ( f == 1u ) { return g; }
  if ( g == 1u ) { return f; }
  if ( f == g )  { return f; }

  /* commutativity */
  if ( f > g ) { return bdd_and( g, f ); }

  const auto r = cache.lookup( f, g, (unsigned)bdd_operation::_and );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( f );
  const auto& node2 = nodes.at( g );
  unsigned rlow, rhigh;
  if ( node1.var < node2.var )
  {
    rlow = bdd_and( node1.low, g );
    rhigh = bdd_and( node1.high, g );
  }
  else if ( node1.var > node2.var )
  {
    rlow = bdd_and( f, node2.low );
    rhigh = bdd_and( f, node2.high );
  }
  else
  {
    rlow = bdd_and( node1.low, node2.low );
    rhigh = bdd_and( node1.high, node2.high );
  }

  auto idx = unique_create( std::min( node1.var, node2.var ), rhigh, rlow );
  return cache.insert( f, g, (unsigned)bdd_operation::_and, idx );
}

unsigned bdd_manager::bdd_or( unsigned f, unsigned g )
{
  /* terminating cases */
  if ( f == 0u ) { return g; }
  if ( g == 0u ) { return f; }
  if ( f == 1u ) { return 1u; }
  if ( g == 1u ) { return 1u; }
  if ( f == g )  { return f; }

  /* commutativity */
  if ( f > g ) { return bdd_or( g, f ); }

  const auto r = cache.lookup( f, g, (unsigned)bdd_operation::_or );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( f );
  const auto& node2 = nodes.at( g );
  unsigned rlow, rhigh;
  if ( node1.var < node2.var )
  {
    rlow = bdd_or( node1.low, g );
    rhigh = bdd_or( node1.high, g );
  }
  else if ( node1.var > node2.var )
  {
    rlow = bdd_or( f, node2.low );
    rhigh = bdd_or( f, node2.high );
  }
  else
  {
    rlow = bdd_or( node1.low, node2.low );
    rhigh = bdd_or( node1.high, node2.high );
  }

  auto idx = unique_create( std::min( node1.var, node2.var ), rhigh, rlow );
  return cache.insert( f, g, (unsigned)bdd_operation::_or, idx );
}

unsigned bdd_manager::bdd_xor( unsigned f, unsigned g )
{
  /* terminating cases */
  if ( f == 0u ) { return g; }
  if ( g == 0u ) { return f; }
  if ( f == g )  { return 0u; }

  /* commutativity */
  if ( f > g ) { return bdd_xor( g, f ); }

  const auto r = cache.lookup( f, g, (unsigned)bdd_operation::_xor );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( f );
  const auto& node2 = nodes.at( g );
  unsigned rlow, rhigh;
  if ( node1.var < node2.var )
  {
    rlow = bdd_xor( node1.low, g );
    rhigh = bdd_xor( node1.high, g );
  }
  else if ( node1.var > node2.var )
  {
    rlow = bdd_xor( f, node2.low );
    rhigh = bdd_xor( f, node2.high );
  }
  else
  {
    rlow = bdd_xor( node1.low, node2.low );
    rhigh = bdd_xor( node1.high, node2.high );
  }

  auto idx = unique_create( std::min( node1.var, node2.var ), rhigh, rlow );
  return cache.insert( f, g, (unsigned)bdd_operation::_xor, idx );
}

unsigned bdd_manager::bdd_not( unsigned f )
{
  /* terminating cases */
  if ( f == 0u ) { return 1u; }
  if ( f == 1u ) { return 0u; }

  const auto r = cache.lookup( f, f, (unsigned)bdd_operation::_not );
  if ( r >= 0 ) { return r; }

  const auto& node = nodes.at( f );

  auto rlow = bdd_not( node.low );
  auto rhigh = bdd_not( node.high );

  auto idx = unique_create( node.var, rhigh, rlow );
  return cache.insert( f, f, (unsigned)bdd_operation::_not, idx );
}

unsigned bdd_manager::bdd_cof0( unsigned f, unsigned v )
{
  /* terminating cases */
  if ( f <= 1u ) { return f; }

  const auto& node = nodes.at( f );
  if ( node.var > v ) { return f; }

  const auto r = cache.lookup( f, v, (unsigned)bdd_operation::cof0 );
  if ( r >= 0 ) { return r; }

  unsigned idx;
  if ( node.var < v )
  {
    const auto rlow  = bdd_cof0( node.low, v );
    const auto rhigh = bdd_cof0( node.high, v );
    idx = unique_create( node.var, rhigh, rlow );
  }
  else
  {
    idx = node.low;
  }
  return cache.insert( f, v, (unsigned)bdd_operation::cof0, idx );
}

unsigned bdd_manager::bdd_cof1( unsigned f, unsigned v )
{
  /* terminating cases */
  if ( f <= 1u ) { return f; }

  const auto& node = nodes.at( f );
  if ( node.var > v ) { return f; }

  const auto r = cache.lookup( f, v, (unsigned)bdd_operation::cof1 );
  if ( r >= 0 ) { return r; }

  unsigned idx;
  if ( node.var < v )
  {
    const auto rlow  = bdd_cof1( node.low, v );
    const auto rhigh = bdd_cof1( node.high, v );
    idx = unique_create( node.var, rhigh, rlow );
  }
  else
  {
    idx = node.high;
  }
  return cache.insert( f, v, (unsigned)bdd_operation::cof1, idx );
}

unsigned bdd_manager::bdd_exists( unsigned f, unsigned g )
{
  /* terminating cases */
  if ( g == 1u || f <= 1u ) { return f; }

  const auto& node1 = nodes.at( f );
  const auto& node2 = nodes.at( g );

  if ( node1.var > node2.var )
  {
    return bdd_exists( f, node2.high );
  }

  const auto r = cache.lookup( f, g, (unsigned)bdd_operation::exists );
  if ( r >= 0 ) { return r; }

  unsigned idx;
  auto rlow  = bdd_exists( node1.low, node1.var == node2.var ? node2.high : g );

  if ( rlow == 1 && node1.var == node2.var )
  {
    idx = 1;
  }
  else
  {
    auto rhigh = bdd_exists( node1.high, node1.var == node2.var ? node2.high : g );

    if ( node1.var < node2.var )
    {
      idx = unique_create( node1.var, rhigh, rlow );
    }
    else
    {
      idx = bdd_or( rlow, rhigh );
    }
  }

  return cache.insert( f, g, (unsigned)bdd_operation::exists, idx );
}

unsigned bdd_manager::bdd_constrain( unsigned f, unsigned g )
{
  /* terminating cases */
  if ( g == 0u )            { return 0u; }
  if ( g == 1u || f <= 1u ) { return f;  }
  if ( f == g )             { return 1u; }

  const auto r = cache.lookup( f, g, (unsigned)bdd_operation::constrain );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( f );
  const auto& node2 = nodes.at( g );

  unsigned idx;

  do {
    unsigned v;

    if ( node1.var < node2.var )
    {
      v = node1.var;
    }
    else
    {
      v = node2.var;
      if ( node2.low == 0u )
      {
        idx = bdd_constrain( node1.var == v ? node1.high : f, node2.high );
        break;
      }
      else if ( node2.high == 0u )
      {
        idx = bdd_constrain( node1.var == v ? node1.low : f, node2.low );
        break;
      }
    }

    auto rlow = bdd_constrain( node1.var == v ? node1.low : f, node2.var == v ? node2.low : g );
    auto rhigh = bdd_constrain( node1.var == v ? node1.high : f, node2.var == v ? node2.high : g );
    idx = unique_create( v, rhigh, rlow );
  } while ( false );

  return cache.insert( f, g, (unsigned)bdd_operation::constrain, idx );
}

unsigned bdd_manager::bdd_restrict( unsigned f, unsigned g )
{
  /* terminating cases */
  if ( g == 0u )            { return 0u; }
  if ( g == 1u || f <= 1u ) { return f;  }
  if ( f == g )             { return 1u; }

  const auto r = cache.lookup( f, g, (unsigned)bdd_operation::restrict );
  if ( r >= 0 ) { return r; }

  const auto& node1 = nodes.at( f );
  const auto& node2 = nodes.at( g );

  unsigned idx;

  do {
    unsigned v;

    if ( node1.var < node2.var )
    {
      v = node1.var;
    }
    else
    {
      v = node2.var;
      if ( node2.low == 0u )
      {
        idx = bdd_restrict( node1.var == v ? node1.high : f, node2.high );
        break;
      }
      else if ( node2.high == 0u )
      {
        idx = bdd_restrict( node1.var == v ? node1.low : f, node2.low );
        break;
      }
    }

    /* special case in RESTRICT */
    if ( node1.low == node1.high )
    {
      idx = bdd_restrict( f, bdd_exists( g, v + 2u ) );
      break;
    }

    auto rlow = bdd_restrict( node1.var == v ? node1.low : f, node2.var == v ? node2.low : g );
    auto rhigh = bdd_restrict( node1.var == v ? node1.high : f, node2.var == v ? node2.high : g );
    idx = unique_create( v, rhigh, rlow );
  } while ( false );

  return cache.insert( f, g, (unsigned)bdd_operation::restrict, idx );
}

unsigned bdd_manager::bdd_round_to( unsigned f, unsigned level, unsigned cop, unsigned to )
{
  properties::ptr settings = std::make_shared<properties>();
  properties::ptr statistics = std::make_shared<properties>();

  count_solutions( bdd( this, f ), settings, statistics );

  return bdd_round_to( f, level, cop, to, statistics->get<std::map<unsigned, boost::multiprecision::uint256_t>>( "count_map" ) );
}

unsigned bdd_manager::bdd_round_to( unsigned f, unsigned level, unsigned cop, unsigned to, const std::map<unsigned, boost::multiprecision::uint256_t>& count_map )
{
  /* terminating cases */
  if ( f <= 1u ) { return f; }

  const auto r = cache.lookup( f, level, cop );
  if ( r >= 0 ) { return r; }

  const auto& node = nodes.at( f );

  auto idx = 0u;
  if ( node.var < level )
  {
    auto rlow = bdd_round_to( node.low, level, cop, to, count_map );
    auto rhigh = bdd_round_to( node.high, level, cop, to, count_map );

    idx = unique_create( node.var, rhigh, rlow );
  }
  else
  {
    auto cl = count_map.at( node.low );
    auto ch = count_map.at( node.high );

    if ( cl < ch )
    {
      auto rhigh = bdd_round_to( node.high, level, cop, to, count_map );
      idx = unique_create( node.var, rhigh, to );
    }
    else
    {
      auto rlow = bdd_round_to( node.low, level, cop, to, count_map );
      idx = unique_create( node.var, to, rlow );
    }
  }
  return cache.insert( f, level, cop, idx );
}

unsigned bdd_manager::bdd_round_down( unsigned f, unsigned level )
{
  return bdd_round_to( f, level, (unsigned)bdd_operation::round_down, 0u );
}

unsigned bdd_manager::bdd_round_up( unsigned f, unsigned level )
{
  return bdd_round_to( f, level, (unsigned)bdd_operation::round_up, 1u );
}

unsigned bdd_manager::bdd_round( unsigned f, unsigned level )
{
  /* terminating cases */
  if ( f <= 1u ) { return f; }

  const auto r = cache.lookup( f, level, (unsigned)bdd_operation::round );
  if ( r >= 0 ) { return r; }

  const auto& node = nodes.at( f );

  auto idx = 0u;
  if ( node.var < level )
  {
    auto rlow = bdd_round( node.low, level );
    auto rhigh = bdd_round( node.high, level );

    idx = unique_create( node.var, rhigh, rlow );
  }
  else
  {
    auto onset = count_solutions( bdd( this, f ) ) / ( 1ull << node.var );
    auto all   = 1ull << ( nvars - node.var );

    if ( ( onset << 1u ) > all ) /* if onset / all > .5 */
    {
      idx = 1u;
    }
  }
  return cache.insert( f, level, (unsigned)bdd_operation::round, idx );
}

bdd_manager_ptr bdd_manager::create( unsigned nvars, unsigned log_max_objs, bool verbose )
{
  return std::make_shared<bdd_manager>( nvars, log_max_objs, verbose );
}

unsigned bdd_manager::unique_create( unsigned var, unsigned high, unsigned low )
{
  if ( verbose )
  {
    //std::cout << boost::format( "[i] attempt to create (%d, %d, %d)" ) % var % high % low << std::endl;
  }
  assert( var < nvars );
  assert( var < nodes[high].var );
  assert( var < nodes[low].var );

  if ( high == low ) { return high; }

  return unique_lookup( var, high, low );
}

std::ostream& operator<<( std::ostream& os, const bdd_manager& mgr )
{
  for ( auto i : boost::counting_range( 0u, mgr.nvars + 2u ) )
  {
    os << i << ": " << mgr.nodes[i] << std::endl;
  }

  auto * q = mgr.unique;

  while ( q != mgr.unique + mgr.nodes.size() )
  {
    if ( *q )
    {
      os << *q << ": " << mgr.nodes[*q] << std::endl;
    }
    ++q;
  }

  return os;
}

bdd& bdd::operator=( const bdd& other )
{
  if ( this == &other ) { return *this; }
  assert( !manager || manager == other.manager );
  manager = other.manager;
  index   = other.index;
  return *this;
}

unsigned bdd::var() const
{
  return manager->get_var( index );
}

bdd bdd::high() const
{
  return bdd( manager, manager->get_high( index ) );
}

bdd bdd::low() const
{
  return bdd( manager, manager->get_low( index ) );
}

bdd bdd::operator&&( const bdd& other ) const
{
  assert( manager == other.manager );
  return bdd( manager, manager->bdd_and( index, other.index ) );
}

bdd bdd::operator||( const bdd& other ) const
{
  assert( manager == other.manager );
  return bdd( manager, manager->bdd_or( index, other.index ) );
}

bdd bdd::operator^( const bdd& other ) const
{
  assert( manager == other.manager );
  return bdd( manager, manager->bdd_xor( index, other.index ) );
}

bdd bdd::operator!() const
{
  return bdd( manager, manager->bdd_not( index ) );
}

bdd bdd::cof0( unsigned v ) const
{
  return bdd( manager, manager->bdd_cof0( index, v ) );
}

bdd bdd::cof1( unsigned v ) const
{
  return bdd( manager, manager->bdd_cof1( index, v ) );
}

bdd bdd::exists( const bdd& other ) const
{
  assert( manager == other.manager );
  return bdd( manager, manager->bdd_exists( index, other.index ) );
}

bdd bdd::constrain( const bdd& other ) const
{
  assert( manager == other.manager );
  return bdd( manager, manager->bdd_constrain( index, other.index ) );
}

bdd bdd::restrict( const bdd& other ) const
{
  assert( manager == other.manager );
  return bdd( manager, manager->bdd_restrict( index, other.index ) );
}

bdd bdd::round_down( unsigned level ) const
{
  return bdd( manager, manager->bdd_round_down( index, level ) );
}

bdd bdd::round_up( unsigned level ) const
{
  return bdd( manager, manager->bdd_round_up( index, level ) );
}

bdd bdd::round( unsigned level ) const
{
  return bdd( manager, manager->bdd_round( index, level ) );
}

bool bdd::equals( const bdd& other ) const
{
  assert( manager == other.manager );
  return index == other.index;
}

std::ostream &operator<<(std::ostream &stream, bdd::const_param_ref bdd)
{
  if ( bdd.is_bot() ) {
    return stream << "(zero)";
  }
  else if ( bdd.is_top() ) {
    return stream << "(one)";
  }
  else {
    return stream << boost::format ( "(var=%d, high=%d, low=%d)") % bdd.var() % bdd.high() % bdd.low();
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
