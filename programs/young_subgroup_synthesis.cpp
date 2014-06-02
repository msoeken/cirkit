/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <core/truth_table.hpp>
#include <core/functions/circuit_to_truth_table.hpp>
#include <core/io/print_circuit.hpp>
#include <core/io/read_specification.hpp>
#include <core/io/write_realization.hpp>
#include <core/utils/program_options.hpp>

#include <algorithms/simulation/simple_simulation.hpp>
#include <algorithms/synthesis/young_subgroup_synthesis.hpp>

using namespace revkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  bool print_circuit     = false;
  bool print_truth_table = false;
  bool verbose           = false;

  program_options opts;
  opts.add_read_specification_option();
  opts.add_write_realization_option();
  opts.add_options()
    ( "print_circuit",     value<bool>( &print_circuit )->default_value( false ),     "Prints the circuit" )
    ( "print_truth_table", value<bool>( &print_truth_table )->default_value( false ), "Prints the truth table of the circuit" )
    ( "verbose",           value<bool>( &verbose )->default_value( false ),           "Be verbose" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  binary_truth_table spec;
  read_specification( spec, opts.read_specification_filename() );

  circuit circ;
  properties::ptr settings( new properties );
  settings->set( "verbose", verbose );
  young_subgroup_synthesis( circ, spec, settings );

  if ( print_circuit )
  {
    std::cout << circ << std::endl;
  }

  if ( print_truth_table )
  {
    binary_truth_table spec2;
    circuit_to_truth_table( circ, spec2, simple_simulation_func() );
    std::cout << spec2 << std::endl;
  }

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( circ, opts.write_realization_filename() );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:
