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

#include "mig_rewrite.hpp"

#include <boost/graph/topological_sort.hpp>

#include <core/utils/timer.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::map<mig_node, mig_function> init_visited_table( const mig_graph& mig, mig_graph& mig_new )
{
  std::map<mig_node, mig_function> old_to_new;

  old_to_new[0] = mig_get_constant( mig_new, false );
  for ( const auto& pi : mig_info( mig ).inputs )
  {
    old_to_new[pi] = mig_create_pi( mig_new, mig_info( mig ).node_names.at( pi ) );
  }

  return old_to_new;
}

mig_function mig_rewrite_top_down_rec( const mig_graph& mig, mig_node node,
                                       mig_graph& mig_new,
                                       const mig_maj_rewrite_func_t& on_maj,
                                       std::map<mig_node, mig_function>& old_to_new )
{
  /* visited */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() )
  {
    return it->second;
  }

  const auto c = get_children( mig, node );
  const auto f = on_maj( mig_new,
                         mig_rewrite_top_down_rec( mig, c[0].node, mig_new, on_maj, old_to_new ) ^ c[0].complemented,
                         mig_rewrite_top_down_rec( mig, c[1].node, mig_new, on_maj, old_to_new ) ^ c[1].complemented,
                         mig_rewrite_top_down_rec( mig, c[2].node, mig_new, on_maj, old_to_new ) ^ c[2].complemented );

  old_to_new.insert( {node, f} );
  return f;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_function mig_rewrite_default_maj( mig_graph& mig_new, const mig_function& a, const mig_function& b, const mig_function& c )
{
  return mig_create_maj( mig_new, a, b, c );
}

mig_graph mig_rewrite_top_down( const mig_graph& mig,
                                const mig_maj_rewrite_func_t& on_maj,
                                const properties::ptr& settings,
                                const properties::ptr& statistics )
{
  /* settings */
  const auto prefill = get( settings, "prefill", prefill_func_t() );

  /* statistics */
  properties_timer t( statistics );

  mig_graph mig_new;
  mig_initialize( mig_new, mig_info( mig ).model_name );

  /* create constant and PIs */
  auto old_to_new = init_visited_table( mig, mig_new );

  /* prefill */
  if ( prefill )
  {
    prefill( mig_new, old_to_new );
  }

  /* map nodes */
  for ( const auto& po : mig_info( mig ).outputs )
  {
    mig_create_po( mig_new, mig_rewrite_top_down_rec( mig, po.first.node, mig_new, on_maj, old_to_new ) ^ po.first.complemented, po.second );
  }

  return mig_new;
}

mig_graph mig_rewrite_bottom_up( const mig_graph& mig,
                                 const mig_maj_rewrite_func_t& on_maj,
                                 const properties::ptr& settings,
                                 const properties::ptr& statistics )
{
  /* statistics */
  properties_timer t( statistics );

  mig_graph mig_new;
  mig_initialize( mig_new, mig_info( mig ).model_name );

  /* create constant and PIs */
  auto old_to_new = init_visited_table( mig, mig_new );

  /* map nodes */
  std::vector<mig_node> top( num_vertices( mig ) );
  boost::topological_sort( mig, top.begin() );
  for ( auto node : top )
  {
    if ( out_degree( node, mig ) == 0u ) { continue; }

    const auto c = get_children( mig, node );
    old_to_new[node] = on_maj( mig_new,
                               old_to_new[c[0].node] ^ c[0].complemented,
                               old_to_new[c[1].node] ^ c[1].complemented,
                               old_to_new[c[2].node] ^ c[2].complemented );
  }

  for ( const auto& po : mig_info( mig ).outputs )
  {
    mig_create_po( mig_new, old_to_new[po.first.node] ^ po.first.complemented, po.second );
  }

  return mig_new;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
