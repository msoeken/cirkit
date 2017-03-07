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

#include "aig_to_mig.hpp"

#include <classical/functions/simulate_aig.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_to_mig_simulator : public aig_simulator<mig_function>
{
public:
  aig_to_mig_simulator( mig_graph& mig ) : mig( mig ), info( mig_info( mig ) ) {}

  mig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    return { info.inputs.at( pos ), false };
  }

  mig_function get_constant() const
  {
    return mig_get_constant( mig, false );
  }

  mig_function invert( const mig_function& v ) const
  {
    return !v;
  }

  mig_function and_op( const aig_node& node, const mig_function& v1, const mig_function& v2 ) const
  {
    return mig_create_and( mig, v1, v2 );
  }

private:
  mig_graph& mig;
  const mig_graph_info& info;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

mig_graph aig_to_mig( const aig_graph& aig,
                      const properties::ptr& settings,
                      const properties::ptr& statistics )
{
  mig_graph mig;
  mig_initialize( mig );

  auto& info_mig   = mig_info( mig );
  const auto& info = aig_info( aig );

  /* copy inputs */
  for ( const auto& input : info.inputs )
  {
    mig_create_pi( mig, info.node_names.at( input ) );
  }

  /* copy other info */
  info_mig.model_name = info.model_name;

  auto result = simulate_aig( aig, aig_to_mig_simulator( mig ) );

  for ( const auto& output : info.outputs )
  {
    mig_create_po( mig, result[output.first], output.second );
  }

  return mig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
