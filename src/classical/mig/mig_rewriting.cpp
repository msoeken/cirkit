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

#include "mig_rewriting.hpp"

#include <functional>

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/graph/depth.hpp>
#include <core/utils/timer.hpp>
#include <classical/mig/mig_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using mig_function_vec_t = std::vector<mig_function>;

struct children_pair_t
{
  unsigned left_a;
  unsigned left_b;
  unsigned right_a;
  unsigned right_b;
};

inline std::pair<unsigned, unsigned> three_without( unsigned x )
{
  return std::make_pair( x == 0u ? 1u : 0u, x == 2u ? 1u : 2u );
}

class mig_rewriting_manager
{
public:
  mig_rewriting_manager( const mig_graph& mig, bool verbose );

  void swap_current( const std::string& method );
  inline unsigned depth() const { return max_depth; }

  void run_distributivity_rtl();
  void run_associativity();
  void run_compl_associativity();
  void run_push_up();
  void run_relevance();
  void run_memristor_optimization();
  void run_memristor_inverter();

  mig_function distributivity_rtl( const mig_node& f );
  mig_function distributivity_ltr( const mig_node& f );
  mig_function associativity( const mig_node& f );
  mig_function compl_associativity( const mig_node& f );
  mig_function relevance( const mig_node& f, std::map<mig_function, mig_function> replacements );

  mig_function push_up( const mig_node& f );
  mig_function memristor_optimization( const mig_node& f );
  mig_function memristor_inverter( const mig_node& f );

private:
  inline mig_function distributivity_rtl_apply( const children_pair_t& pair,
                                                const mig_function_vec_t& children_a,
                                                const mig_function_vec_t& children_b,
                                                const mig_function& other )
  {
    assert( children_a[pair.left_a] == children_b[pair.right_a] );
    assert( children_a[pair.left_b] == children_b[pair.right_b] );

    const auto x = pair.left_a;
    const auto y = pair.left_b;
    const auto u = 3u - pair.left_a - pair.left_b;
    const auto v = 3u - pair.right_a - pair.right_b;

    if ( verbose )
    {
      std::cout << boost::format( "[i] x: %d, y: %d, u: %d, v: %d") % x % y % u % v << std::endl;
    }

    return mig_create_maj( mig_current,
                           make_function( distributivity_rtl( children_a[x].node ), children_a[x].complemented ),
                           make_function( distributivity_rtl( children_a[y].node ), children_a[y].complemented ),
                           mig_create_maj(
                                          mig_current,
                                          make_function( distributivity_rtl( children_a[u].node ), children_a[u].complemented ),
                                          make_function( distributivity_rtl( children_b[v].node ), children_b[v].complemented ),
                                          make_function( distributivity_rtl( other.node ), other.complemented ) ) );
  }

  inline mig_function associativity_apply( const mig_function_vec_t& grand_children,
                                           const mig_function& common,
                                           const mig_function& extra )
  {
    const auto common_f = make_function( associativity( common.node ), common.complemented );
    return mig_create_maj( mig_current,
                           make_function( associativity( grand_children[0u].node ), grand_children[0u].complemented ),
                           common_f,
                           mig_create_maj( mig_current,
                                           make_function( associativity( grand_children[1u].node ), grand_children[1u].complemented ),
                                           common_f,
                                           make_function( associativity( extra.node ), extra.complemented ) ) );
  }

  inline mig_function compl_associativity_apply( const mig_function_vec_t& grand_children,
                                                 const mig_function& common,
                                                 const mig_function& extra )
  {
    const auto extra_f = make_function( compl_associativity( extra.node ), extra.complemented );
    return mig_create_maj( mig_current,
                           extra_f,
                           make_function( compl_associativity( common.node ), common.complemented ),
                           mig_create_maj( mig_current,
                                           make_function( compl_associativity( grand_children[0u].node ), grand_children[0u].complemented ),
                                           extra_f,
                                           make_function( compl_associativity( grand_children[1u].node ), grand_children[1u].complemented ) ) );
  }

  /**
   * @return (a,b) where z = node[a][b]
   */
  boost::optional<std::pair<unsigned, unsigned>> find_depth_distributivity_candidate( const mig_node& node ) const;

  /**
   * @return (a,b,c) where x = node[a] and z = node[b][c]
   */
  boost::optional<std::tuple<unsigned, unsigned, unsigned>> find_depth_associativity_candidate( const mig_node& node ) const;

  /**
   * @return (a,b) where x = node[a] and u = node[b]
   */
  boost::optional<std::pair<unsigned, unsigned>> find_depth_compl_associativity_candidate( const mig_node& node ) const;

  /**
   * @return (a,b,c) where x = node[a], y = node[b], and z = node[c]
   */
  boost::optional<std::tuple<mig_function, mig_function, mig_function>> find_depth_relevance_candidate( const mig_node& node ) const;

public:
  mig_graph                        mig_old;
  mig_graph                        mig_current;
  std::map<mig_node, mig_function> old_to_new;
  bool                             verbose;
  std::vector<unsigned>            indegree;
  std::vector<unsigned>            depths;
  unsigned                         max_depth;

  bool                             use_distributivity       = true;
  bool                             use_associativity        = true;
  bool                             use_compl_associativity  = true;

  /* statistics */
  unsigned                         distributivity_count       = 0u;
  unsigned                         associativity_count        = 0u;
  unsigned                         compl_associativity_count  = 0u;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

inline bool is_terminal( const mig_graph& mig, const mig_function& f )
{
  return out_degree( f.node, mig ) == 0u;
}

inline bool is_regular_nonterminal( const mig_graph& mig, const mig_function& f )
{
  return !f.complemented && out_degree( f.node, mig );
}

boost::dynamic_bitset<> get_pair_pattern( const mig_function_vec_t& c1,
                                          const mig_function_vec_t& c2 )
{
  boost::dynamic_bitset<> equals( 9u );

  equals.set( 0u, c1[0u] == c2[0u] );
  equals.set( 1u, c1[0u] == c2[1u] );
  equals.set( 2u, c1[0u] == c2[2u] );
  equals.set( 3u, c1[1u] == c2[0u] );
  equals.set( 4u, c1[1u] == c2[1u] );
  equals.set( 5u, c1[1u] == c2[2u] );
  equals.set( 6u, c1[2u] == c2[0u] );
  equals.set( 7u, c1[2u] == c2[1u] );
  equals.set( 8u, c1[2u] == c2[2u] );

  return equals;
}

std::vector<children_pair_t> get_children_pairs( const mig_function_vec_t& c1,
                                                 const mig_function_vec_t& c2 )
{
  std::vector<children_pair_t> pairs;

  const auto pattern = get_pair_pattern( c1, c2 );

  auto pos_a = pattern.find_first();

  while ( pos_a != boost::dynamic_bitset<>::npos )
  {
    auto pos_b = pattern.find_next( pos_a );
    while ( pos_b != boost::dynamic_bitset<>::npos )
    {
      pairs.push_back({(unsigned)pos_a / 3u,
                       (unsigned)pos_b / 3u,
                       (unsigned)pos_a % 3u,
                       (unsigned)pos_b % 3u} );
      pos_b = pattern.find_next( pos_b );
    }
    pos_a = pattern.find_next( pos_a );
  }

  return pairs;
}

mig_rewriting_manager::mig_rewriting_manager( const mig_graph& mig, bool verbose )
  : mig_current( mig ),
    verbose( verbose )
{
}

void mig_rewriting_manager::swap_current( const std::string& method )
{
  mig_old = mig_current;
  mig_current = mig_graph();

  const auto& info_old = mig_info( mig_old );
  mig_initialize( mig_current, info_old.model_name );

  auto& info_current = mig_info( mig_current );
  info_current.constant_used = info_old.constant_used;

  /* node to node mapping from old to new mig */
  old_to_new.clear();
  old_to_new.insert( {info_old.constant, {info_current.constant, false}} );

  for ( const auto& input : info_old.inputs )
  {
    old_to_new.insert( {input, mig_create_pi( mig_current, info_old.node_names.at( input ) )} );
  }

  /* indegrees */
  indegree = precompute_in_degrees( mig_old );

  /* depth */
  std::vector<mig_node> outputs;
  for ( const auto& output : info_old.outputs )
  {
    outputs += output.first.node;
  }

  max_depth = compute_depth( mig_old, outputs, depths );

  if ( verbose )
  {
    std::cout << "[i] current depth: " << max_depth << ", run " << method << std::endl;
  }
}

void mig_rewriting_manager::run_distributivity_rtl()
{
  swap_current( "D_RTL" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    mig_create_po( mig_current, make_function( distributivity_rtl( output.first.node ), output.first.complemented ), output.second );
  }
}

void mig_rewriting_manager::run_associativity()
{
  swap_current( "A" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    mig_create_po( mig_current, make_function( associativity( output.first.node ), output.first.complemented ), output.second );
  }
}

void mig_rewriting_manager::run_compl_associativity()
{
  swap_current( "C" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    mig_create_po( mig_current, make_function( compl_associativity( output.first.node ), output.first.complemented ), output.second );
  }
}

void mig_rewriting_manager::run_push_up()
{
  swap_current( "PU" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    mig_create_po( mig_current, make_function( push_up( output.first.node ), output.first.complemented ), output.second );
  }
}

void mig_rewriting_manager::run_relevance()
{
  swap_current( "R" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    std::map<mig_function, mig_function> replacements;
    mig_create_po( mig_current, make_function( relevance( output.first.node, replacements ), output.first.complemented ), output.second );
  }
}

void mig_rewriting_manager::run_memristor_optimization()
{
  /* mig_old is removed
     mig_old = mig_current
     mig_current is empty */
  swap_current( "MO" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    mig_create_po( mig_current, make_function( memristor_optimization( output.first.node ), output.first.complemented ), output.second );
  }
}

void mig_rewriting_manager::run_memristor_inverter()
{
  /* mig_old is removed
     mig_old = mig_current
     mig_current is empty */
  swap_current( "MO_INV" );

  for ( const auto& output : mig_info( mig_old ).outputs )
  {
    mig_create_po( mig_current, make_function( memristor_optimization( output.first.node ), output.first.complemented ), output.second );
  }
}


/**
 * 〈〈xyu〉〈xyv〉z〉↦〈xy〈uvz〉〉
 */
mig_function mig_rewriting_manager::distributivity_rtl( const mig_node& node )
{
  /* node is terminal */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() ) { return it->second; }

