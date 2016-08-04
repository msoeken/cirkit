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

#include "xmg_mffc.hpp"

#include <classical/xmg/xmg_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/* inspired by: https://bitbucket.org/alanmi/abc/src/9f0e7e81524337aeebec196723c23915c7354982/src/aig/gia/giaMffc.c?at=default&fileviewer=file-view-default */

unsigned xmg_mffc_node_deref( xmg_graph& xmg, xmg_node n )
{
  auto counter = 0u;

  if ( xmg.is_input( n ) )
  {
    return 0;
  }

  for ( auto child : xmg.children( n ) )
  {
    assert( xmg.get_ref( child.node ) > 0u );
    if ( xmg.dec_ref( child.node ) == 0u )
    {
      counter += xmg_mffc_node_deref( xmg, child.node );
    }
  }

  return counter + 1u;
}

unsigned xmg_mffc_node_ref( xmg_graph& xmg, xmg_node n )
{
  auto counter = 0u;

  if ( xmg.is_input( n ) )
  {
    return 0;
  }

  for ( auto child : xmg.children( n ) )
  {
    if ( xmg.inc_ref( child.node ) == 0u )
    {
      counter += xmg_mffc_node_ref( xmg, child.node );
    }
  }

  return counter + 1u;
}

void xmg_mffc_node_collect( xmg_graph& xmg, xmg_node n, std::vector<xmg_node>& support )
{
  if ( xmg.is_marked( n ) ) return;
  xmg.mark( n );

  if ( xmg.get_ref( n ) > 0u || xmg.is_input( n ) )
  {
    support.push_back( n );
    return;
  }

  for ( const auto& child : xmg.children( n ) )
  {
    xmg_mffc_node_collect( xmg, child.node, support );
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unsigned xmg_compute_mffc( xmg_graph& xmg, xmg_node n, std::vector<xmg_node>& support )
{
  assert( !xmg.is_input( n ) );

  xmg.init_refs();
  xmg.inc_output_refs();
  xmg.init_marks();

  const auto size1 = xmg_mffc_node_deref( xmg, n );
  for ( const auto& child : xmg.children( n ) )
  {
    xmg_mffc_node_collect( xmg, child.node, support );
  }
  const auto size2 = xmg_mffc_node_ref( xmg, n );

  assert( size1 == size2 );

  return size1;
}

std::map<xmg_node, std::vector<xmg_node>> xmg_mffcs( xmg_graph& xmg )
{
  std::map<xmg_node, std::vector<xmg_node>> map;

  auto nodes = xmg_output_deque( xmg );

  while ( !nodes.empty() )
  {
    const auto n = nodes.front();
    nodes.pop_front();

    if ( map.find( n ) != map.end() || xmg.is_input( n ) ) { continue; }

    std::vector<xmg_node> support;
    xmg_compute_mffc( xmg, n, support );
    map[n] = support;

    for ( auto s : support )
    {
      nodes.push_back( s );
    }
  }

  return map;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
