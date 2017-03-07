/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
