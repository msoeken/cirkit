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

#include "paged.hpp"

#include <map>
#include <mutex>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/compute_levels.hpp>
#include <classical/functions/parallel_compute.hpp>
#include <classical/functions/simulate_aig.hpp>

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>

using namespace boost::assign;

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

paged_aig_cuts::paged_aig_cuts( const aig_graph& aig, unsigned k, bool parallel, unsigned priority )
  : _aig( aig ),
    _k( k ),
    _priority( priority ),
    data( num_vertices( _aig ) )
{
  _levels = compute_levels( aig );

  if ( parallel )
  {
    enumerate_parallel();
  }
  else
  {
    enumerate();
  }
}

unsigned paged_aig_cuts::total_cut_count() const
{
  return data.sets_count();
}

double paged_aig_cuts::enumeration_time() const
{
  return _enumeration_time;
}

unsigned paged_aig_cuts::memory() const
{
  return data.memory();
}

unsigned paged_aig_cuts::count( aig_node node ) const
{
  return data.count( node );
}

boost::iterator_range<paged_memory::iterator> paged_aig_cuts::cuts( aig_node node ) const
{
  return data.sets( node );
}

tt paged_aig_cuts::simulate( aig_node node, const paged_aig_cuts::cut& c )
{
  std::map<aig_node, tt> inputs;
  auto i = 0u;
  for ( const auto& child : c )
  {
    inputs[child] = tt_nth_var( i++ );
  }

  tt_simulator tt_sim;
  aig_partial_node_assignment_simulator<tt> sim( tt_sim, inputs, tt_const0() );

  return simulate_aig_node( _aig, node, sim );
}

unsigned paged_aig_cuts::depth( aig_node node, const paged_aig_cuts::cut& c )
{
  std::map<aig_node, unsigned> inputs;
  auto i = 0u;
  for ( const auto& child : c )
  {
    inputs[child] = 0u;
  }

  depth_simulator depth_sim;
  aig_partial_node_assignment_simulator<unsigned> sim( depth_sim, inputs, 0u );

  return simulate_aig_node( _aig, node, sim );
}

void paged_aig_cuts::enumerate()
{
  reference_timer t( &_enumeration_time );

  /* topsort */
  std::vector<unsigned> topsort( num_vertices( _aig ) );
  boost::topological_sort( _aig, topsort.begin() );

  /* loop */
  _top_index = 0u;
  for ( auto n : topsort )
  {
    if ( out_degree( n, _aig ) == 0u )
    {
      /* constant */
      if ( n == 0u )
      {
        data.assign_empty( 0u );
      }
      /* PI */
      else
      {
        data.assign_singleton( n, n );
      }
    }
    else
    {
      data.append_begin( n );

      /* get children */
      auto it = adjacent_vertices( n, _aig ).first;
      const auto n1 = *it++;
      const auto n2 = *it;

      enumerate_node_with_bitsets( n, n1, n2 );

      data.append_singleton( n, n );
    }

    _top_index++;
  }
}

void paged_aig_cuts::enumerate_parallel()
{
  reference_timer t( &_enumeration_time );
  std::mutex mutex;

  /* constant */
  data.assign_empty( 0u );

  auto on_input = [this, &mutex]( aig_node n ) {
    mutex.lock();
    this->data.assign_singleton( n, n );
    mutex.unlock();
  };

  const auto size = boost::num_vertices( _aig );
  auto on_and = [this, &mutex, &size]( aig_node n, const aig_function& c1, const aig_function& c2 ) {
    const auto local_cuts = this->enumerate_local_cuts( c1.node, c2.node, size );

    mutex.lock();
    this->data.append_begin( n );
    for ( const auto& cut : local_cuts )
    {
      this->data.append_set( n, get_index_vector( cut.first ) );
    }

    this->data.append_singleton( n, n );
    mutex.unlock();
  };

  parallel_process( _aig, on_input, on_and );
}

std::vector<std::pair<boost::dynamic_bitset<>, unsigned>> paged_aig_cuts::enumerate_local_cuts( aig_node n1, aig_node n2, unsigned max_cut_size ) const
{
  std::vector<std::pair<boost::dynamic_bitset<>, unsigned>> local_cuts;

  for ( const auto& c1 : cuts( n1 ) )
  {
    for ( const auto& c2 : cuts( n2 ) )
    {
      auto min_level = std::numeric_limits<unsigned>::max();
      boost::dynamic_bitset<> new_cut( max_cut_size );
      auto f = [&new_cut, &min_level, this]( unsigned pos ) {
        new_cut.set( pos );
        min_level = std::min( min_level, this->_levels.at( pos ) );
      };
      std::for_each( c1.begin(), c1.end(), f );
      std::for_each( c2.begin(), c2.end(), f );

      const auto size = new_cut.count();

      if ( size <= _k )
      {
        auto first_subsume = true;
        auto add = true;

        auto l = 0u;
        while ( l < local_cuts.size() )
        {
          auto cut = local_cuts[l].first;

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
              local_cuts[l] = {new_cut, min_level};
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
          local_cuts += std::make_pair( new_cut, min_level );
        }
      }
    }
  }

  boost::sort( local_cuts, []( const std::pair<boost::dynamic_bitset<>, unsigned>& e1,
                               const std::pair<boost::dynamic_bitset<>, unsigned>& e2 ) {
                 return ( e1.second > e2.second ) || ( e1.second == e2.second && e1.first.count() < e2.first.count() ); } );

  if ( local_cuts.size() > _priority )
  {
    local_cuts.resize( _priority );
  }

  return local_cuts;
}

void paged_aig_cuts::enumerate_node_with_bitsets( aig_node n, aig_node n1, aig_node n2 )
{
  for ( const auto& cut : enumerate_local_cuts( n1, n2, _top_index ) )
  {
    data.append_set( n, get_index_vector( cut.first ) );
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
