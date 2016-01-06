/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/optimization/window_optimization.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  reversible_program_options opts;

  opts.add_read_realization_option();
  opts.add_write_realization_option();
  opts.add_costs_option();
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_write_realization_filename_set() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  circuit circ, opt;
  read_realization( circ, opts.read_realization_filename() );

  auto settings = std::make_shared<properties>();
  auto statistics = std::make_shared<properties>();
  settings->set( "cost_function", opts.costs() );
  window_optimization( opt, circ, settings, statistics );

  if ( opts.is_write_realization_filename_set() )
  {
    write_realization( opt, opts.write_realization_filename() );
  }

  print_statistics( opt, statistics->get<double>( "runtime" ) );

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
