/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dsop.hpp"

#include <boost/format.hpp>

#include <core/properties.hpp>
#include <core/cli/rules.hpp>
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
