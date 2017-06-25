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

#include "print_statistics.hpp"

#include <fstream>

#include <boost/format.hpp>

#include <reversible/utils/circuit_utils.hpp>
#include <reversible/utils/costs.hpp>

namespace cirkit
{

std::string format_costs( cost_t cost )
{
  if ( cost == cost_invalid() )
  {
    return "N/A";
  }
  else
  {
    return std::to_string( cost );
  }
}

void print_statistics( std::ostream& os, const circuit& circ, double runtime, const print_statistics_settings& settings )
{
  std::string runtime_string;

  if ( runtime != -1 )
  {
    runtime_string = boost::str( boost::format( settings.runtime_template ) % runtime );
  }

  boost::format fmt( settings.main_template );
  fmt.exceptions( boost::io::all_error_bits ^ ( boost::io::too_many_args_bit | boost::io::too_few_args_bit ) );

  os << fmt
    % runtime_string
    % circ.lines()
    % circ.num_gates()
    % format_costs( costs( circ, costs_by_gate_func( ncv_quantum_costs() ) ) )
    % format_costs( costs( circ, costs_by_gate_func( t_depth_costs() ) ) )
    % format_costs( costs( circ, costs_by_gate_func( t_costs() ) ) )
    % format_costs( costs( circ, costs_by_gate_func( h_costs() ) ) )
    % number_of_qubits( circ )
    % format_costs( costs( circ, costs_by_gate_func( transistor_costs() ) ) )
    % format_costs( costs( circ, costs_by_gate_func( sk2013_quantum_costs() ) ) );
}

void print_statistics( const std::string& filename, const circuit& circ, double runtime, const print_statistics_settings& settings )
{
  std::fstream os( filename.c_str(), std::fstream::out );
  print_statistics( os, circ, runtime, settings );
  os.close();
}

void print_statistics( const circuit& circ, double runtime, const print_statistics_settings& settings )
{
  print_statistics( std::cout, circ, runtime, settings );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