  const auto children = get_children( mig_old, node );
  mig_function_vec_t children_a, children_b, children_c;
  mig_function res;

  if ( is_regular_nonterminal( mig_old, children[0u] ) && indegree[children[0u].node] == 1u )
  {
    /* first child is not terminal */
    children_a = get_children( mig_old, children[0u].node );

    /* check (0,1) */
    if ( is_regular_nonterminal( mig_old, children[1u] ) && indegree[children[1u].node] == 1u )
    {
      children_b = get_children( mig_old, children[1u].node );

      const auto pairs = get_children_pairs( children_a, children_b );

      if ( !pairs.empty() )
      {
        res = distributivity_rtl_apply( pairs.front(), children_a, children_b, children[2u] );
        goto cache_and_return;
      }
    }

    /* check (0,2) */
    if ( is_regular_nonterminal( mig_old, children[2u] ) && indegree[children[2u].node] == 1u )
    {
      children_c = get_children( mig_old, children[2u].node );

      const auto pairs = get_children_pairs( children_a, children_c );

      if ( !pairs.empty() )
      {
        res = distributivity_rtl_apply( pairs.front(), children_a, children_c, children[1u] );
        goto cache_and_return;
      }
    }
  }

  /* check (1,2) */
  if ( is_regular_nonterminal( mig_old, children[1u] ) && is_regular_nonterminal( mig_old, children[2u] ) &&
       indegree[children[1u].node] == 1u && indegree[children[2u].node] == 1u )
  {
    if ( children_b.empty() ) { children_b = get_children( mig_old, children[1u].node ); }
    if ( children_c.empty() ) { children_c = get_children( mig_old, children[2u].node ); }

    const auto pairs = get_children_pairs( children_b, children_c );

    if ( !pairs.empty() )
    {
      res = distributivity_rtl_apply( pairs.front(), children_b, children_c, children[0u] );
      goto cache_and_return;
    }
  }

  /* recur */
  res = mig_create_maj( mig_current,
                        make_function( distributivity_rtl( children[0u].node ), children[0u].complemented ),
                        make_function( distributivity_rtl( children[1u].node ), children[1u].complemented ),
                        make_function( distributivity_rtl( children[2u].node ), children[2u].complemented ) );

cache_and_return:
  old_to_new.insert( {node, res} );
  return res;
}

/**
 * 〈xy〈uvz〉〉↦〈〈xyu〉〈xyv〉z〉
 */
mig_function mig_rewriting_manager::distributivity_ltr( const mig_node& node )
{
  /* node is terminal */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() ) { return it->second; }

  assert( false );

  return mig_function();
}

boost::optional<std::pair<unsigned, unsigned>> mig_rewriting_manager::find_depth_distributivity_candidate( const mig_node& node ) const
{
  const auto children = get_children( mig_old, node );

  for ( auto i = 0u; i < 3u; ++i )
  {
    if ( is_regular_nonterminal( mig_old, children[i] ) )
    {
      auto grand_children = get_children( mig_old, children[i].node );

      for ( auto j = 0u; j < 3u; ++j )
      {
        auto valid = true;

        for ( auto k = 0u; k < 3u; ++k )
        {
          if ( i == k ) { continue; }

          int diff = (int)depths[grand_children[j].node] - (int)depths[children[k].node];

          if ( diff < 2 ) { valid = false; break; }
        }

        if ( !valid ) { continue; }

        /* WE HAVE A CANDIDATE: z = ( i -> j ) */
        return std::make_pair( i, j );
      }
    }
  }

  return boost::none;
}

boost::optional<std::tuple<unsigned, unsigned, unsigned>> mig_rewriting_manager::find_depth_associativity_candidate( const mig_node& node ) const
{
  const auto children = get_children( mig_old, node );

  /* i iterates to find grand children (which contain z) */
  for ( auto i = 0u; i < 3u; ++i )
  {
    if ( !is_regular_nonterminal( mig_old, children[i] ) ) { continue; }

    const auto grand_children = get_children( mig_old, children[i].node );

    /* j iterates to find u */
    for ( auto j = 0u; j < 3u; ++j )
    {
      if ( i == j ) { continue; }

      /* if u ( = children[j] ) can be found in the grand_children */
      if ( boost::find( grand_children, children[j] ) != grand_children.end() )
      {
        for ( auto k = 0u; k < 3u; ++k )
        {
          if ( grand_children[k] == children[j] ) { continue; }

          int diff = (int)depths[grand_children[k].node] - (int)depths[children[3u - j - i].node];

          if ( diff >= 2 )
          {
            return std::make_tuple( 3u - j - i, i, k );
          }
        }
      }
    }
  }

  return boost::none;
}

boost::optional<std::pair<unsigned, unsigned>> mig_rewriting_manager::find_depth_compl_associativity_candidate( const mig_node& node ) const
{
  const auto children = get_children( mig_old, node );

  /* i iterates to find grand children (which contain z) */
  for ( auto i = 0u; i < 3u; ++i )
  {
    if ( !is_regular_nonterminal( mig_old, children[i] ) ) { continue; }

    const auto grand_children = get_children( mig_old, children[i].node );

    /* j iterates to find u */
    for ( auto j = 0u; j < 3u; ++j )
    {
      if ( i == j ) { continue; }

      /* if u ( = children[j] ) can be found in the grand_children */
      if ( boost::find( grand_children, !children[j] ) != grand_children.end() )
      {
        int diff = (int)depths[children[j].node] - (int)depths[children[3u - j - i].node];

        if ( diff >= 2 )
        {
          return std::make_pair( 3u - j - i, j );
        }
      }
    }
  }

  return boost::none;
}

