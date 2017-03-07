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

#include "aig_from_cirkit_bdd.hpp"

#include <functional>
#include <map>
#include <vector>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/utils/timer.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

aig_function aig_from_bdd_rec( aig_graph& aig, const bdd& node,
                               std::map<unsigned, aig_function>& visited,
                               bool complement_optimization )
{
  auto it = visited.find( node.index );
  if ( it != visited.end() )
  {
    return it->second;
  }

  if ( complement_optimization )
  {
    auto comp = !node;
    it = visited.find( comp.index );
    if ( it != visited.end() )
    {
      return {it->second.node, !it->second.complemented};
    }
  }

  const auto& info = aig_info( aig );

  auto index = node.var();

  auto f_true  = aig_from_bdd_rec( aig, node.high(), visited, complement_optimization );
  auto f_false = aig_from_bdd_rec( aig, node.low(), visited, complement_optimization );

  auto func = aig_create_ite( aig, {info.inputs[index], false}, f_true, f_false );
  visited.insert( {node.index, func} );

  return func;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

std::vector<aig_function> aig_from_bdd( aig_graph& aig, const std::vector<bdd>& fs,
                                        const properties::ptr& settings, const properties::ptr& statistics )
{
  using namespace std::placeholders;

  assert( !fs.empty() );

  /* settings */
  auto input_labels            = get( settings, "input_labels", std::vector<std::string>() );
  auto complement_optimization = get( settings, "complement_optimization", false );

  /* run-time */
  properties_timer t( statistics );

  const auto& info = aig_info( aig );

  auto n = fs.front().manager->num_vars();
  auto num_pis = info.inputs.size();

  for ( auto i = num_pis; i < n; ++i )
  {
    aig_create_pi( aig, i < input_labels.size() ? input_labels.at( i ) : boost::str( boost::format( "x%d" ) % i ) );
  }

  std::map<unsigned, aig_function> visited = { {0u, aig_get_constant( aig, false )}, {1u, aig_get_constant( aig, true )} };

  std::vector<aig_function> result( fs.size() );
  boost::transform( fs, result.begin(), std::bind( aig_from_bdd_rec, std::ref( aig ), _1, std::ref( visited ), complement_optimization ) );
  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
