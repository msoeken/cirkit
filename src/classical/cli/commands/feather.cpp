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

#include "feather.hpp"

#include <core/utils/program_options.hpp>
#include <classical/functions/feathering.hpp>

using namespace boost::program_options;

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

feather_command::feather_command( const environment::ptr& env ) : aig_base_command( env, "Adds outputs to higher levels of a circuit" )
{
  opts.add_options()
    ( "levels,l",        value_with_default( &levels ),        "Number of levels to feather" )
    ( "respect_edges,r", value_with_default( &respect_edges ), "If true, then for each node within the feathering level, "
                                                               "an output is added according to the polarities of outgoing"
                                                               "edges.  Otherwise, always two outputs are created for each"
                                                               "node, if not existing." )
    ( "output_name,o",   value_with_default( &output_name ),   "Template for name of the new outputs" )
    ;
  be_verbose();
}

bool feather_command::execute()
{
  auto settings = make_settings();
  settings->set( "respect_edges", respect_edges );
  settings->set( "output_name",   output_name );
  aig() = output_feathering( aig(), levels, settings );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
