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

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  reversible_program_options opts;
  opts.add_read_realization_option();
  opts.add_options()
    ( "circuit,c",     "Prints the circuit" )
    ( "truth_table,t", "Prints truth table" )
    ( "statistics,s",  "Prints circuit statistics " )
    ( "image,i",       "Creates circuit image in LaTeX" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  circuit circ;
  read_realization( circ, opts.read_realization_filename() );

  if ( opts.is_set( "circuit" ) )
  {
    std::cout << circ << std::endl;
  }

  if ( opts.is_set( "truth_table" ) )
  {
    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, simple_simulation_func() );
    std::cout << spec << std::endl;
  }

  if ( opts.is_set( "statistics" ) )
  {
    print_statistics( circ );
  }

  if ( opts.is_set( "image" ) )
  {
    create_tikz_settings settings;
    create_image( std::cout, circ, settings );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
