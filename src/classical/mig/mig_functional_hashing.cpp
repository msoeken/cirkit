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

#include "mig_functional_hashing.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/optional.hpp>

#include <core/graph/coi.hpp>
#include <core/graph/depth.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/cuts/stack.hpp>
#include <classical/functions/cuts/traits.hpp>
#include <classical/functions/fanout_free_regions.hpp>
#include <classical/mig/mig_from_string.hpp>
#include <classical/functions/npn_canonization.hpp>
#include <classical/mig/mig_simulate.hpp>
#include <classical/mig/mig_functional_hashing_constants.hpp>
#include <classical/utils/cut_enumeration.hpp>
#include <classical/utils/cut_enumeration_traits.hpp>
#include <classical/mig/mig_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

namespace cirkit
{

/******************************************************************************
 * Macros                                                                     *
 ******************************************************************************/

#define L(x) { if ( verbose ) { std::cout << x << std::endl; } }

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using mig_node_vec_t = std::vector<mig_node>;
using mig_edge_vec_t = std::vector<mig_edge>;

using opt_ffr_t      = boost::optional<std::pair<mig_node, std::vector<mig_node>>>;

struct candidate_t
{
  mig_function f;
  unsigned     area;
  unsigned     depth;

  inline bool operator==( const candidate_t& other ) const
  {
    return f == other.f;
  }
};

inline bool sort_candidates_area_first( const candidate_t& c1, const candidate_t& c2 )
{
  return ( c1.area < c2.area ) || ( c1.area == c2.area && c1.depth < c2.depth );
}

inline bool sort_candidates_depth_first( const candidate_t& c1, const candidate_t& c2 )
{
  return ( c1.depth < c2.depth ) || ( c1.depth == c2.depth && c1.area < c2.area );
}

using candidate_vec_t = std::vector<candidate_t>;

inline void sort_candidates( candidate_vec_t& candidates, bool sort_area_first )
{
  if ( sort_area_first )
  {
    boost::sort( candidates, sort_candidates_area_first );
  }
  else
  {
    boost::sort( candidates, sort_candidates_depth_first );
  }
}

struct visited_entry_t
{
  candidate_vec_t candidates;
  structural_cut  cuts;

  const candidate_t& min_element( unsigned max_size, bool sort_area_first )
  {
    if ( candidates.size() < max_size )
    {
      sort_candidates( candidates, sort_area_first );
    }
    return candidates.front();
  }
};

void add_candidate( candidate_vec_t& vec, const candidate_t& c, unsigned max_size, bool sort_area_first )
{
  if ( boost::find( vec, c ) != vec.end() ) { return; }

  if ( vec.size() < max_size )
  {
    vec.push_back( c );

    if ( vec.size() == max_size )
    {
      sort_candidates( vec, sort_area_first );
    }
  }
  else
  {
    if ( sort_area_first )
    {
      if ( sort_candidates_area_first( c, vec.back() ) )
      {
        vec.insert( boost::lower_bound( vec, c, sort_candidates_area_first ), c );
        vec.pop_back();
      }
    }
    else
    {
      if ( sort_candidates_depth_first( c, vec.back() ) )
      {
        vec.insert( boost::lower_bound( vec, c, sort_candidates_depth_first ), c );
        vec.pop_back();
      }
    }
  }
}

struct npn_hash_table_entry_t
{
  npn_hash_table_entry_t() {}
  npn_hash_table_entry_t( int tt, int npn, const std::vector<unsigned>& perm, const boost::dynamic_bitset<>& phase )
    : tt( tt ), npn( npn ), perm( perm ), phase( phase ) {}

  int                     tt  = -1;
  int                     npn = -1;
  std::vector<unsigned>   perm;
  boost::dynamic_bitset<> phase;
};

using npn_hash_table_t = std::vector<npn_hash_table_entry_t>;

class mig_functional_hashing_manager
{
public:
  mig_functional_hashing_manager( const mig_graph& mig, bool use_ffrs, bool top_down, unsigned npn_hash_table_size, bool verbose );

  void run();

private:
  int find_best_cut( const mig_node& node, const std::map<aig_node, structural_cut>& cuts,
                     boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm, std::string& expr );

  mig_function optimize_node( const std::vector<mig_node>& ffr_leafs, const mig_node& node,
                              const std::map<aig_node, structural_cut>& cuts );

  mig_function optimize_node( const mig_node& node,
                              const std::map<aig_node, structural_cut>& cuts );

  tt compute_npn( const tt& tt, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );

  bool is_fanout_free_cut( const mig_node& node, const boost::dynamic_bitset<>& cut ) const;

  // bottom-up
  void depth_preserving_functional_hashing( const opt_ffr_t& ffr = boost::none );
  mig_function copy_tmp_to_new( const mig_node& node, const mig_graph& mig_tmp, std::map<mig_node, mig_function>& visited );

private:
  inline std::vector<unsigned> inv( const std::vector<unsigned>& perm ) const
  {
    std::vector<unsigned> invperm( perm.size() );
    for ( auto i = 0u; i < perm.size(); ++i ) { invperm[perm[i]] = i; }
    return invperm;
  }

public:
  const mig_graph&                   mig;
  mig_graph                          mig_new;
  const mig_graph_info&              info;
  std::map<mig_node, mig_function>   old_to_new;
  bool                               use_ffrs;
  bool                               top_down;
  mig_node_vec_t                     topsort;
  std::map<mig_node, mig_node_vec_t> ffrs;
  mig_node_vec_t                     ffrs_topsort;
  std::map<mig_node, mig_edge_vec_t> ingoing;
  std::vector<unsigned>              depths;
  unsigned                           max_depth;
  npn_hash_table_t                   npn_table;
  bool                               progress;
  bool                               depth_heuristic;
  unsigned                           max_candidates = 10u;
  bool                               allow_area_inc = false;
  bool                               allow_depth_inc = false;
  bool                               sort_area_first = true;
  bool                               verbose;
  double                             runtime_ffr = 0.0;
  double                             runtime_cut = 0.0;
  double                             runtime_npn = 0.0;
  unsigned long                      cache_hit = 0ul;
  unsigned long                      cache_miss = 0ul;
  properties::ptr                    ffr_statistics;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

mig_functional_hashing_manager::mig_functional_hashing_manager( const mig_graph& mig, bool use_ffrs, bool top_down, unsigned npn_hash_table_size, bool verbose )
  : mig( mig ),
    info( mig_info( mig ) ),
    use_ffrs( use_ffrs ),
    top_down( top_down ),
    topsort( boost::num_vertices( mig ) ),
    npn_table( npn_hash_table_size ),
    verbose( verbose )
{
  mig_initialize( mig_new, info.model_name );

  auto& info_new = mig_info( mig_new );
  info_new.constant_used = info.constant_used;

  /* node to node mapping from old to new mig */
  old_to_new.insert( {info.constant, {info_new.constant, false}} );

  for ( const auto& input : info.inputs )
  {
    old_to_new.insert( {input, mig_create_pi( mig_new, info.node_names.at( input ) )} );
  }

  /* outputs */
  mig_node_vec_t outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  /* compute topological order of vertices */
  if ( use_ffrs )
  {
    boost::topological_sort( mig, topsort.begin() );

    /* compute fanout free regions */
    auto ffr_settings = std::make_shared<properties>();
    ffr_statistics = std::make_shared<properties>();
    ffr_settings->set( "outputs",      outputs );
    ffr_settings->set( "verbose",      verbose );
    ffr_settings->set( "relabel",      true );
    ffr_settings->set( "has_constant", true );
    ffrs = fanout_free_regions( mig, ffr_settings, ffr_statistics );
    runtime_ffr = ffr_statistics->get<double>( "runtime" );

    /* sort FFRs in topoplogical order */
    ffrs_topsort = topological_sort_ffrs<mig_graph>( ffrs, topsort );
  }
  else
  {
    ingoing = precompute_ingoing_edges( mig );
  }

  /* compute depths */
  max_depth = compute_depth( mig, outputs, depths );
}

void mig_functional_hashing_manager::run()
{
  if ( top_down )
  {
    if ( use_ffrs )
    {
      null_stream ns;
      std::ostream null_out( &ns );
      boost::progress_display show_progress( ffrs_topsort.size(), progress ? std::cout : null_out );

      for ( const auto& id : ffrs_topsort )
      {
        ++show_progress;

        L( "[i] optimize ffr at " << id );

        /* already computed? */
        if ( old_to_new.find( id ) != old_to_new.end() ) { continue; }

        /* perform cut enumeration */
        boost::dynamic_bitset<> boundary( boost::num_vertices( mig ) );
        for ( const auto& ffr_leaf : ffrs.at( id ) )
        {
          boundary.set( ffr_leaf );
        }

        auto sce_settings = std::make_shared<properties>();
        auto sce_statistics = std::make_shared<properties>();
        sce_settings->set( "boundary", boundary );
        sce_settings->set( "start_nodes", std::vector<mig_node>( {id} ) );
        auto cuts = structural_cut_enumeration( mig, 5u, sce_settings, sce_statistics );

        runtime_cut += sce_statistics->get<double>( "runtime" );

        const auto f = optimize_node( ffrs.at( id ), id, cuts );
        old_to_new.insert( {id, f} );
      }
    }
    else
    {
      /* perform cut enumeration */
      auto sce_settings = std::make_shared<properties>();
      auto sce_statistics = std::make_shared<properties>();

      L( "[i] start cut enumeration" );
      auto cuts = stack_based_structural_cut_enumeration( mig, 5u, sce_settings, sce_statistics );
      L( "[i] stop cut enumeration" );

      for ( const auto& output : info.outputs )
      {
        const auto id = output.first.node;

        L( "[i] optimize output at " << id );

        optimize_node( id, cuts );
      }

      runtime_cut += sce_statistics->get<double>( "runtime" );
    }
  }
  else
  {
    if ( use_ffrs )
    {
      for ( const auto& id : ffrs_topsort )
      {
        L( "[i] optimize ffr at " << id );

        depth_preserving_functional_hashing( opt_ffr_t( *ffrs.find( id ) ) );
      }
    }
    else
    {
      depth_preserving_functional_hashing();
    }
  }

  for ( const auto& output : info.outputs )
  {
    const auto& f = old_to_new.at( output.first.node );
    mig_create_po( mig_new, output.first.complemented ? !f : f, output.second );
  }
}

int mig_functional_hashing_manager::find_best_cut( const mig_node& node, const std::map<mig_node, structural_cut>& cuts,
                                                   boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm, std::string& expr )
{
  auto best_gain  = 0u;
  auto best_index = -1;

  for ( const auto& cut : index( cuts.at( node ) ) )
  {
    const auto current_area  = cut_cone_size( node, cut.value, mig ) - cut.value.count();
    const auto current_depth = cut_cone_depth( node, cut.value, mig );

    if ( current_area == 1u ) { continue; }

    auto cut_copy = cut.value; cut_copy.reset( 0u );
    if ( cut_copy.count() >= 4u ) { continue; }

    auto tt         = simulate_cut_tt<mig_graph>( node, cut.value, mig );
    //const auto vars = tt_num_vars( tt );

    tt_shrink( tt, 4u );

    boost::dynamic_bitset<> local_phase;
    std::vector<unsigned>   local_perm;
    const auto npn = compute_npn( tt, local_phase, local_perm );

    /* better result? */
    const auto best_area  = std::get<0>( mig_functional_hashing_constants::min_depth_mig_sizes.at( npn.to_ulong() ) );
    const auto best_depth = std::get<1>( mig_functional_hashing_constants::min_depth_mig_sizes.at( npn.to_ulong() ) );

    if ( verbose )
    {
      std::cout << "[i]   analyze cut ";
      print_as_set( std::cout, cut.value ) << ", tt: " << tt_to_hex( tt )
                                           << ", npn: " << tt_to_hex( npn )
                                           << ", cur. area:  " << current_area
                                           << ", cur. depth: " << current_depth
                                           << ", best area:  " << best_area
                                           << ", best depth: " << best_depth << std::endl;
    }

    if ( ( current_area - best_area ) > best_gain && ( !depth_heuristic || ( best_depth < current_depth ) ) && ( use_ffrs || is_fanout_free_cut( node, cut.value ) ) )
    {
      L( "[i]    new local optimum" );
      best_gain  = current_area - best_area;
      best_index = cut.index;
      phase      = local_phase;
      perm       = local_perm;
      expr       = std::get<3>( mig_functional_hashing_constants::min_depth_mig_sizes.at( npn.to_ulong() ) );
    }
  }

  return best_index;
}

mig_function mig_functional_hashing_manager::optimize_node( const std::vector<mig_node>& ffr_leafs, const mig_node& node,
                                                            const std::map<aig_node, structural_cut>& cuts )
{
  L( "[i]  optimize node " << node );

  /* node is leaf of the FFR */
  if ( boost::find( ffr_leafs, node ) != ffr_leafs.end() )
  {
    return old_to_new.at( node );
  }

  boost::dynamic_bitset<> phase;
  std::vector<unsigned>   perm;
  std::string             expr;
  auto best_cut = find_best_cut( node, cuts, phase, perm, expr );

  /* there is no better realization */
  if ( best_cut == -1 )
  {
    auto children = get_children( mig, node );
    return mig_create_maj( mig_new,
                           optimize_node( ffr_leafs, children[0].node, cuts ) ^ children[0].complemented,
                           optimize_node( ffr_leafs, children[1].node, cuts ) ^ children[1].complemented,
                           optimize_node( ffr_leafs, children[2].node, cuts ) ^ children[2].complemented );
  }

  std::map<char, mig_function> var_to_function;
  const auto vars = std::string( "abcd" );
  auto index = 0u;

  const auto invperm = inv( perm );

  foreach_bit( cuts.at( node ).at( best_cut ), [&]( unsigned child ) {
      if ( child != 0u )
      {
        const auto childf = optimize_node( ffr_leafs, child, cuts );
        var_to_function.insert( {vars[invperm[index]], phase.test( index ) ? !childf : childf} );
        ++index;
      }
    } );

  auto mfs_settings = std::make_shared<properties>();
  mfs_settings->set( "variable_map", var_to_function );

  return make_function( mig_from_string( mig_new, expr, mfs_settings ), phase.test( phase.size() - 1u ) );
}

mig_function mig_functional_hashing_manager::optimize_node( const mig_node& node,
                                                            const std::map<aig_node, structural_cut>& cuts )
{
  L( "[i]  optimize node " << node );

  /* already computed? */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() ) { return it->second; }

  boost::dynamic_bitset<> phase;
  std::vector<unsigned>   perm;
  std::string             expr;
  auto best_cut = find_best_cut( node, cuts, phase, perm, expr );

  /* there is no better realization */
  if ( best_cut == -1 )
  {
    auto children = get_children( mig, node );
    auto f = mig_create_maj( mig_new,
                             make_function( optimize_node( children[0].node, cuts ), children[0].complemented ),
                             make_function( optimize_node( children[1].node, cuts ), children[1].complemented ),
                             make_function( optimize_node( children[2].node, cuts ), children[2].complemented ) );

    old_to_new.insert( {node, f} );
    return f;
  }

  std::map<char, mig_function> var_to_function;
  const auto vars = std::string( "abcd" );
  auto index = 0u;

  const auto invperm = inv( perm );

  foreach_bit( cuts.at( node ).at( best_cut ), [&]( unsigned child ) {
      if ( child != 0u )
      {
        const auto childf = optimize_node( child, cuts );
        var_to_function.insert( {vars[invperm[index]], phase.test( index ) ? !childf : childf} );
        ++index;
      }
    } );

  auto mfs_settings = std::make_shared<properties>();
  mfs_settings->set( "variable_map", var_to_function );

  auto f = mig_from_string( mig_new, expr, mfs_settings );