boost::optional<std::tuple<mig_function, mig_function, mig_function>> mig_rewriting_manager::find_depth_relevance_candidate( const mig_node& node ) const
{
  auto children = get_children( mig_old, node );

  boost::sort( children, [this]( const mig_function& a, const mig_function& b ) { return depths.at( a.node ) > depths.at( b.node ); } );

  if ( indegree[children[1u].node] > 1u )
  {
    return std::make_tuple( children[0u], children[2u], children[1u] );
  }
  else if ( indegree[children[2u].node] > 1u )
  {
    return std::make_tuple( children[0u], children[1u], children[2u] );
  }
  else if ( indegree[children[0u].node] > 1u )
  {
    return std::make_tuple( children[1u], children[2u], children[0u] );
  }

  return boost::none;
}

/**
 * 〈xu〈yuz〉〉↦〈zu〈yux〉〉
 */
mig_function mig_rewriting_manager::associativity( const mig_node& node )
{
  /* node is terminal */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() ) { return it->second; }

  const auto children = get_children( mig_old, node );
  mig_function res;

  for ( auto i = 0u; i < 3u; ++i )
  {
    if ( is_regular_nonterminal( mig_old, children[i] ) && indegree[children[i].node] == 1u )
    {
      for ( auto j = 0u; j < 3u; ++j )
      {
        if ( i == j ) { continue; }

        auto grand_children = get_children( mig_old, children[i].node );

        const auto it = boost::find( grand_children, children[j] );
        if ( it != grand_children.end() )
        {
          grand_children.erase( it );
          res = associativity_apply( grand_children, children[j], children[3u - j - i] );
          goto cache_and_return;
        }
      }
    }
  }

  /* recur */
  res = mig_create_maj( mig_current,
                        make_function( associativity( children[0u].node ), children[0u].complemented ),
                        make_function( associativity( children[1u].node ), children[1u].complemented ),
                        make_function( associativity( children[2u].node ), children[2u].complemented ) );

cache_and_return:
  old_to_new.insert( {node, res} );
  return res;
}

/**
 * 〈xu〈yu'z〉〉↦〈xu〈yxz〉〉
 */
mig_function mig_rewriting_manager::compl_associativity( const mig_node& node )
{
  /* node is terminal */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() ) { return it->second; }

  const auto children = get_children( mig_old, node );
  mig_function res;

  for ( auto i = 0u; i < 3u; ++i )
  {
    if ( is_regular_nonterminal( mig_old, children[i] ) && indegree[children[i].node] == 1u )
    {
      for ( auto j = 0u; j < 3u; ++j )
      {
        if ( i == j ) { continue; }

        auto grand_children = get_children( mig_old, children[i].node );

        const auto it = boost::find( grand_children, !children[j] );
        if ( it != grand_children.end() )
        {
          grand_children.erase( it );
          res = compl_associativity_apply( grand_children, children[j], children[3u - j - i] );
          goto cache_and_return;
        }
      }
    }
  }

  /* recur */
  res = mig_create_maj( mig_current,
                        make_function( compl_associativity( children[0u].node ), children[0u].complemented ),
                        make_function( compl_associativity( children[1u].node ), children[1u].complemented ),
                        make_function( compl_associativity( children[2u].node ), children[2u].complemented ) );

cache_and_return:
  old_to_new.insert( {node, res} );
  return res;
}

/**
 * 〈xyz〉↦〈xyz_{x/y'}〉
 */
