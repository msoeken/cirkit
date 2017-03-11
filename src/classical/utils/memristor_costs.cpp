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

#include "memristor_costs.hpp"

#include <map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

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

std::pair<unsigned, unsigned> memristor_costs( const mig_graph& mig )
{
  auto max_level = 0u;
  auto levels = compute_levels( mig, max_level );

  std::map<unsigned, std::vector<mig_node>> level_to_nodes;
  for ( const auto& p : levels )
  {
    level_to_nodes[p.second].push_back( p.first );
  }

  const auto& complement = boost::get( boost::edge_complement, mig );

  auto current_max_size  = 0u;
  auto levels_with_complement = 0u;

  for ( auto l = 0u; l <= max_level; ++l )
  {
    auto size = 6u * level_to_nodes[l].size();
    // std::cout << "l: " << l << " " << level_to_nodes[l].size() << std::endl;
    for ( const auto& node : level_to_nodes[l] )
    {
      for ( const auto& edge : boost::make_iterator_range( boost::out_edges( node, mig ) ) )
      {
        if ( complement[edge] ) { ++size; }
      }
    }

    if ( size > current_max_size )
    {
      current_max_size = size;
    }

    if ( size > 6u * level_to_nodes[l].size() ) { ++levels_with_complement; }
  }

  // std::cout << "[i] current_max_size:       " << current_max_size << std::endl
  //           << "[i] levels_with_complement: " << levels_with_complement << std::endl
  //           << "[i] max_level:              " << max_level << std::endl;

  auto operations = max_level * 10u + levels_with_complement;

  return {current_max_size, operations};
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
