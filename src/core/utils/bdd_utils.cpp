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

#include "bdd_utils.hpp"

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include <boost/assign/std/vector.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/numeric.hpp>

#include <cudd.h>
#include <cuddInt.h>

#include <core/utils/range_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Functions                                                                  *
 ******************************************************************************/

BDD make_cube( Cudd& manager, const std::vector<BDD>& vars )
{
  return boost::accumulate( vars, manager.bddOne(), []( const BDD& x1, const BDD& x2 ) { return x1 & x2; } );
}

BDD make_cube( Cudd& manager, const std::string& cube )
{
  std::vector<BDD> vars;

  for ( auto i = 0u; i < cube.size(); ++i )
  {
    if ( cube[i] == '0' )
    {
      vars.push_back( ~manager.bddVar( i ) );
    }
    else if ( cube[i] == '1' )
    {
      vars.push_back( manager.bddVar( i ) );
    }
    else
    {
      assert( cube[i] == '-' );
    }
  }
  return make_cube( manager, vars );
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
  for ( auto i = 0; i < manager.ReadSize(); ++i )
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
  return false;
}

bool is_unate( const Cudd& manager, const BDD& f, std::vector<int>& ps )
{
  return Extra_bddUnate( manager.getManager(), f.getNode(), ps );
}

void collect_nodes( DdNode * f, std::unordered_set<DdNode*>& visited )
{
  assert( f );
  assert( !Cudd_IsComplement( f ) );

  /* node visited before? */
  if ( visited.find( f ) != visited.end() ) { return; }

  /* mark node as visited */
  visited.insert( f );

  /* terminate? */
  if cuddIsConstant( f ) { return; }

  /* recur */
  collect_nodes( cuddT( f ), visited );
  collect_nodes( Cudd_Regular( cuddE( f ) ), visited );
}

void collect_nodes_and_count( DdNode * f, std::unordered_map<DdNode*, unsigned>& visited )
{
  assert( f );
  assert( !Cudd_IsComplement( f ) );

  /* node visited before? */
  auto it = visited.find( f );
  if ( it != visited.end() )
  {
    /* visited once more */
    it->second++;
    return;
  }

  /* mark node as visited */
  visited.insert( {f, 1u} );

  /* terminate? */
  if cuddIsConstant( f ) { return; }

  /* recur */
  collect_nodes_and_count( cuddT( f ), visited );
  collect_nodes_and_count( Cudd_Regular( cuddE( f ) ), visited );
}

void collect_nodes_and_count_ignore_complemented( const DdNode * f, std::unordered_map<DdNode*, unsigned>& visited )
{
  assert( f );

  auto fr = Cudd_Regular( f );

  /* node visited before? */
  auto it = visited.find( fr );
  if ( it != visited.end() )
  {
    /* visited once more */
    if ( !Cudd_IsComplement( f ) )
    {
      it->second++;
    }
    return;
  }

  /* mark node as visited */
  visited.insert( {fr, (Cudd_IsComplement( f ) ? 0u : 1u)} );

  /* terminate? */
  if cuddIsConstant( fr ) { return; }

  /* recur */
  collect_nodes_and_count_ignore_complemented( cuddT( fr ), visited );
  collect_nodes_and_count_ignore_complemented( cuddE( fr ), visited );
}

std::vector<unsigned> level_sizes( DdManager * dd, const std::vector<DdNode*>& fs )
{
  using namespace std::placeholders;

  std::vector<unsigned> sizes( Cudd_ReadSize( dd ) );

  /* collect all nodes in the BDDs */
  std::unordered_set<DdNode*> visited;
  for ( const auto* f : fs )
  {
    collect_nodes( Cudd_Regular( f ), visited );
  }

  for ( const auto* node : visited )
  {
    if ( !cuddIsConstant( node ) )
    {
      sizes[node->index]++;
    }
  }

  return sizes;
}

std::vector<unsigned> level_sizes( const Cudd& manager, const std::vector<BDD>& fs )
{
  using namespace std::placeholders;
  using boost::adaptors::transformed;

  std::vector<DdNode*> fs_native( fs.size() );
  boost::copy( fs | transformed( std::bind( &BDD::getRegularNode, _1 ) ), fs_native.begin() );

  return level_sizes( manager.getManager(), fs_native );
}

unsigned maximum_fanout( DdManager* manager, const std::vector<DdNode*>& fs )
{
  using boost::adaptors::map_values;

  /* collect all nodes in the BDDs and count */
  std::unordered_map<DdNode*, unsigned> visited;
  for ( const auto* f : fs )
  {
    collect_nodes_and_count_ignore_complemented( f, visited );
  }

  /* erase constant nodes from visited list */
  visited.erase( DD_ONE( manager ) );
  visited.erase( Cudd_Not( DD_ONE( manager ) ) );

  if ( visited.empty() )
  {
    return 0u;
  }
  else
  {
    return *boost::max_element( visited | map_values );
  }
}

unsigned maximum_fanout( const Cudd& manager, const std::vector<BDD>& fs )
{
  using namespace std::placeholders;
  using boost::adaptors::transformed;

  std::vector<DdNode*> fs_native( fs.size() );
  boost::copy( fs | transformed( std::bind( &BDD::getNode, _1 ) ), fs_native.begin() );

  return maximum_fanout( manager.getManager(), fs_native );
}

void count_complement_edges_rec( DdManager* manager, DdNode* f, unsigned& count, std::unordered_set<DdNode*>& visited )
{
  if ( visited.find( f ) != visited.end() )
  {
    return;
  }

  if ( Cudd_IsComplement( f ) )
  {
    ++count;
  }

  if ( Cudd_IsConstant( f ) )
  {
    return;
  }

  const auto fr = Cudd_Regular( f );

  count_complement_edges_rec( manager, cuddT( fr ), count, visited );
  count_complement_edges_rec( manager, cuddE( fr ), count, visited );

  visited.insert( f );
}

unsigned count_complement_edges( DdManager* manager, const std::vector<DdNode*>& fs )
{
  std::unordered_set<DdNode*> visited;

  auto count = 0u;

  for ( const auto& f : fs )
  {
    count_complement_edges_rec( manager, f, count, visited );
  }

  return count;
}

unsigned count_complement_edges( const Cudd& manager, const std::vector<BDD>& fs )
{
  using namespace std::placeholders;
  using boost::adaptors::transformed;

  std::vector<DdNode*> fs_native( fs.size() );
  boost::copy( fs | transformed( std::bind( &BDD::getNode, _1 ) ), fs_native.begin() );

  return count_complement_edges( manager.getManager(), fs_native );
}

BDD make_eq_rec( const Cudd& manager, const std::vector<BDD>& vars, unsigned pos, int k, std::map<std::pair<unsigned, int>, BDD>& visited )
{
  /* terminal */
  if ( pos == vars.size() )
  {
    return ( k == 0 ) ? manager.bddOne() : manager.bddZero();
  }

  /* cannot be 1 anymore */
  if ( ( k < 0 ) || ( k > static_cast<int>( vars.size() - pos ) ) )
  {
    return manager.bddZero();
  }

  /* cached? */
  const auto it = visited.find( {pos, k} );
  if ( it != visited.end() ) { return it->second; }

  const auto f = ( vars[pos] & make_eq_rec( manager, vars, pos + 1, k - 1, visited ) ) |
                 ( ~vars[pos] & make_eq_rec( manager, vars, pos + 1, k, visited ) );

  visited.insert( {{pos, k}, f} );

  return f;
}

BDD make_eq( const Cudd& manager, const std::vector<BDD>& vars, unsigned k )
{
  std::map<std::pair<unsigned, int>, BDD> visited;
  return make_eq_rec( manager, vars, 0, (int)k, visited );
}

DdNode * bdd_copy_rec( DdManager* mgr_from, DdNode* from, DdManager* mgr_to, std::map<DdNode*, DdNode*>& visited, const std::vector<unsigned>& index_map )
{
  assert( !Cudd_IsComplement( from ) );

  if ( Cudd_IsConstant( from ) )
  {
    if ( from == DD_ONE( mgr_from ) )
    {
      return DD_ONE( mgr_to );
    }
    else
    {
      return Cudd_Not( DD_ONE( mgr_to ) );
    }
  }

  /* visited table */
  const auto it = visited.find( from );
  if ( it != visited.end() ) { return it->second; }

  auto * var = Cudd_bddIthVar( mgr_to, index_map.at( from->index ) );

  auto * high = Cudd_NotCond( bdd_copy_rec( mgr_from, Cudd_Regular( cuddT( from ) ), mgr_to, visited, index_map ), Cudd_IsComplement( cuddT( from ) ) );
  auto * low  = Cudd_NotCond( bdd_copy_rec( mgr_from, Cudd_Regular( cuddE( from ) ), mgr_to, visited, index_map ), Cudd_IsComplement( cuddE( from ) ) );

  /* create new node */
  auto* node = Cudd_bddIte( mgr_to, var, high, low );

  visited.insert( {from, node} );
  return node;
}

std::vector<DdNode*> bdd_copy( DdManager* mgr_from, const std::vector<DdNode*>& from, DdManager* mgr_to, std::vector<unsigned>& index_map )
{
  if ( index_map.empty() )
  {
    index_map.resize( Cudd_ReadSize( mgr_from ) );
    boost::iota( index_map, 0u );
  }

  std::map<DdNode*, DdNode*> visited;

  std::vector<DdNode*> ret;

  for ( auto* node : from )
  {
    ret += Cudd_NotCond( bdd_copy_rec( mgr_from, Cudd_Regular( node ), mgr_to, visited, index_map ), Cudd_IsComplement( node ) );
  }

  return ret;
}

