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

#include "stack.hpp"

#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/graph/topological_sort.hpp>

#include <core/utils/bitset_utils.hpp>
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

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::map<mig_node, structural_cut> stack_based_structural_cut_enumeration( const mig_graph& circ, unsigned k,
                                                                           const properties::ptr& settings,
                                                                           const properties::ptr& statistics )
{
  /* settings */
  const auto include_constant = get( settings, "include_constant", false );

  /* timer */
  properties_timer t( statistics );

  const auto n = num_vertices( circ );
  std::map<mig_node, structural_cut> cut_map;

  std::vector<mig_node> topsort( n );
  boost::topological_sort( circ, topsort.begin() );

  for ( auto node : topsort )
  {
    if ( node == 0u && !include_constant )
    {
      cut_map.insert( {0u, {boost::dynamic_bitset<>( n )}} );
    }
    else if ( out_degree( node, circ ) == 0u )
    {
      cut_map.insert( {node, {onehot_bitset( n, node )}} );
    }
    else
    {
      structural_cut cuts;
      const auto children = get_children( circ, node );
      for ( const auto& v1 : cut_map[children[0u].node] )
      {
        for ( const auto& v2 : cut_map[children[1u].node] )
        {
          auto cut = v1 | v2;
          if ( cut.count() > k ) { continue; }

          for ( const auto& v3 : cut_map[children[2u].node] )
          {
            cut |= v3;
            if ( cut.count() > k ) { continue; }
            cuts += cut;
          }
        }
      }

      cuts += onehot_bitset( n, node );
      cut_map.insert( {node, cuts} );
    }
  }

  return cut_map;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
