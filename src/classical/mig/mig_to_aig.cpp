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

#include "mig_to_aig.hpp"

#include <classical/mig/mig_simulate.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class mig_to_aig_simulator : public mig_simulator<aig_function>
{
public:
  mig_to_aig_simulator( aig_graph& aig ) : aig( aig ), info( aig_info( aig ) ) {}

  aig_function get_input( const mig_node& node, const std::string& name, unsigned pos, const mig_graph& mig ) const
  {
    return { info.inputs.at( pos ), false };
  }

  aig_function get_constant() const
  {
    return aig_get_constant( aig, false );
  }

  aig_function invert( const aig_function& v ) const
  {
    return !v;
  }

  aig_function maj_op( const mig_node& node, const aig_function& v1, const aig_function& v2, const aig_function& v3 ) const
  {
    return aig_create_maj( aig, v1, v2, v3 );
  }

private:
  aig_graph& aig;
  const aig_graph_info& info;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

aig_graph mig_to_aig( const mig_graph& mig,
                      const properties::ptr& settings,
                      const properties::ptr& statistics )
{
  aig_graph aig;
  aig_initialize( aig );

  auto& info_aig   = aig_info( aig );
  const auto& info = mig_info( mig );

  /* copy inputs */
  for ( const auto& input : info.inputs )
  {
    aig_create_pi( aig, info.node_names.at( input ) );
  }

  /* copy other info */
  info_aig.model_name = info.model_name;

  auto result = simulate_mig( mig, mig_to_aig_simulator( aig ) );

  for ( const auto& output : info.outputs )
  {
    aig_create_po( aig, result[output.first], output.second );
  }

  return aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
