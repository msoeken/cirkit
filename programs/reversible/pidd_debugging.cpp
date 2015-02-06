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

#include <boost/format.hpp>

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/utils/reversible_program_options.hpp>
#include <reversible/verification/pidd_debugging.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string specname;
  auto negative = false;

  reversible_program_options opts;
  opts.add_read_realization_option();
  opts.add_options()
    ( "specname",   value( &specname ),              "Specification" )
    ( "negative,n", value_with_default( &negative ), "Allow negative control lines" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "specname" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  circuit circ;
  read_realization( circ, opts.read_realization_filename() );

  binary_truth_table spec;
  read_specification( spec, specname );

  auto settings = std::make_shared<properties>();
  settings->set( "with_negated", negative );
  auto statistics = std::make_shared<properties>();
  auto result = pidd_debugging( circ, spec, settings, statistics );

  std::cout << "[i] circuit and spec are" << ( result ? " " : " not " ) << "almost equal" << std::endl;
  std::cout << boost::format( "[i] run-time: %.2f secs" ) % statistics->get<double>( "runtime" ) << std::endl;

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
