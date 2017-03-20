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

#include "cone.hpp"

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <classical/functions/aig_cone.hpp>
#include <classical/utils/aig_utils.hpp>

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

cone_command::cone_command( const environment::ptr& env ) : aig_base_command( env, "Extracts cone by outputs" )
{
  opts.add_options()
    ( "output,o",      value( &outputs )->composing(),        "names of outputs that should be kept" )
    ( "ouput_index,i", value( &output_indexes )->composing(), "indexes of outputs that should be kept" )
    ;
  be_verbose();
}

command::rules_t cone_command::validity_rules() const
{
  return {
    {[&]() { return !outputs.empty() || !output_indexes.empty(); }, "no output name specified" },
  };
}

bool cone_command::execute()
{
  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();

  for ( const auto& name : outputs )
  {
    const auto id = aig_output_index( aig(), name );
    if ( std::find( output_indexes.begin(), output_indexes.end(), id ) == output_indexes.end() )
    {
      output_indexes.push_back( id );
    }
  }

  aig() = aig_cone( aig(), output_indexes, settings, statistics );
  std::cout << boost::format( "[i] Run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
