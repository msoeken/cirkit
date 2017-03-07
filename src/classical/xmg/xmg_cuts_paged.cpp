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

#include "xmg_cuts_paged.hpp"

#include <map>
#include <mutex>
#include <stack>
#include <type_traits>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg_simulate.hpp>
#include <classical/xmg/xmg_utils.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/numeric.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::vector<std::pair<unsigned, unsigned>> compute_level_ranges( xmg_graph& xmg, unsigned& max_level )
{
  xmg.compute_levels();
  xmg.compute_parents();

  max_level = 0u;
  for ( const auto& o : xmg.outputs() )
  {
    max_level = std::max( max_level, xmg.level( o.first.node ) );
  }

  std::vector<unsigned> levels( xmg.size() );
  for ( const auto& v : xmg.nodes() )
  {
    levels[v] = xmg.level( v );
  }

  std::vector<std::pair<unsigned, unsigned>> level_ranges( xmg.size() );

  const auto top = xmg.topological_nodes();
  for ( const auto& v : top )
  {
    const auto& parents = xmg.parents( v );

    if ( parents.empty() )
    {
      level_ranges[v] = {levels[v], max_level};
      levels[v] = max_level;
      continue;
    }

    const auto min_parent = *boost::min_element( parents, [&levels]( const xmg_node& p1, const xmg_node& p2 ) {
        return levels[p1] < levels[p2];
      } );
    auto to_l = levels[min_parent] - 1u;
    level_ranges[v] = {levels[v], to_l};
    levels[v] = to_l;
  }

  return level_ranges;
}

xmg_cuts_paged::xmg_cuts_paged( xmg_graph& xmg, unsigned k, const properties::ptr& settings )
  : _xmg( xmg ),
    _k( k ),
    _priority( get( settings, "priority", 8u ) ),
    _extra( get( settings, "extra", 0u ) ),
    _progress( get( settings, "progress", false ) ),
    data( _xmg.size(), 2u + _extra ),
    cones( _xmg.size() )
{
  unsigned max_level;
  _levels = compute_level_ranges( xmg, max_level );

  enumerate();
}

xmg_cuts_paged::xmg_cuts_paged( xmg_graph& xmg, unsigned k, const std::vector<xmg_node>& start, const std::vector<xmg_node>& boundary,
                                const std::vector<std::pair<unsigned, unsigned>>& levels, const properties::ptr& settings )
  : _xmg( xmg ),
    _k( k ),
    _priority( get( settings, "priority", 8u ) ),
    _extra( get( settings, "extra", 0u ) ),
    _progress( get( settings, "progress", false ) ),
    data( _xmg.size(), 2u + _extra ),
    cones( _xmg.size() ),
    _levels( levels )
{
  enumerate_partial( start, boundary );
}

const xmg_graph& xmg_cuts_paged::xmg() const
{
  return _xmg;
}

unsigned xmg_cuts_paged::total_cut_count() const
{
  return data.sets_count();
}

double xmg_cuts_paged::enumeration_time() const
{
  return _enumeration_time;
}

unsigned xmg_cuts_paged::memory() const
{
  return data.memory() + cones.memory();
}

unsigned xmg_cuts_paged::count( xmg_node node ) const
{
  return data.count( node );
}

boost::iterator_range<paged_memory::iterator> xmg_cuts_paged::cuts( xmg_node node )
{
  return data.sets( node );
}

boost::iterator_range<paged_memory::iterator> xmg_cuts_paged::cut_cones( xmg_node node )
{
  return cones.sets( node );
}

tt xmg_cuts_paged::simulate( xmg_node node, const xmg_cuts_paged::cut& c ) const
{
  std::vector<xmg_node> leafs;
  for ( auto child : c )
  {
    leafs.push_back( child );
  }
  return xmg_simulate_cut( _xmg, node, leafs );
}

unsigned xmg_cuts_paged::depth( xmg_node node, const xmg_cuts_paged::cut& c ) const
{
  return c.extra( 0u );
}

unsigned xmg_cuts_paged::size( xmg_node node, const xmg_cuts_paged::cut& c ) const
{
  return c.extra( 1u );
}

unsigned xmg_cuts_paged::index( const xmg_cuts_paged::cut& c ) const
{
  return data.index( c );
}

xmg_cuts_paged::cut xmg_cuts_paged::from_address( unsigned address )
{
  return data.from_address( address );
}

void xmg_cuts_paged::foreach_cut( const std::function<void(xmg_node, cut&)>& func )
{
  for ( const auto& n : _xmg.nodes() )
  {
    for ( auto cut : cuts( n ) )
    {
      func( n, cut );
    }
  }
}

void xmg_cuts_paged::enumerate()
{
  reference_timer t( &_enumeration_time );

  /* topsort */
  const auto top = _xmg.topological_nodes();

  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( top.size(), _progress ? std::cout : null_out );

  /* loop */
  _top_index = 0u;
  for ( auto n : top )
  {
    ++show_progress;

    if ( _xmg.is_input( n ) )
    {
      /* constant */
      if ( n == 0u )
      {
        data.assign_empty( 0u, get_extra( 0u, 0u ) );
        cones.assign_empty( 0u );
      }
      /* PI */
      else
      {
        data.assign_singleton( n, n, get_extra( 0u, 1u ) );
        cones.assign_singleton( n, n );
      }
    }
    else
    {
      data.append_begin( n );
      cones.append_begin( n );

      std::vector<xmg_node> cns;
      for ( const auto& c : _xmg.children( n ) )
      {
        cns.push_back( c.node );
      }

      enumerate_node_with_bitsets( n, cns );

      data.append_singleton( n, n, get_extra( 0u, 1u ) );
      cones.append_singleton( n, n );
    }

    _top_index++;
  }
}

void xmg_cuts_paged::enumerate_with_xor_blocks( const std::unordered_map<xmg_node, xmg_xor_block_t>& blocks )
{
  reference_timer t( &_enumeration_time );

  /* find ignored nodes */
  boost::dynamic_bitset<> ignore( _xmg.size() );
  std::unordered_map<xmg_node, boost::dynamic_bitset<>> block_areas;

  for ( const auto& p : blocks )
  {
    auto root = p.first;
    const auto& leafs = p.second;
    boost::dynamic_bitset<> area( _xmg.size() );
    area.set( p.first );

    std::stack<xmg_node> stack;
    for ( const auto& c : _xmg.children( p.first ) )
    {
      stack.push( c.node );
    }

    while ( !stack.empty() )
    {
      auto n = stack.top();
      stack.pop();

      area.set( n );

      /* no leaf -> internal node */
      if ( boost::find( leafs, n ) != leafs.end() )
      {
        ignore.set( n );
        for ( const auto& c : _xmg.children( n ) )
        {
          stack.push( c.node );
        }
      }
    }

    block_areas[root] = area;
  }

    /* topsort */
  const auto top = _xmg.topological_nodes();

  /* loop */
  std::unordered_map<xmg_node, xmg_xor_block_t>::const_iterator it;
  //  std::remove_const<decltype( blocks )>::type::const_iterator it;
  _top_index = 0u;
  for ( auto n : top )
  {
    if ( _xmg.is_input( n ) )
    {
      /* constant */
      if ( n == 0u )
      {
        data.assign_empty( 0u, get_extra( 0u, 0u ) );
        cones.assign_empty( 0u );
      }
      /* PI */
      else
      {
        data.assign_singleton( n, n, get_extra( 0u, 1u ) );
        cones.assign_singleton( n, n );
      }
    }
    else if ( ignore[n] )
    {
      /* skip */
    }
    else if ( ( it = blocks.find( n ) ) != blocks.end() )
    {
      data.append_begin( n );
      cones.append_begin( n );

      std::vector<unsigned> leafs( it->second.size() );
      boost::copy( it->second, leafs.begin() );

      const auto& area = block_areas[n];
      data.append_set( n, leafs, get_extra( 0u, area.count() ) );
      cones.append_set( n, get_index_vector( area ) );

      data.append_singleton( n, n, get_extra( 0u, 1u ) );
      cones.append_singleton( n, n );
    }
    else
    {
      data.append_begin( n );
      cones.append_begin( n );

      std::vector<xmg_node> cns;
      for ( const auto& c : _xmg.children( n ) )
      {
        cns.push_back( c.node );
      }

      enumerate_node_with_bitsets( n, cns );

      data.append_singleton( n, n, get_extra( 0u, 1u ) );
      cones.append_singleton( n, n );
    }

    _top_index++;
  }
}

void xmg_cuts_paged::enumerate_partial( const std::vector<xmg_node>& start, const std::vector<xmg_node>& boundary )
{
  using boost::adaptors::reversed;

  reference_timer t( &_enumeration_time );

  std::vector<xmg_node> colors( _xmg.size(), 0u );

  /* children */
  data.assign_empty( 0u, get_extra( 0u, 0u ) );
  cones.assign_empty( 0u );
  colors[0u] = 2u;

  for ( auto n : boundary )
  {
    if ( n == 0 ) { continue; }
    data.assign_singleton( n, n, get_extra( 0u, 1u ) );
    cones.assign_singleton( n, n );
    colors[n] = 2u;
  }

  /* create order */
  std::vector<xmg_node> topo;
  std::stack<xmg_node> stack;
  for ( auto root : start )
  {
    stack.push( root );
  }

  while ( !stack.empty() )
  {
    auto n = stack.top();

    switch ( colors[n] )
    {
    case 0u:
      {
        for ( const auto& c : _xmg.children( n ) | reversed )
        {
          stack.push( c.node );
        }
        colors[n] = 1u;
      } break;

    case 1u:
      colors[n] = 2u;
      topo.push_back( n );
      stack.pop();
      break;

    case 2u:
      stack.pop();
      break;
    }
  }

  for ( auto n : topo )
  {
    data.append_begin( n );
    cones.append_begin( n );

    std::vector<xmg_node> cns;
    for ( const auto& c : _xmg.children( n ) )
    {
      cns.push_back( c.node );
    }

    _top_index = _xmg.size();
    enumerate_node_with_bitsets( n, cns );

    data.append_singleton( n, n, get_extra( 0u, 1u ) );
    cones.append_singleton( n, n );
  }
}

void xmg_cuts_paged::merge_cut( local_cut_vec_t& local_cuts, const boost::dynamic_bitset<>& new_cut, unsigned min_level, const boost::dynamic_bitset<>& new_cone ) const
{
  /* too large? */
  if ( new_cut.count() > _k ) { return; }

  auto first_subsume = true;
  auto add = true;

  auto l = 0u;
  while ( l < local_cuts.size() )
  {
    auto cut = std::get<0>( local_cuts[l] );

    /* same cut */
    if ( cut == new_cut ) { add = false; break; }

    /* cut subsumes new_cut */
    if ( ( cut & new_cut ) == cut ) { add = false; break; }

    /* new_cut subsumes cut */
    if ( ( cut & new_cut ) == new_cut )
    {
      add = false;
      if ( first_subsume )
      {
        local_cuts[l] = std::make_tuple( new_cut, min_level, new_cone );
        first_subsume = false;
      }
      else
      {
        local_cuts[l] = local_cuts.back();
        local_cuts.pop_back();
      }
    }

    ++l;
  }

  if ( add )
  {
    local_cuts += std::make_tuple( new_cut, min_level, new_cone );
  }
}

xmg_cuts_paged::local_cut_vec_t xmg_cuts_paged::enumerate_local_cuts( xmg_node n1, xmg_node n2, unsigned max_cut_size )
{
  local_cut_vec_t local_cuts;

  for ( const auto& c1 : boost::combine( cuts( n1 ), cut_cones( n1 ) ) )
  {
    for ( const auto& c2 : boost::combine( cuts( n2 ), cut_cones( n2 ) ) )
    {
      auto min_level = std::numeric_limits<unsigned>::max();
      boost::dynamic_bitset<> new_cut( max_cut_size );
      auto f = [&new_cut, &min_level, this]( unsigned pos ) {
        new_cut.set( pos );
        min_level = std::min( min_level, this->_levels[pos].second );
      };
      std::for_each( boost::get<0>( c1 ).begin(), boost::get<0>( c1 ).end(), f );
      std::for_each( boost::get<0>( c2 ).begin(), boost::get<0>( c2 ).end(), f );

      boost::dynamic_bitset<> new_cone( max_cut_size );
      auto f2 = [&new_cone]( unsigned pos ) {
        new_cone.set( pos );
      };
      std::for_each( boost::get<1>( c1 ).begin(), boost::get<1>( c1 ).end(), f2 );
      std::for_each( boost::get<1>( c2 ).begin(), boost::get<1>( c2 ).end(), f2 );

      merge_cut( local_cuts, new_cut, min_level, new_cone );
    }
  }

  return local_cuts;
}

xmg_cuts_paged::local_cut_vec_t xmg_cuts_paged::enumerate_local_cuts( xmg_node n1, xmg_node n2, xmg_node n3, unsigned max_cut_size )
{
  local_cut_vec_t local_cuts;

  for ( const auto& c1 : boost::combine( cuts( n1 ), cut_cones( n1 ) ) )
  {
    for ( const auto& c2 : boost::combine( cuts( n2 ), cut_cones( n2 ) ) )
    {
      for ( const auto& c3 : boost::combine( cuts( n3 ), cut_cones( n3 ) ) )
      {
        auto min_level = std::numeric_limits<unsigned>::max();
        boost::dynamic_bitset<> new_cut( max_cut_size );
        auto f = [&new_cut, &min_level, this]( unsigned pos ) {
          new_cut.set( pos );
          min_level = std::min( min_level, this->_levels[pos].second );
        };
        std::for_each( boost::get<0>( c1 ).begin(), boost::get<0>( c1 ).end(), f );
        std::for_each( boost::get<0>( c2 ).begin(), boost::get<0>( c2 ).end(), f );
        std::for_each( boost::get<0>( c3 ).begin(), boost::get<0>( c3 ).end(), f );

        boost::dynamic_bitset<> new_cone( max_cut_size );
        auto f2 = [&new_cone]( unsigned pos ) {
          new_cone.set( pos );
        };
        std::for_each( boost::get<1>( c1 ).begin(), boost::get<1>( c1 ).end(), f2 );
        std::for_each( boost::get<1>( c2 ).begin(), boost::get<1>( c2 ).end(), f2 );
        std::for_each( boost::get<1>( c3 ).begin(), boost::get<1>( c3 ).end(), f2 );

        merge_cut( local_cuts, new_cut, min_level, new_cone );
      }
    }
  }

  return local_cuts;
}

xmg_cuts_paged::local_cut_vec_t xmg_cuts_paged::enumerate_local_cuts( const std::vector<xmg_node>& ns, unsigned max_cut_size )
{
  local_cut_vec_t local_cuts;

  if ( ns.size() == 2u )
  {
    local_cuts = enumerate_local_cuts( ns[0u], ns[1u], max_cut_size );
  }
  else if ( ns.size() == 3u )
  {
    local_cuts = enumerate_local_cuts( ns[0u], ns[1u], ns[2u], max_cut_size );
  }
  else
  {
    assert( false );
  }

  boost::sort( local_cuts, []( const std::tuple<boost::dynamic_bitset<>, unsigned, boost::dynamic_bitset<>>& e1,
                               const std::tuple<boost::dynamic_bitset<>, unsigned, boost::dynamic_bitset<>>& e2 ) {
                 return ( std::get<1>( e1 ) > std::get<1>( e2 ) ) || ( std::get<1>( e1 ) == std::get<1>( e2 ) && std::get<0>( e1 ).count() < std::get<0>( e2 ).count() ); } );

  if ( local_cuts.size() > _priority )
  {
    local_cuts.resize( _priority );
  }

  return local_cuts;
}

void xmg_cuts_paged::enumerate_node_with_bitsets( xmg_node n, const std::vector<xmg_node>& ns )
{
  for ( const auto& cut : enumerate_local_cuts( ns, _top_index ) )
  {
    auto area = std::get<2>( cut );
    area.resize( n + 1 );
    area.set( n );
    const auto extra = get_extra( _levels[n].first - std::get<1>( cut ), static_cast<unsigned int>( area.count() ) );
    data.append_set( n, get_index_vector( std::get<0>( cut ) ), extra );
    cones.append_set( n, get_index_vector( area ) );
  }
}

std::vector<unsigned> xmg_cuts_paged::get_extra( unsigned depth, unsigned size ) const
{
  std::vector<unsigned> v( 2u + _extra, 0u );
  v[0u] = depth;
  v[1u] = size;
  return v;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