mig_function mig_rewriting_manager::relevance( const mig_node& f, std::map<mig_function, mig_function> replacements )
{
  /* node should be replaced? */
  auto it_replace = replacements.find( {f, false} );
  if ( it_replace != replacements.end() ) { return it_replace->second; }

  it_replace = replacements.find( {f, true} );
  if ( it_replace != replacements.end() ) { return !it_replace->second; }

  /* node is terminal */
  const auto it = old_to_new.find( f );
  if ( it != old_to_new.end() ) { return it->second; }

  mig_function res;

  const auto cand = find_depth_relevance_candidate( f );

  if ( cand != boost::none )
  {
    const auto zcand = *cand;

    const auto x = std::get<0>( zcand );
    const auto y = std::get<1>( zcand );
    const auto z = std::get<2>( zcand );

    assert( !( x == y ) );
    assert( !( x == z ) );
    assert( !( y == z ) );

    const auto xf = make_function( relevance( x.node, replacements ), x.complemented );
    const auto yf = make_function( relevance( y.node, replacements ), y.complemented );

    if ( xf.node != 0u && yf.node != 0u )
    {
      replacements.insert( {xf, !yf} );
    }
    const auto zf = make_function( relevance( z.node, replacements ), z.complemented );

    res = mig_create_maj( mig_current, xf, yf, zf );
  }
  else
  {
    const auto children = get_children( mig_old, f );

    /* recur */
    res = mig_create_maj( mig_current,
                          make_function( relevance( children[0u].node, replacements ), children[0u].complemented ),
                          make_function( relevance( children[1u].node, replacements ), children[1u].complemented ),
                          make_function( relevance( children[2u].node, replacements ), children[2u].complemented ) );
  }

  old_to_new.insert( {f, res} );
  return res;
}

mig_function mig_rewriting_manager::push_up( const mig_node& f )
{
  /* node is terminal */
  const auto it = old_to_new.find( f );
  if ( it != old_to_new.end() ) { return it->second; }

  mig_function res;

  const auto children = get_children( mig_old, f );

  /* distributivity */
    if ( use_distributivity )
  {
    const auto cand_dist = find_depth_distributivity_candidate( f );

    if ( cand_dist != boost::none )
    {
      const auto zcand = *cand_dist;

      const auto grand_children = get_children( mig_old, children[zcand.first].node );

      const auto xy = three_without( zcand.first );
      const auto uv = three_without( zcand.second );

      const auto x = children[xy.first];
      const auto y = children[xy.second];
      const auto u = grand_children[uv.first];
      const auto v = grand_children[uv.second];
      const auto z = grand_children[zcand.second];

      const auto xf = make_function( push_up( x.node ), x.complemented );
      const auto yf = make_function( push_up( y.node ), y.complemented );
      const auto uf = make_function( push_up( u.node ), u.complemented ^ children[zcand.first].complemented );
      const auto vf = make_function( push_up( v.node ), v.complemented ^ children[zcand.first].complemented );
      const auto zf = make_function( push_up( z.node ), z.complemented ^ children[zcand.first].complemented );

      res = mig_create_maj( mig_current,
                            mig_create_maj( mig_current, xf, yf, uf ),
                            mig_create_maj( mig_current, xf, yf, vf ),
                            zf );

      ++distributivity_count;
      goto cache_and_return;
    }
  }

  /* associativity */
  if ( use_associativity )
  {
    const auto cand_assoc = find_depth_associativity_candidate( f );

    if ( cand_assoc != boost::none )
    {
      const auto zcand = *cand_assoc;

      const auto grand_children = get_children( mig_old, children[std::get<1>( zcand )].node );

      assert( !children[std::get<1>( zcand )].complemented );

      const auto yu = three_without( std::get<2>( zcand ) );

      const auto x = children[std::get<0>( zcand )];
      const auto u = children[3u - std::get<0>( zcand ) - std::get<1>( zcand )];
      const auto z = grand_children[std::get<2>( zcand )];
      const auto y = ( u == grand_children[yu.first] ) ? grand_children[yu.second] : grand_children[yu.first];

      assert( grand_children[yu.first] == u || grand_children[yu.second] == u );

      const auto xf = make_function( push_up( x.node ), x.complemented );
      const auto uf = make_function( push_up( u.node ), u.complemented );
      const auto zf = make_function( push_up( z.node ), z.complemented );
      const auto yf = make_function( push_up( y.node ), y.complemented );

      res = mig_create_maj( mig_current,
                            zf,
                            uf,
                            mig_create_maj( mig_current, yf, uf, xf ) );

      ++associativity_count;
      goto cache_and_return;
    }
  }

  /* complementary associativity */
  if ( use_compl_associativity )
  {
    const auto cand = find_depth_compl_associativity_candidate( f );

    if ( cand != boost::none )
    {
      const auto zcand = *cand;

      const auto grand_children = get_children( mig_old, children[3u - zcand.first - zcand.second].node );

      const auto x = children[zcand.first];
      const auto u = children[zcand.second];

      const auto ui = std::distance( grand_children.begin(), boost::find( grand_children, !u ) );
      const auto yz = three_without( ui );

      const auto y = grand_children[yz.first];
      const auto z = grand_children[yz.second];

      const auto xf = make_function( push_up( x.node ), x.complemented );
      const auto uf = make_function( push_up( u.node ), u.complemented );
      const auto zf = make_function( push_up( z.node ), z.complemented );
      const auto yf = make_function( push_up( y.node ), y.complemented );

      res = mig_create_maj( mig_current,
                            xf,
                            uf,
                            mig_create_maj( mig_current, yf, xf, zf ) );

      ++compl_associativity_count;
      goto cache_and_return;
    }
  }

  /* recur */
  res = mig_create_maj( mig_current,
                        make_function( push_up( children[0u].node ), children[0u].complemented ),
                        make_function( push_up( children[1u].node ), children[1u].complemented ),
                        make_function( push_up( children[2u].node ), children[2u].complemented ) );

cache_and_return:
  old_to_new.insert( {f, res} );
  return res;
}

