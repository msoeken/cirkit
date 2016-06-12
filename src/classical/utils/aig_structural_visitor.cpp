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
 * @author Heinz Riener
 */

#include "aig_structural_visitor.hpp"

#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

void aig_visit_nodes( const aig_graph& aig, const aig_structural_visitor& vis )
{
  const auto& info = aig_info( aig );
  const auto& complementmap = boost::get( boost::edge_complement, aig );

  if ( aig_is_constant_used( aig ) )
  {
    vis.visit_constant( info.constant );
  }

  /* iterate through nodes in order */
  for ( const auto& node : boost::make_iterator_range( vertices( aig ) ) )
  {
    const auto _out_degree = out_degree( node, aig );

    /* input */
    if ( boost::find( info.inputs, node ) != info.inputs.end() )
    {
      vis.visit_input( node );
    }

    /* latches */
    if ( _out_degree == 1u )
    {
      vis.visit_latch( node );
    }
    /* and gates */
    else if ( _out_degree == 2u )
    {
      const auto& edges = boost::make_iterator_range( out_edges( node, aig ) );
      assert( edges.size() == 2u );
      aig_function l = { boost::target( edges[ 0u ], aig), complementmap[ edges[ 0u ] ] };
      aig_function r = { boost::target( edges[ 1u ], aig), complementmap[ edges[ 1u ] ] };
      vis.visit_and( node, l, r );
    }

    /* output */
    for ( const auto &o : info.outputs )
    {
      if ( o.first.node == node )
      {
        vis.visit_output( node );
      }
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
