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