mig_function mig_rewriting_manager::memristor_optimization( const mig_node& f )
{
  /* node is terminal */
  const auto it = old_to_new.find( f );
  if ( it != old_to_new.end() ) { return it->second; }

  mig_function res;

  const auto children = get_children( mig_old, f );

  /* if at least two children are complemented */
  if ( ( static_cast<int>( children[0u].complemented ) + static_cast<int>( children[1u].complemented ) + static_cast<int>( children[2u].complemented ) >= 2 ) ) //&& indegree[f] == 1u )
  {
    res = !mig_create_maj( mig_current,
                           make_function( memristor_optimization( children[0u].node ), !children[0u].complemented ),
                           make_function( memristor_optimization( children[1u].node ), !children[1u].complemented ),
                           make_function( memristor_optimization( children[2u].node ), !children[2u].complemented ) );
  }
  else
  {
    res = mig_create_maj( mig_current,
                          make_function( memristor_optimization( children[0u].node ), children[0u].complemented ),
                          make_function( memristor_optimization( children[1u].node ), children[1u].complemented ),
                          make_function( memristor_optimization( children[2u].node ), children[2u].complemented ) );
  }

  old_to_new.insert( {f, res} );
  return res;
}



mig_function mig_rewriting_manager::memristor_inverter( const mig_node& f )
{
  /* node is terminal */
  const auto it = old_to_new.find( f );
  if ( it != old_to_new.end() ) { return it->second; }

  mig_function res;

  const auto children = get_children( mig_old, f );

  /* if at least two children are complemented */
  if ( ( static_cast<int>( children[0u].complemented ) + static_cast<int>( children[1u].complemented ) + static_cast<int>( children[2u].complemented ) == 3u ) && indegree[f] == 1u )
  {
    res = !mig_create_maj( mig_current,
                           make_function( memristor_optimization( children[0u].node ), !children[0u].complemented ),
                           make_function( memristor_optimization( children[1u].node ), !children[1u].complemented ),
                           make_function( memristor_optimization( children[2u].node ), !children[2u].complemented ) );
  }
  else
  {
    res = mig_create_maj( mig_current,
                          make_function( memristor_optimization( children[0u].node ), children[0u].complemented ),
                          make_function( memristor_optimization( children[1u].node ), children[1u].complemented ),
                          make_function( memristor_optimization( children[2u].node ), children[2u].complemented ) );
  }

  old_to_new.insert( {f, res} );
  return res;
}


