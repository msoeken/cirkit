/* RevKit (www.revkit.org)
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

#include <chrono>
#include <random>

#include <boost/dynamic_bitset.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/io/write_specification.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace cirkit;

void create_random_gate( gate& g, unsigned lines, bool negative, std::default_random_engine& generator )
{
  std::uniform_int_distribution<unsigned> dist( 0u, lines - 1u );
  std::uniform_int_distribution<unsigned> bdist( 0u, 1u );

  auto controls = random_bitset( lines, generator );
  auto target   = dist( generator );

  g.set_type( toffoli_tag() );
  g.add_target( target );
  auto pos = controls.find_first();
  while ( pos != controls.npos )
  {
    if ( pos != target )
    {
      g.add_control( make_var( pos, negative ? ( bdist( generator ) == 1u ) : true ) );
    }
    pos = controls.find_next( pos );
  }
}

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  auto lines    = 4u;
  auto gates    = 10u;
  auto negative = false;
  std::string specname, specname_ae;
  unsigned seed;

  reversible_program_options opts;
  opts.add_write_realization_option();
  opts.add_options()
    ( "lines",        value_with_default( &lines ),    "Number of lines" )
    ( "gates",        value_with_default( &gates ),    "Number of gates" )
    ( "negative,n",   value_with_default( &negative ), "Allow negative control lines" )
    ( "specname",     value( &specname ),              "If set, writes truth table to that file" )
    ( "specname_ae",  value( &specname_ae ),           "If set, inserts a random gate to the circuit and writes its truth table to that file" )
    ( "seed",         value( &seed ),                  "Random seed (if not given, current time is used)" )
    ( "statistics,s",                                  "Print circuit statistics" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  if ( !opts.is_set( "seed" ) )
  {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
  }

  std::default_random_engine generator( seed );
  circuit circ( lines );
  for ( auto i = 0u; i < gates; ++i )
  {
    create_random_gate( circ.append_gate(), lines, negative, generator );
  }

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( circ, opts.write_realization_filename() );
  }

  if ( !specname.empty() )
  {
    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, simple_simulation_func() );
    write_specification( spec, specname );
  }

  if ( !specname_ae.empty() )
  {
    std::uniform_int_distribution<unsigned> dist( 0u, gates - 1u );
    create_random_gate( circ.insert_gate( dist( generator ) ), lines, negative, generator );

    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, simple_simulation_func() );
    write_specification( spec, specname_ae );
  }

  if ( opts.is_set( "statistics" ) )
  {
    print_statistics( circ );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
