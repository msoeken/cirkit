/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#if ADDON_FORMAL

#include <reversible/truth_table.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/read_specification.hpp>
#include <reversible/utils/reversible_program_options.hpp>
#include <reversible/synthesis/exact_synthesis.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  reversible_program_options opts;
  opts.add_read_specification_option();

  opts.parse( argc, argv );

  if ( !opts.good() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  binary_truth_table spec;
  read_specification( spec, opts.read_specification_filename() );

  circuit circ;
  exact_synthesis( circ, spec );

  std::cout << circ << std::endl;

  return 0;
}

#else

int main( int argc, char ** argv )
{
  std::cout << "[E] Addon `formal' is not installed." << std::endl;
  return 1;
}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
