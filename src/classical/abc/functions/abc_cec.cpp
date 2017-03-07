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

#include "abc_cec.hpp"

#include <classical/abc/abc_api.hpp>
#include <classical/abc/abc_manager.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/utils/aig_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <boost/format.hpp>

namespace cirkit
{


class simple_node_assignment_resimulator : public aig_simulator<bool>
{
public:
  using aig_node_value_map = std::unordered_map<aig_node, bool>;

  simple_node_assignment_resimulator( aig_node_value_map& assignment );

  bool get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool and_op( const aig_node& node, const bool& v1, const bool& v2 ) const;
  bool terminate( const aig_node& node, const aig_graph& aig ) const;

private:
  aig_node_value_map& assignment;
};

/******************************************************************************
 * Boolean simulation (on nodes)                                              *
 ******************************************************************************/

simple_node_assignment_resimulator::simple_node_assignment_resimulator( aig_node_value_map& assignment ) : assignment( assignment ) {}

bool simple_node_assignment_resimulator::get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
{
  auto it = assignment.find( node );
  if ( it == assignment.end() )
  {
    std::cout << "[w] no assignment given for '" << node << "', assume 0" << std::endl;
    return false;
  }
  else
  {
    return it->second;
  }
}

bool simple_node_assignment_resimulator::get_constant() const
{
  return false;
}

bool simple_node_assignment_resimulator::invert( const bool& v ) const
{
  return !v;
}

bool simple_node_assignment_resimulator::and_op( const aig_node& node, const bool& v1, const bool& v2 ) const
{
  auto it = assignment.find( node );
  if ( it == assignment.end() )
  {
    const auto v = v1 && v2;
    assignment.insert( { node, v } );
    return v;
  }
  else
  {
    return it->second;
  }
}

bool simple_node_assignment_resimulator::terminate( const aig_node& node, const aig_graph& aig ) const
{
  return assignment.find( node ) != assignment.end();
}

simple_node_assignment_resimulator::aig_node_value_map node_value_map_from_counterexample( const aig_graph& aig, const abc_counterexample_t& cec_counterexample )
{
  /* do not count node 0 and start with index 0 */
  simple_node_assignment_resimulator::aig_node_value_map node_value_map( boost::num_vertices( aig ) - 2u );
  node_value_map[ 0u ] = 0u;
  for ( auto i = 1u; i <= aig_info( aig ).inputs.size(); ++i )
  {
    node_value_map[ i ] = cec_counterexample[ cec_counterexample.size() - i ];
  }
  return node_value_map;
}

boost::optional< counterexample_t > abc_cec( const aig_graph& circuit, const aig_graph& spec,
                                             properties::ptr settings, properties::ptr statistics )
{
  const auto& circuit_info = aig_info( circuit );
  const auto& spec_info = aig_info( spec );
  const auto num_inputs = circuit_info.inputs.size();
  const auto num_outputs = circuit_info.outputs.size();
  assert( num_inputs == spec_info.inputs.size() );
  assert( num_outputs == spec_info.outputs.size() );

  auto abc = abc_manager::get();

  /* Timer */
  properties_timer t( statistics );

  auto cec_result = abc->cec( circuit, spec );
  assert( cec_result.first == 0 );

  if ( cec_result.second )
  {
    const auto cec_counterexample = *cec_result.second;

    /*** counterexample: all internal nodes|outputs|expected_outputs ***/
    const auto _num_vertices = boost::num_vertices( circuit );

    /* re-simulate the counterexample */
    auto node_value_map = node_value_map_from_counterexample( circuit, cec_counterexample );
    simple_node_assignment_resimulator sim( node_value_map );
    auto result = simulate_aig_full( circuit, sim, settings );

    counterexample_t cex( _num_vertices - 1u, num_outputs );

    /* internal nodes */
    for ( const auto& node : boost::make_iterator_range( vertices( circuit ) ) )
    {
      if ( node == 0u ) { continue; }
      cex.in.bits[ node - 1u ] = node_value_map[ node ];
      cex.in.mask[ node - 1u ] = 1u;
    }

    /* circuit outputs */
    for ( const auto& po : index( circuit_info.outputs ) )
    {
      cex.out.bits[ po.index ] = po.value.first.complemented ? !node_value_map[ po.value.first.node ] : node_value_map[ po.value.first.node ];
      cex.out.mask[ po.index ] = 1u;
    }

    /* spec outputs */
    node_value_map = node_value_map_from_counterexample( spec, cec_counterexample );
    result = simulate_aig( spec, sim, settings );
    for ( const auto& po : index( spec_info.outputs ) )
    {
      cex.expected_out.bits[ po.index ] = po.value.first.complemented ? !node_value_map[ po.value.first.node ] : node_value_map[ po.value.first.node ];
      cex.expected_out.mask[ po.index ] = 1u;
    }

    return boost::optional< counterexample_t >( cex );
  }
  else
  {
    return boost::optional< counterexample_t >();
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
