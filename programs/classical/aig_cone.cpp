/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

/**
 * @author Mathias Soeken
 */

#include <string>
#include <vector>

#include <boost/format.hpp>

#include <core/properties.hpp>
#include <core/utils/program_options.hpp>

#include <classical/aig.hpp>
#include <classical/functions/aig_cone.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/write_aiger.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string              filename, aagname;
  std::vector<std::string> outputs;

  program_options opts;
  opts.add_options()
    ( "filename",  value( &filename ),             "Input circuit" )
    ( "output,o", value( &outputs )->composing(),  "Names of outputs that should be kept" )
    ( "aagname",   value( &aagname ),              "Output circuit" )
    ( "verbose,v",                                 "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* Create AIGs */
  aig_graph aig;
  read_aiger( aig, filename );

  auto settings = std::make_shared<properties>();
  settings->set( "verbose", opts.is_set( "verbose" ) );
  auto statistics = std::make_shared<properties>();
  auto aig_new = aig_cone( aig, outputs, settings, statistics );

  if ( opts.is_set( "verbose" ) )
  {
    std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;
  }

  if ( !aagname.empty() )
  {
    write_aiger( aig_new, aagname );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
