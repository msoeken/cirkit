/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

/**
 * @file add_bdd.hpp
 *
 * @brief Convert BDD to CNF clauses
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ADD_BDD_HPP
#define ADD_BDD_HPP

#include <cstdio>
#include <map>

#include <cudd.h>
#include <cuddInt.h>
#include <cuddObj.hh>

#include <core/utils/bdd_utils.hpp>
#include <formal/sat/sat_solver.hpp>

namespace cirkit
{

template<class S>
int add_bdd_rec( S& solver, DdManager* mgr, DdNode* f, int n, int sid, int& free, std::map<DdNode*, int>& node_to_var )
{
  if ( f == DD_ONE( mgr ) )
  {
    return sid + n;
  }

  const auto it = node_to_var.find( f );
  if ( it != node_to_var.end() ) { return it->second; }

  auto* t = cuddT( f );
  auto* e = cuddE( f );

  auto tid = add_bdd_rec<S>( solver, mgr, Cudd_Regular( t ), n, sid, free, node_to_var );
  auto eid = add_bdd_rec<S>( solver, mgr, Cudd_Regular( e ), n, sid, free, node_to_var );

  if ( Cudd_IsComplement( t ) ) { tid = -tid; }
  if ( Cudd_IsComplement( e ) ) { eid = -eid; }

  const int xid = sid + f->index;
  const int fid = free++;

  if ( tid == sid + n )
  {
    add_clause( solver )( {-xid, fid} );
  }
  else if ( -tid == sid + n )
  {
    add_clause( solver )( {-xid, -fid} );
  }
  else
  {
    add_clause( solver )( {-xid, -tid, fid} );
    add_clause( solver )( {-xid, tid, -fid } );
  }

  if ( eid == sid + n )
  {
    add_clause( solver )( {xid, fid} );
  }
  else if ( -eid == sid + n )
  {
    add_clause( solver )( {xid, -fid} );
  }
  else
  {
    add_clause( solver )( {xid, -eid, fid} );
    add_clause( solver )( {xid, eid, -fid} );
  }

  node_to_var.insert( {f, fid} );

  return fid;

  // f = x ? t : e
  // xt -> f, x!t -> !f, !xe -> f, !x!e -> !f
  // (!x !t f) (!x t !f) (x !e f) (x e !f)
}

template<class S>
int add_bdd( S& solver, DdManager* mgr, const std::vector<DdNode*>& fs, int sid, std::map<DdNode*, int>& node_to_var )
{
  const auto n = Cudd_ReadSize( mgr );
  add_clause( solver )( {sid + n} ); /* constant one */

  int free = sid + n + 1;

  for ( auto* f : fs )
  {
    add_bdd_rec<S>( solver, mgr, Cudd_Regular( f ), n, sid, free, node_to_var );
  }

  return free;
}

template<class S>
int add_bdd( S& solver, const bdd_function_t& bdd, int sid, std::map<BDD, int>& node_to_var )
{
  std::map<DdNode*, int> node_to_var_native;

  for ( const auto& p : node_to_var )
  {
    node_to_var_native.insert( {p.first.getNode(), p.second} );
  }

  std::vector<DdNode*> fs;

  for ( const auto& f : bdd.second )
  {
    fs.push_back( f.getNode() );
  }

  const auto ret = add_bdd<S>( solver, bdd.first.getManager(), fs, sid, node_to_var_native );

  for ( const auto& p : node_to_var_native )
  {
    assert( !Cudd_IsComplement( p.first ) );
    node_to_var.insert( {BDD( bdd.first, p.first ), p.second} );
  }

  return ret;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
