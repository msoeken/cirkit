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

#include "print_statistics.hpp"

#include <fstream>

#include <boost/format.hpp>

#include "../utils/costs.hpp"

namespace cirkit
{

  bool has_fully_controlled_gate( const circuit& circ )
  {
    const auto n = circ.lines();
    for ( const auto& g : circ )
    {
      if ( g.controls().size() + g.targets().size() == n )
      {
        return true;
      }
    }
    return false;
  }

  unsigned number_of_qubits( const circuit& circ )
  {
    const auto n = circ.lines();
    return has_fully_controlled_gate( circ ) ? n + 1 : n;
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
      % costs( circ, costs_by_gate_func( ncv_quantum_costs() ) )
      % costs( circ, costs_by_gate_func( t_depth_costs() ) )
      % costs( circ, costs_by_gate_func( t_costs() ) )
      % costs( circ, costs_by_gate_func( h_costs() ) )
      % number_of_qubits( circ )
      % costs( circ, costs_by_gate_func( transistor_costs() ) )
      % costs( circ, costs_by_gate_func( sk2013_quantum_costs() ) );
  }

  void print_statistics( const std::string& filename, const circuit& circ, double runtime, const print_statistics_settings& settings )
  {
    std::filebuf fb;
    fb.open( filename.c_str(), std::ios::out );
    std::ostream os( &fb );
    print_statistics( os, circ, runtime, settings );
    fb.close();
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