std::vector<BDD> bdd_copy( const Cudd& mgr_from, const std::vector<BDD>& from, const Cudd& mgr_to, std::vector<unsigned>& index_map )
{
  using namespace std::placeholders;
  using boost::adaptors::transformed;

  std::vector<DdNode*> from_native( from.size() );
  boost::copy( from | transformed( std::bind( &BDD::getNode, _1 ) ), from_native.begin() );

  auto orig = bdd_copy( mgr_from.getManager(), from_native, mgr_to.getManager(), index_map );
  std::vector<BDD> ret;

  for ( auto* node : orig )
  {
    ret += BDD( mgr_to, node );
  }

  return ret;
}

bdd_function_t compute_characteristic( const bdd_function_t& bdd, bool inputs_first )
{
  const auto& mgr_from = bdd.first;
  const auto& from     = bdd.second;

  Cudd mgr;
  ntimes( mgr_from.ReadSize() + from.size(), [&]() { mgr.bddVar(); } );

  std::vector<unsigned> index_map( mgr_from.ReadSize() );
  boost::iota( index_map, inputs_first ? 0u : from.size() );

  const auto fs = bdd_copy( mgr_from, from, mgr, index_map );

  auto f = mgr.bddOne();

  const auto offset = inputs_first ? mgr_from.ReadSize() : 0u;
  for ( auto i = 0u; i < fs.size(); ++i )
  {
    f &= mgr.bddVar( offset + i ).Xnor( fs[i] );
  }

  return {mgr, {f}};
}

cube_vec_t bdd_to_cubes( DdManager* manager, DdNode* f )
{
  DdGen * gen;
  int * ddcube;
  CUDD_VALUE_TYPE value;

  cube_vec_t cubes;

  Cudd_ForeachCube( manager, f, gen, ddcube, value )
  {
    boost::dynamic_bitset<> bits( manager->size ), care( manager->size );
    for ( auto i = 0; i < manager->size; ++i )
    {
      switch ( ddcube[i] )
      {
      case 0:
        care[i] = true;
        break;
      case 1:
        bits[i] = true;
        care[i] = true;
        break;
      default:
        break;
      }
    }
    cubes.push_back( cube( bits, care ) );
  }

  return cubes;
}

cube_vec_t bdd_to_cubes( const Cudd& manager, BDD f )
{
  return bdd_to_cubes( manager.getManager(), f.getNode() );
}

/******************************************************************************
 * new BDD operations                                                         *
 ******************************************************************************/

DdNode* bdd_up_rec( DdManager* manager, DdNode* f )
{
  /* terminal case */
  if ( Cudd_IsConstant( f ) ) { return f; }

  const auto* F = Cudd_Regular( f );

  if ( F->ref != 1 )
  {
    auto* r = cuddCacheLookup1( manager, bdd_up_rec, f );
    if ( r )
    {
      return r;
    }
  }

  const auto top = manager->perm[F->index];

  auto* fl = Cudd_NotCond( cuddE( F ), Cudd_IsComplement( f ) );
  auto* fh = Cudd_NotCond( cuddT( F ), Cudd_IsComplement( f ) );
  auto* t1 = Cudd_bddOr( manager, fl, fh );
  if ( !t1 )
  {
    return nullptr;
  }
  cuddRef( t1 );

  auto* rh = bdd_up_rec( manager, t1 );
  if ( !rh )
  {
    Cudd_IterDerefBdd( manager, t1 );
    return nullptr;
  }
  cuddRef( rh );

  auto* rl = bdd_up_rec( manager, fl );
  if ( !rl )
  {
    Cudd_IterDerefBdd( manager, t1 );
    Cudd_IterDerefBdd( manager, rh );
  }
  cuddRef( rl );

  DdNode* r;
  if ( rl == rh )
  {
    r = rh;
  }
  else
  {
    if ( Cudd_IsComplement( rh ) )
    {
      r = cuddUniqueInter( manager, (int)top, Cudd_Not( rh ), Cudd_Not( rl ) );
      if ( !r )
      {
        Cudd_IterDerefBdd( manager, t1 );
        Cudd_IterDerefBdd( manager, rh );
        Cudd_IterDerefBdd( manager, rl );
        return nullptr;
      }
      r = Cudd_Not( r );
    }
    else
    {
      r = cuddUniqueInter( manager, (int)top, rh, rl );
      if ( !r )
      {
        Cudd_IterDerefBdd( manager, t1 );
        Cudd_IterDerefBdd( manager, rh );
        Cudd_IterDerefBdd( manager, rl );
        return nullptr;
      }
    }
  }
  cuddDeref( t1 );
  cuddDeref( rh );
  cuddDeref( rl );
  if ( F->ref != 1 )
  {
    cuddCacheInsert1( manager, bdd_up, f, r );
  }
  return r;
}

DdNode* bdd_up( DdManager *manager, DdNode *f )
{
  DdNode *res;

  do
  {
    manager->reordered = 0;
    res = bdd_up_rec( manager, f );
  }
  while ( manager->reordered == 1 );
  return res;
}

BDD bdd_up( Cudd& manager, const BDD& f )
{
  auto* r = bdd_up( f.manager(), f.getNode() );
  return BDD( manager, r );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
