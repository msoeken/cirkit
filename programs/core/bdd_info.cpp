/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include <iostream>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  std::string ordering;

  program_options opts;
  opts.add_options()
    ( "filename",  value( &filename ), "PLA filename" )
    ( "ordering",  value( &ordering ), "Complete variable ordering (space separated)" )
    ( "verbose,v",                     "Be verbose" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  read_pla_to_bdd_settings settings;
  parse_string_list( settings.ordering, ordering );

  BDDTable bdd;
  read_pla_to_bdd( bdd, filename, settings );

  std::cout << "Node count: " << Cudd_ReadNodeCount( bdd.cudd ) << std::endl;
  for ( const auto& p : bdd.outputs )
  {
    std::cout << "Info for output " << p.first << ":" << std::endl;
    std::cout << "- Path count:               " << Cudd_CountPath( p.second ) << std::endl;
    std::cout << "- Path count (to non-zero): " << Cudd_CountPathsToNonZero( p.second ) << std::endl;
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
