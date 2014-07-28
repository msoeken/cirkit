/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2014  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/reversible_program_options.hpp>

#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/synthesis/young_subgroup_synthesis.hpp>

#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/adaptors.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string ordering;
  bool        print_circuit     = false;
  bool        print_truth_table = false;
  unsigned    esop_minimizer    = 0u;
  bool        verbose           = false;

  reversible_program_options opts;
  opts.add_read_specification_option();
  opts.add_write_realization_option();
  opts.add_options()
    ( "ordering",          value<std::string>( &ordering ),                           "Complete variable ordering (space separated)" )
    ( "print_circuit",     value<bool>( &print_circuit )->default_value( false ),     "Prints the circuit" )
    ( "print_truth_table", value<bool>( &print_truth_table )->default_value( false ), "Prints the truth table of the circuit" )
    ( "esop_minimizer",    value<unsigned>( &esop_minimizer )->default_value( 0u ),   "ESOP minizer (0: built-in, 1: exorcism)" )
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
  properties::ptr statistics( new properties );
  settings->set( "verbose", verbose );

  properties::ptr esopmin_settings( new properties );
  esopmin_settings->set( "verbose", verbose );
  settings->set( "esopmin", esop_minimizer ? dd_based_exorcism_minimization_func( esopmin_settings ) : dd_based_esop_minimization_func( esopmin_settings ) );

  if ( !ordering.empty() )
  {
    using boost::adaptors::transformed;

    std::vector<std::string> vsordering;
    boost::split( vsordering, ordering, boost::is_any_of( " " ) );
    std::vector<unsigned> vuordering;
    boost::push_back( vuordering, vsordering | transformed( []( const std::string& s ) { return boost::lexical_cast<unsigned>( s ); } ) );
    settings->set( "ordering", vuordering );
  }

  young_subgroup_synthesis( circ, spec, settings, statistics );

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

  print_statistics( circ, statistics->get<double>( "runtime" ) );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:
