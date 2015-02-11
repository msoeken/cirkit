/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bdd_utils.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>

#include <cuddInt.h>

namespace cirkit
{

BDD make_cube( Cudd& manager, const std::vector<BDD>& vars )
{
  return boost::accumulate( vars, manager.bddOne(), []( const BDD& x1, const BDD& x2 ) { return x1 & x2; } );
}

bool is_selfdual( const Cudd& manager, const BDD& f )
{
  /* negative literals */
  std::vector<BDD> lits( manager.ReadSize() );
  for ( auto i = 0u; i < lits.size(); ++i )
  {
    lits[i] = !manager.bddVar( i );
  }

  return f.VectorCompose( lits ) == !f;
}

bool is_monotone( const Cudd& manager, const BDD& f )
{
  for ( auto i = 0u; i < manager.ReadSize(); ++i )
  {
    if ( !f.Increasing( i ) ) { return false; }
  }
  return true;
}

/*
 * Implementation according to
 * Knuth TAOCP Exercise 7.1.4-106
 * [T. Horiyama and T. Ibaraki, Artificial Intelligence 136 (2002), 189-213]
 */
bool is_horn( const Cudd& manager, const BDD& f, const BDD& g, const BDD& h )
{
  if ( f > g ) { return is_horn( manager, g, f, h ); }
  if ( f == manager.bddZero() || h == manager.bddOne() ) { return true; }
  if ( g == manager.bddOne() || h == manager.bddZero() ) { return false; }

  assert( f != manager.bddOne() );
  assert( g != manager.bddZero() );

  BDD fl( manager, cuddE( f.getRegularNode() ) );
  BDD fh( manager, cuddT( f.getRegularNode() ) );
  BDD gl( manager, cuddE( g.getRegularNode() ) );
  BDD gh( manager, cuddT( g.getRegularNode() ) );
  BDD hl( manager, cuddE( h.getRegularNode() ) );
  BDD hh( manager, cuddT( h.getRegularNode() ) );

  return is_horn( manager, fl, gl, hl ) && is_horn( manager, fl, gh, hl ) && is_horn( manager, fh, gl, hl ) && is_horn( manager, fh, gh, hh );
}

bool is_horn( const Cudd& manager, const BDD& f )
{
  return is_horn( manager, f, f, f );
}

#define EXTRA_BDD_COMPARE_TAG 0x96
#define EXTRA_BDD_COMPARE_EQ ((DdNode*)1)
#define EXTRA_BDD_COMPARE_IC ((DdNode*)2)
#define EXTRA_BDD_COMPARE_LT ((DdNode*)3)
#define EXTRA_BDD_COMPARE_GT ((DdNode*)4)

DdNode * Extra_bddCompare( DdManager * dd, DdNode * f, DdNode * g )
{
  DdNode * one = DD_ONE( dd );
  DdNode * zero = Cudd_Not( one );

  if ( f == g ) { return EXTRA_BDD_COMPARE_EQ; }
  if ( f == zero || g == one ) { return EXTRA_BDD_COMPARE_LT; }
  if ( f == one || g == zero ) { return EXTRA_BDD_COMPARE_GT; }

  DdNode * r = nullptr;
  /*if ( ( r = cuddConstantLookup( dd, EXTRA_BDD_COMPARE_TAG, f, g, nullptr ) ) )
  {
    return r;
  }*/

  auto fl = Cudd_NotCond( Cudd_E( Cudd_Regular( f ) ), Cudd_IsComplement( f ) );
  auto fh = Cudd_NotCond( Cudd_T( Cudd_Regular( f ) ), Cudd_IsComplement( f ) );
  auto gl = Cudd_NotCond( Cudd_E( Cudd_Regular( g ) ), Cudd_IsComplement( g ) );
  auto gh = Cudd_NotCond( Cudd_T( Cudd_Regular( g ) ), Cudd_IsComplement( g ) );

  auto rl = Extra_bddCompare( dd, fl, gl );
  if ( rl == EXTRA_BDD_COMPARE_IC ) { r = EXTRA_BDD_COMPARE_IC; }
  else
  {
    auto rh = Extra_bddCompare( dd, fh, gh );
    if ( rh == EXTRA_BDD_COMPARE_IC ) { r = EXTRA_BDD_COMPARE_IC; }
    else if ( rl == EXTRA_BDD_COMPARE_EQ ) { r = rh; }
    else if ( rh == EXTRA_BDD_COMPARE_EQ ) { r = rl; }
    else if ( rl == rh ) { r = rl; }
    else { r = EXTRA_BDD_COMPARE_IC; }
  }

  //cuddCacheInsert( dd, EXTRA_BDD_COMPARE_TAG, f, g, nullptr, r );
  return r;
}

bool Extra_bddUnate( DdManager * dd, DdNode * f, std::vector<int>& ps )
{
  /* Initialize */
  ps.resize( Cudd_ReadSize( dd ) );
  boost::fill( ps, 0 );

  if ( Cudd_IsConstant( f ) ) return true;

  auto fr = Cudd_Regular( f );

  auto fl = Cudd_NotCond( Cudd_E( fr ), Cudd_IsComplement( f ) );
  auto fh = Cudd_NotCond( Cudd_T( fr ), Cudd_IsComplement( f ) );

  if ( !Extra_bddUnate( dd, fl, ps ) || !Extra_bddUnate( dd, fh, ps ) ) { return false; }

  auto r = Extra_bddCompare( dd, fl, fh );
  if ( r == EXTRA_BDD_COMPARE_IC ) { return false; }

  if ( r == EXTRA_BDD_COMPARE_LT )
  {
    if ( ps[fr->index] < 0 ) { return false; }
    ps[fr->index] = 1;
    return true;
  }
  if ( r == EXTRA_BDD_COMPARE_GT )
  {
    if ( ps[fr->index] > 0 ) { return false; }
    ps[fr->index] = -1;
    return true;
  }

  /* this should not happen */
  assert( r == EXTRA_BDD_COMPARE_EQ );
  assert( false );
}

bool is_unate( const Cudd& manager, const BDD& f, std::vector<int>& ps )
{
  return Extra_bddUnate( manager.getManager(), f.getNode(), ps );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:










