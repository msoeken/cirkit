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

#include "mig_cuts_paged.hpp"

#include <map>
#include <mutex>
#include <stack>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/mig/mig_simulate.hpp>
#include <classical/mig/mig_utils.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/numeric.hpp>

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

std::vector<std::pair<unsigned, unsigned>> compute_level_ranges( const mig_graph& mig, unsigned& max_level )
{
  auto sa_settings = std::make_shared<properties>();
  auto sa_statistics = std::make_shared<properties>();

  auto output_levels = simulate_mig( mig, mig_depth_simulator(), sa_settings, sa_statistics );

  max_level = 0u;
  const auto& info = mig_info( mig );
  for ( const auto& o : info.outputs )
  {
    max_level = std::max( max_level, output_levels.at( o.first ) );
  }

  auto levels = sa_statistics->get<std::map<mig_node, unsigned>>( "node_values" );
  std::vector<std::pair<unsigned, unsigned>> level_ranges( num_vertices( mig ) );

  std::vector<mig_node> topsort( boost::num_vertices( mig ) );
  boost::topological_sort( mig, topsort.begin() );

  auto ingoing = precompute_ingoing_edges( mig );

  for ( const auto& v : topsort )
  {
    auto it = ingoing.find( v );

    /* no ingoing edges (outputs) */
    if ( it == ingoing.end() )
    {
      level_ranges[v] = {levels[v], max_level};
      levels[v] = max_level;
      continue;
    }

    const auto min_edge = *boost::min_element( it->second, [&]( const mig_edge& e1, const mig_edge& e2 ) {
        return levels.at( boost::source( e1, mig ) ) < levels.at( boost::source( e2, mig ) );
      } );
    auto to_l = levels.at( boost::source( min_edge, mig ) ) - 1u;
    level_ranges[v] = {levels[v], to_l};
    levels[v] = to_l;
  }

  return level_ranges;
}

mig_cuts_paged::mig_cuts_paged( const mig_graph& mig, unsigned k, unsigned priority )
  : _mig( mig ),
    _k( k ),
    _priority( priority ),
    data( num_vertices( _mig ), 2u ),
    cones( num_vertices( _mig ) )
{
  unsigned max_level;
  _levels = compute_level_ranges( mig, max_level );

  enumerate();
}

mig_cuts_paged::mig_cuts_paged( const mig_graph& mig, unsigned k, const std::vector<mig_node>& start, const std::vector<mig_node>& boundary,
                                const std::vector<std::pair<unsigned, unsigned>>& levels, unsigned priority )
  : _mig( mig ),
    _k( k ),
    _priority( priority ),
    data( num_vertices( _mig ), 2u ),
    cones( num_vertices( _mig ) ),
    _levels( levels )
{
  enumerate_partial( start, boundary );
}

unsigned mig_cuts_paged::total_cut_count() const
{
  return data.sets_count();
}

double mig_cuts_paged::enumeration_time() const
{
  return _enumeration_time;
}

unsigned mig_cuts_paged::memory() const
{
  return data.memory() + cones.memory();
}

unsigned mig_cuts_paged::count( mig_node node ) const
{
  return data.count( node );
}

boost::iterator_range<paged_memory::iterator> mig_cuts_paged::cuts( mig_node node )
{
  return data.sets( node );
}

boost::iterator_range<paged_memory::iterator> mig_cuts_paged::cut_cones( mig_node node )
{
  return cones.sets( node );
}

tt mig_cuts_paged::simulate( mig_node node, const mig_cuts_paged::cut& c ) const
{
  std::map<mig_node, tt> inputs;
  auto i = 0u;
  for ( const auto& child : c )
  {
    inputs[child] = tt_nth_var( i++ );
  }

  mig_tt_simulator tt_sim;
  mig_partial_node_assignment_simulator<tt> sim( tt_sim, inputs, tt_const0() );

  return simulate_mig_node( _mig, node, sim );
}

unsigned mig_cuts_paged::depth( mig_node node, const mig_cuts_paged::cut& c ) const
{
  return c.extra( 0u );
}

unsigned mig_cuts_paged::size( mig_node node, const mig_cuts_paged::cut& c ) const
{
  return c.extra( 1u );
}

void mig_cuts_paged::enumerate()
{
  reference_timer t( &_enumeration_time );

  /* topsort */
  std::vector<unsigned> topsort( num_vertices( _mig ) );
  boost::topological_sort( _mig, topsort.begin() );

  /* loop */
  _top_index = 0u;
  for ( auto n : topsort )
  {
    if ( out_degree( n, _mig ) == 0u )
    {
      /* constant */
      if ( n == 0u )
      {
        data.assign_empty( 0u, {0u, 0u} );
        cones.assign_empty( 0u );
      }
      /* PI */
      else
      {
        data.assign_singleton( n, n, {0u, 1u} );
        cones.assign_singleton( n, n );
      }
    }
    else
    {
      data.append_begin( n );
      cones.append_begin( n );

      /* get children */
      auto it = adjacent_vertices( n, _mig ).first;
      const auto n1 = *it++;
      const auto n2 = *it++;
      const auto n3 = *it++;

      enumerate_node_with_bitsets( n, n1, n2, n3 );

      data.append_singleton( n, n, {0u, 1u} );
      cones.append_singleton( n, n );
    }

    _top_index++;
  }
}

void mig_cuts_paged::enumerate_partial( const std::vector<mig_node>& start, const std::vector<mig_node>& boundary )
{
  reference_timer t( &_enumeration_time );

  std::vector<mig_node> colors( num_vertices( _mig ), 0u );

  /* children */
  data.assign_empty( 0u, {0u, 0u} );
  cones.assign_empty( 0u );
  colors[0u] = 2u;

  for ( auto n : boundary )
  {
    if ( n == 0 ) { continue; }
    data.assign_singleton( n, n, {0u, 1u} );
    cones.assign_singleton( n, n );
    colors[n] = 2u;
  }

  /* create order */
  std::vector<mig_node> topo;
  std::stack<mig_node> stack;
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
        auto it = adjacent_vertices( n, _mig ).first;
        const auto n1 = *it++;
        const auto n2 = *it++;
        const auto n3 = *it++;

        colors[n] = 1u;
        stack.push( n3 );
        stack.push( n2 );
        stack.push( n1 );
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

  //std::cout << "[i] start: " << any_join( start, ", " ) << ", boundary: " << any_join( boundary, ", " ) << ", topo: " << any_join( topo, ", " ) << std::endl;

  for ( auto n : topo )
  {
    data.append_begin( n );
    cones.append_begin( n );

    /* get children */
    auto it = adjacent_vertices( n, _mig ).first;
    const auto n1 = *it++;
    const auto n2 = *it++;
    const auto n3 = *it++;

    _top_index = num_vertices( _mig );
    enumerate_node_with_bitsets( n, n1, n2, n3 );

    data.append_singleton( n, n, {0u, 1u} );
    cones.append_singleton( n, n );
  }
}

std::vector<std::tuple<boost::dynamic_bitset<>, unsigned, boost::dynamic_bitset<>>> mig_cuts_paged::enumerate_local_cuts( mig_node n1, mig_node n2, mig_node n3, unsigned max_cut_size )
{
  std::vector<std::tuple<boost::dynamic_bitset<>, unsigned, boost::dynamic_bitset<>>> local_cuts;

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

        /* too large? */
        if ( new_cut.count() > _k ) { continue; }

        auto first_subsume = true;
        auto add = true;

        auto l = 0u;
        while ( l < local_cuts.size() )
        {
          auto cut = std::get<0>( local_cuts[l] );

          /* same cut */
          if ( cut == new_cut ) { add = false; break; }

          /* cut subsumes new_cut */
          if ( ( cut & new_cut ) == new_cut ) { add = false; break; }

          /* new_cut subsumes cut */
          if ( ( cut & new_cut ) == cut )
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
    }
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

void mig_cuts_paged::enumerate_node_with_bitsets( mig_node n, mig_node n1, mig_node n2, mig_node n3 )
{
  for ( const auto& cut : enumerate_local_cuts( n1, n2, n3, _top_index ) )
  {
    auto area = std::get<2>( cut );
    area.resize( n + 1 );
    area.set( n );
    data.append_set( n, get_index_vector( std::get<0>( cut ) ), {_levels[n].first - std::get<1>( cut ), static_cast<unsigned int>( area.count() )} );
    cones.append_set( n, get_index_vector( area ) );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
