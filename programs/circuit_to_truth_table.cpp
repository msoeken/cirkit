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

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/write_specification.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/utils/program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string specname;

  program_options opts;
  opts.add_read_realization_option();
  opts.add_options()
    ( "specname", value<std::string>( &specname ), "SPEC filename" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "specname" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  circuit circ;
  binary_truth_table spec;

  read_realization( circ, opts.read_realization_filename() );
  circuit_to_truth_table( circ, spec, simple_simulation_func() );
  write_specification( spec, specname );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// End:

