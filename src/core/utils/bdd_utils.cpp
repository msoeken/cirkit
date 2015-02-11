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

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:






