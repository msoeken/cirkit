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
 * @author Oliver Keszöcze
 * @since  2.0
 */

#include <iostream>

#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>
#include <reversible/synthesis/quantified_exact_synthesis.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  unsigned mode = 0u;
  unsigned max_depth = 20u;

  reversible_program_options opts;
  opts.add_read_specification_option();
  opts.add_write_realization_option();
  opts.add_options()
    ( "mode",                value_with_default( &mode ),      "Mode (0: BDD, 1: SAT)" )
    ( "max_depth",           value_with_default( &max_depth ), "Maximum search depth" )
    ( "print_circuit,c",                                       "Prints the circuit" )
    ( "print_truth_table,t",                                   "Prints the truth table of the circuit" )
    ( "negative,n",                                            "Allow negative control lines" )
    ( "multiple,m",                                            "Allow multiple target lines (only with SAT)" )
    ( "all_solutions,a",                                       "Extract all solutions (only with BDD)" )
    ( "verbose,v",                                             "Be verbose" )
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
  properties::ptr settings = std::make_shared<properties>();
  settings->set( "max_depth",     max_depth );
  settings->set( "negative",      opts.is_set( "negative" ) );
  settings->set( "multiple",      opts.is_set( "multiple" ) );
  settings->set( "all_solutions", opts.is_set( "all_solutions" ) );
  settings->set( "verbose",       opts.is_set( "verbose" ) );
  properties::ptr statistics = std::make_shared<properties>();

  auto result = false;

  if ( mode == 0u )
  {
    result = quantified_exact_synthesis( circ, spec, settings, statistics );
  }
  else if ( mode == 1u )
  {
    result = exact_synthesis( circ, spec, settings, statistics );
  }
  else
  {
    std::cout << "[e] mode " << mode << " invalid." << std::endl;
    return 1;
  }

  if ( opts.is_set( "print_circuit" ) )
  {
    std::cout << circ << std::endl;
  }

  if ( opts.is_set( "print_truth_table" ) )
  {
    binary_truth_table spec2;
    circuit_to_truth_table( circ, spec2, simple_simulation_func() );
    std::cout << spec2 << std::endl;
  }

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( circ, opts.write_realization_filename() );
  }

  print_statistics( circ, statistics->get<double>( "runtime" ) );

  if ( mode == 0u && result )
  {
    std::cout << "Solutions:    " << statistics->get<unsigned>( "num_circuits" ) << std::endl;
  }

  if ( mode == 0u && result && opts.is_set( "all_solutions" ) && opts.is_set( "print_circuit" ) )
  {
    for ( const auto& sol : statistics->get<std::vector<circuit>>( "solutions" ) )
    {
      std::cout << sol << std::endl;
    }
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
