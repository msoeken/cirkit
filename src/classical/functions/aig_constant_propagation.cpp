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

#include "aig_constant_propagation.hpp"

#include <boost/assign/std/vector.hpp>

#include <core/utils/timer.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/utils/aig_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using node_set_t     = std::set<aig_node>;
using edge_set_t     = std::set<aig_edge>;
using filter_graph_t = boost::filtered_graph<aig_graph, boost::is_not_in_subset<edge_set_t>, boost::is_not_in_subset<node_set_t>>;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_constant_propagation_simulator : public aig_simulator<aig_function>
{
public:
  aig_constant_propagation_simulator( aig_graph& aig_new, const std::map<std::string, aig_function>& values )
    : aig_new( aig_new ),
      values( values )
  {
  }

  aig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    return values.at( name );
  }

  aig_function get_constant() const
  {
    return aig_get_constant( aig_new, false );
  }

  aig_function invert( const aig_function& v ) const
  {
    return !v;
  }

  aig_function and_op( const aig_node& node, const aig_function& v1, const aig_function& v2 ) const
  {
    return aig_create_and( aig_new, v1, v2 );
  }

private:
  aig_graph& aig_new;
  const std::map<std::string, aig_function>& values;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph aig_constant_propagation( const aig_graph& aig, const std::map<std::string, bool>& values,
                                    const properties::ptr& settings, const properties::ptr& statistics )
{
  /* settings */

  /* run-time */
  properties_timer t( statistics );

  aig_graph aig_new;
  aig_initialize( aig_new );

  auto& info_new   = aig_info( aig_new );
  const auto& info = aig_info( aig );

  /* copy original PIs (except for those with constants) */
  std::map<std::string, aig_function> name_to_pi;
  for ( auto pi : info.inputs )
  {
    const auto& name = info.node_names.at( pi );
    const auto it = values.find( name );
    if ( it == values.end() )
    {
      name_to_pi[name] = aig_create_pi( aig_new, name );
    }
    else
    {
      name_to_pi[name] = aig_get_constant( aig_new, it->second );
    }
  }

  /* copy other info */
  info_new.model_name = info.model_name;

  auto result = simulate_aig( aig, aig_constant_propagation_simulator( aig_new, name_to_pi ) );

  for ( const auto& output : info.outputs )
  {
    aig_create_po( aig_new, result[output.first], output.second );
  }

  return aig_new;
}

aig_graph aig_constant_propagation( const aig_graph& aig, const std::map<unsigned, bool>& values,
                                    const properties::ptr& settings,
                                    const properties::ptr& statistics )
{
  const auto& info = aig_info( aig );
  std::map<std::string, bool> svalues;

  for ( const auto& p : values )
  {
    svalues.insert( {info.node_names.at( info.inputs[p.first] ), p.second} );
  }

  return aig_constant_propagation( aig, svalues, settings, statistics );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