  if ( phase.test( phase.size() - 1u ) )
  {
    f = !f;
  }

  old_to_new.insert( {node, f} );

  return f;
}

tt mig_functional_hashing_manager::compute_npn( const tt& tt, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm )
{
  boost::dynamic_bitset<> npn;

  /* compute NPN and use hash table if possible */
  if ( !npn_table.empty() )
  {
    const auto ttu      = tt.to_ulong();
    const auto hash_key = ttu % npn_table.size();
    auto& entry   = npn_table[hash_key];

    if ( static_cast<unsigned long>( entry.tt ) == ttu )
    {
      ++cache_hit;
      npn = boost::dynamic_bitset<>( 16u, entry.npn );
      perm = std::vector<unsigned>( entry.perm );
      phase = boost::dynamic_bitset<>( entry.phase );
    }
    else
    {
      ++cache_miss;
      increment_timer t( &runtime_npn );
      npn = exact_npn_canonization( tt, phase, perm );

      entry.tt    = ttu;
      entry.npn   = npn.to_ulong();
      entry.perm  = perm;
      entry.phase = phase;
    }
  }
  else
  {
    increment_timer t( &runtime_npn );
    npn = exact_npn_canonization( tt, phase, perm );
  }

  return npn;
}

bool mig_functional_hashing_manager::is_fanout_free_cut( const mig_node& node, const boost::dynamic_bitset<>& cut ) const
{
  const auto cone = cut_cone( node, cut, mig );

  auto pos = cone.find_first();

  while ( pos != boost::dynamic_bitset<>::npos )
  {
    if ( pos != node && !cut.test( pos ) )
    {
      for ( const auto& in : ingoing.at( pos ) )
      {
        if ( !cone.test( boost::target( in, mig ) ) )
        {
          return false;
        }
      }
    }

    pos = cone.find_next( pos );
  }

  return true;
}

void mig_functional_hashing_manager::depth_preserving_functional_hashing( const opt_ffr_t& ffr )
{
  /* size of the MIG */
  const auto n = boost::num_vertices( mig );

  /* a temporary MIG */
  mig_graph mig_tmp;
  mig_initialize( mig_tmp, info.model_name );

  /* map nodes from mig to candidates in mig_tmp */
  std::vector<visited_entry_t> node_map( n );

  /* topological sort */
  std::vector<mig_node> topsort;

  if ( ffr == boost::none )
  {
    boost::topological_sort( mig, std::back_inserter( topsort ) );
  }
  else
  {
    using topo_visitor = boost::topo_sort_visitor<std::back_insert_iterator<std::vector<mig_node>>>;
    mig_node_color_map colors;
    boost::depth_first_visit( mig,
                              ffr->first,
                              topo_visitor( std::back_inserter( topsort ) ),
                              boost::make_assoc_property_map( colors ),
                              [&]( const mig_node& n, const mig_graph& mig ) { return boost::find( ffr->second, n ) != ffr->second.end(); } );
  }

  /* iterate through the vertices */
  for ( const auto& node : topsort )
  {
    L( "[i] optimizing node " << node );

    /* constant or input? */
    if ( boost::out_degree( node, mig ) == 0u || ( ffr != boost::none && boost::find( ffr->second, node ) != ffr->second.end() ) )
    {
      /* constant? */
      if ( node == info.constant )
      {
        candidate_t candidate{ mig_get_constant( mig_tmp, false ), 0u, 0u };
        node_map[node] = {{candidate}, {{boost::dynamic_bitset<>( n )}}};
      }
      else
      {
        //candidate_t candidate{ mig_create_pi( mig_tmp, info.node_names.at( node ) ), 0u, 0u };
        candidate_t candidate{ mig_create_pi( mig_tmp, boost::str( boost::format( "PI_%d" ) % node ) ), 0u, 0u };
        node_map[node] = {{candidate}, {{onehot_bitset( n, node )}}};
      }
    }
    else
    {
      /* compute all cuts */
      structural_cut cuts{onehot_bitset( n, node )};

      const auto children = get_children( mig, node );

      for ( const auto& c1 : node_map[children[0u].node].cuts )
      {
        for ( const auto& c2 : node_map[children[1u].node].cuts )
        {
          for ( const auto& c3 : node_map[children[2u].node].cuts )
          {
            const auto new_cut = c1 | c2 | c3;
            if ( new_cut.count() >= 5u ) continue;
            //if ( ( new_cut & ~boost::dynamic_bitset<>( n, 0u ) ).count() > 5u ) continue;
            if ( ffr == boost::none && !is_fanout_free_cut( node, new_cut ) ) continue;
            cuts += new_cut;
          }
        }
      }

      candidate_vec_t candidates;

      /* default substitution */
      std::vector<unsigned> a( 4u, 0u );
      std::vector<unsigned> m{ 2u, (unsigned)node_map[children[0u].node].candidates.size(),
                                   (unsigned)node_map[children[1u].node].candidates.size(),
                                   (unsigned)node_map[children[2u].node].candidates.size() };

      mixed_radix( a, m, [&]( const std::vector<unsigned>& a ) {
          std::vector<mig_function> fs( 3u );
          auto max_depth = 0u;

          for ( auto i = 0u; i < 3u; ++i )
          {
            const auto& cand = node_map[children[i].node].candidates[a[i + 1u]];
            fs[i] = make_function( cand.f, children[i].complemented );
            max_depth = std::max( max_depth, cand.depth );
          }

          const auto new_f = mig_create_maj( mig_tmp, fs[0u], fs[1u], fs[2u] );
          const auto new_f_area = compute_coi_size( mig_tmp, new_f.node );

          add_candidate( candidates, {new_f, new_f_area, max_depth + 1u}, max_candidates, sort_area_first );

          return false;
        } );

      /* substitute all cuts */
      for ( const auto& cut : cuts )
      {
        auto cone = cut_cone( node, cut, mig );
        cone.reset( info.constant );
        const auto current_area = cone.count() - cut.count();

        if ( current_area <= 1u ) { continue; }

        auto tt         = simulate_cut_tt( node, cut, mig );
        //const auto vars = tt_num_vars( tt );

        tt_shrink( tt, 4u );

        boost::dynamic_bitset<> phase;
        std::vector<unsigned>   perm;
        const auto npn = compute_npn( tt, phase, perm );

        const auto best_area = std::get<0>( mig_functional_hashing_constants::min_depth_mig_sizes.at( npn.to_ulong() ) );

        if ( !allow_area_inc && best_area > current_area ) { continue; }

        std::map<char, mig_function> var_to_function;
        const auto vars = std::string( "abcd" );

        const auto invperm = inv( perm );
        const auto leafs = get_index_vector( cut );

        /* mixed radix preparation */
        std::vector<unsigned> a( leafs.size() + 1u, 0u );
        std::vector<unsigned> m( leafs.size() + 1u, 2u );

        for ( auto i = 0u; i < leafs.size(); ++i )
        {
          m[i + 1u] = node_map[leafs[i]].candidates.size();
          assert( m[i + 1u] > 0u );
        }

        mixed_radix( a, m, [&]( const std::vector<unsigned>& a ) {
            std::map<char, mig_function> var_to_function;

            /* depth of new_f */
            auto max_depth = 0u;
            const auto& arrival = std::get<2>( mig_functional_hashing_constants::min_depth_mig_sizes.at( npn.to_ulong() ) );

            auto index = 0u;
            for ( auto i = 0u; i < leafs.size(); ++i )
            {
              const auto& cand  = node_map[leafs[i]].candidates[a[i + 1u]];
              const auto childf = cand.f;

              const auto key = 'a' + invperm[index];
              var_to_function.insert( {key, make_function( childf, phase.test( i ) ) } );

              const auto it_arrival = arrival.find( key );
              if ( it_arrival != arrival.end() )
              {
                max_depth = std::max( max_depth, cand.depth + it_arrival->second );
              }
              ++index;
            }

            if ( !allow_depth_inc && max_depth > depths[node] ) { return false; }

            auto mfs_settings = std::make_shared<properties>();
            mfs_settings->set( "variable_map", var_to_function );

            const auto& expr = std::get<3>( mig_functional_hashing_constants::min_depth_mig_sizes.at( npn.to_ulong() ) );
            const auto new_f = make_function( mig_from_string( mig_tmp, expr, mfs_settings ), phase.test( phase.size() - 1u ) );

            /* area of new_f */
            const auto new_f_area = compute_coi_size( mig_tmp, new_f.node );

            add_candidate( candidates, {new_f, new_f_area, max_depth}, max_candidates, sort_area_first );

            return false;
          } );
      }

      node_map[node] = {candidates, cuts};

      L( boost::format( "[i] found %d candidates" ) % candidates.size() );
    }
  }

  if ( ffr == boost::none )
  {
    std::map<mig_node, mig_function> tmp_to_new;

    for ( const auto& p : old_to_new )
    {
      assert( node_map[p.first].candidates.size() == 1u );
      tmp_to_new.insert( {node_map[p.first].candidates.front().f.node, p.second} );
    }

    for ( const auto& output : info.outputs )
    {
      const auto tmp_f = node_map[output.first.node].min_element( max_candidates, sort_area_first ).f;
      const auto f = make_function( copy_tmp_to_new( tmp_f.node, mig_tmp, tmp_to_new ), tmp_f.complemented );
      old_to_new.insert( {output.first.node, f} );
    }
  }
  else
  {
    std::map<mig_node, mig_function> tmp_to_new;

    for ( const auto& child : ffr->second )
    {
      assert( node_map[child].candidates.size() == 1u );
      tmp_to_new.insert( {node_map[child].candidates.front().f.node, old_to_new.at( child )} );
    }

    const auto tmp_f = node_map[ffr->first].min_element( max_candidates, sort_area_first ).f;

    // std::cout << "candidates for " << ffr->first << std::endl;

    // for ( const auto& c : node_map[ffr->first].candidates )
    // {
    //   std::cout << "  " << c.f.node << " " << c.area << " " << c.depth << std::endl;
    // }

    const auto f = make_function( copy_tmp_to_new( tmp_f.node, mig_tmp, tmp_to_new ), tmp_f.complemented );
    old_to_new.insert( {ffr->first, f} );
  }
}

mig_function mig_functional_hashing_manager::copy_tmp_to_new( const mig_node& node, const mig_graph& mig_tmp, std::map<mig_node, mig_function>& visited )
{
  const auto it = visited.find( node );
  if ( it != visited.end() ) { return it->second; }

  // if ( boost::out_degree( node, mig_tmp ) == 0u )
  // {
  //   std::cout << "vertex " << node << " should not be a termimal." << std::endl;
  //   assert( false );
  // }

  const auto children = get_children( mig_tmp, node );

  auto f = mig_create_maj( mig_new,
                           make_function( copy_tmp_to_new( children[0u].node, mig_tmp, visited ), children[0u].complemented ),
                           make_function( copy_tmp_to_new( children[1u].node, mig_tmp, visited ), children[1u].complemented ),
                           make_function( copy_tmp_to_new( children[2u].node, mig_tmp, visited ), children[2u].complemented ) );

  visited.insert( {node, f} );
  return f;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_graph mig_functional_hashing( const mig_graph& mig,
                                  const properties::ptr& settings,
                                  const properties::ptr& statistics )
{
  /* settings */
  const auto top_down            = get( settings, "top_down",            true );
  const auto use_ffrs            = get( settings, "use_ffrs",            true );
  const auto depth_heuristic     = get( settings, "depth_heuristic",     false );
  const auto npn_hash_table_size = get( settings, "npn_hash_table_size", 0u );
  const auto progress            = get( settings, "progress",            false );
  const auto max_candidates      = get( settings, "max_candidates",      10u );
  const auto allow_area_inc      = get( settings, "allow_area_inc",      false );
  const auto allow_depth_inc     = get( settings, "allow_depth_inc",     false );
  const auto sort_area_first     = get( settings, "sort_area_first",     true );
  const auto verbose             = get( settings, "verbose",             false );

  /* timing */
  properties_timer t( statistics );

  /* new graph */
  mig_functional_hashing_manager mgr( mig, use_ffrs, top_down, npn_hash_table_size, verbose );
  mgr.depth_heuristic = depth_heuristic;
  mgr.progress        = progress;
  mgr.max_candidates  = max_candidates;
  mgr.allow_area_inc  = allow_area_inc;
  mgr.allow_depth_inc = allow_depth_inc;
  mgr.sort_area_first = sort_area_first;

  mgr.run();

  set( statistics, "runtime_ffr", mgr.runtime_ffr );
  set( statistics, "runtime_cut", mgr.runtime_cut );
  set( statistics, "runtime_npn", mgr.runtime_npn );
  set( statistics, "cache_hit",   mgr.cache_hit );
  set( statistics, "cache_miss",  mgr.cache_miss );

  return mgr.mig_new;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