/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_graph mig_area_rewriting( const mig_graph& mig,
                              const properties::ptr& settings,
                              const properties::ptr& statistics )
{
  /* settings */
  const auto effort  = get( settings, "effort",  1u );
  const auto verbose = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  mig_rewriting_manager mgr( mig, verbose );

  for ( auto k = 0u; k < effort; ++k )
  {
    mgr.run_distributivity_rtl();
    mgr.run_associativity();
    mgr.run_compl_associativity();
    mgr.run_distributivity_rtl();
  }

  set( statistics, "distributivity_count",       mgr.distributivity_count );
  set( statistics, "associativity_count",        mgr.associativity_count );
  set( statistics, "compl_associativity_count",  mgr.compl_associativity_count );

  return mgr.mig_current;
}

mig_graph mig_depth_rewriting( const mig_graph& mig,
                               const properties::ptr& settings,
                               const properties::ptr& statistics )
{
  /* settings */
  const auto effort                   = get( settings, "effort",  1u );
  const auto use_distributivity       = get( settings, "use_distributivity", true );
  const auto use_associativity        = get( settings, "use_associativity", true );
  const auto use_compl_associativity  = get( settings, "use_compl_associativity", true );
  const auto verbose                  = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  mig_rewriting_manager mgr( mig, verbose );
  mgr.use_distributivity = use_distributivity;
  mgr.use_associativity  = use_associativity;
  mgr.use_compl_associativity = use_compl_associativity;

  for ( auto k = 0u; k < effort; ++k )
  {
    mgr.run_push_up();
    mgr.run_relevance();
    mgr.run_push_up();
  }

  set( statistics, "distributivity_count",       mgr.distributivity_count );
  set( statistics, "associativity_count",        mgr.associativity_count );
  set( statistics, "compl_associativity_count",  mgr.compl_associativity_count );

  return mgr.mig_current;
}

mig_graph mig_memristor_rewriting( const mig_graph& mig,
                                   const properties::ptr& settings,
                                   const properties::ptr& statistics )
{
  /* settings */
  const auto effort   = get( settings, "effort",  1u );
  const auto verbose  = get( settings, "verbose", false );
  const auto strategy = get( settings, "strategy", 0u ); /* 0u: multi-objective, 1u: RRAM step, 2u: PLiM */

  /* timer */
  properties_timer t( statistics );

  mig_rewriting_manager mgr( mig, verbose );
  mgr.use_distributivity = true;
  mgr.use_associativity  = true;
  mgr.use_compl_associativity = true;

  for ( auto k = 0u; k < effort; ++k )
  {
    switch ( strategy )
    {
    case 0u:
      mgr.run_push_up();
      mgr.run_memristor_optimization();
      mgr.run_push_up();
      mgr.run_associativity();
      mgr.run_distributivity_rtl();
      break;
    case 1u:
      mgr.run_push_up();
      mgr.run_memristor_inverter();
      mgr.run_memristor_optimization();
      mgr.run_push_up();
      break;
    case 2u:
      mgr.run_distributivity_rtl();
      mgr.run_associativity();
      mgr.run_compl_associativity();
      mgr.run_distributivity_rtl();
      mgr.run_memristor_optimization();
      mgr.run_memristor_inverter();
      break;
    case 3u:
      mgr.run_memristor_optimization();
      mgr.run_memristor_inverter();
      break;
    }
  }

  set( statistics, "distributivity_count",       mgr.distributivity_count );
  set( statistics, "associativity_count",        mgr.associativity_count );
  set( statistics, "compl_associativity_count",  mgr.compl_associativity_count );

  return mgr.mig_current;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
