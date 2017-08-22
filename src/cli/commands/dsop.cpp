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

#include "dsop.hpp"

#include <boost/format.hpp>

#include <alice/rules.hpp>

#include <core/properties.hpp>
#include <core/utils/program_options.hpp>
#include <classical/optimization/compact_dsop.hpp>

using namespace boost::program_options;
using boost::format;

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

dsop_command::dsop_command( const environment::ptr& env ) : cirkit_command( env, "Computes disjoint SOP" )
{
  add_positional_option( "filename" );
  opts.add_options()
    ( "filename,i", value( &filename ),              "PLA filename for SOP (input)" )
    ( "outname,o",  value( &outname ),               "PLA filename for DSOP (output)" )
    ( "optfunc",    value_with_default( &optfunc ),  "Optimization function: 1-5" )
    ( "sortfunc",   value_with_default( &sortfunc ), "Sort function:\n0: dimension first\n1: weight first" )
    ;
  be_verbose();
}

command::rules_t dsop_command::validity_rules() const
{
  return { file_exists( filename, "filename" ) };
}

bool dsop_command::execute()
{
  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();

  settings->set( "sortfunc", std::vector<sort_cube_meta_func_t>( {sort_by_dimension_first, sort_by_weight_first} )[sortfunc] );
  settings->set( "optfunc", std::vector<opt_cube_func_t>( {opt_dsop_1,opt_dsop_2,opt_dsop_3,opt_dsop_4,opt_dsop_5} )[optfunc - 1u] );
  compact_dsop( outname, filename, settings, statistics );

  std::cout << format( "[i] cubes:    %d       " ) % statistics->get<unsigned>( "cube_count" ) << std::endl;
  std::cout << format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
