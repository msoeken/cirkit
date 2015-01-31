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

#include "transposition_based_synthesis.hpp"

#include <limits.h>

#include <iostream>
#include <iterator>   // for std::distance
#include <string>

#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>

#include <core/functor.hpp>
#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/gate.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/transposition_to_circuit.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/print_circuit.hpp>

#include "synthesis_utils_p.hpp"

using namespace boost::assign;

namespace cirkit
{

  bool transposition_based_synthesis( circuit& circ, const binary_truth_table& spec,
      properties::ptr settings, properties::ptr statistics )
  {
    // Settings parsing
    // Run-time measuring
    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    unsigned bw = spec.num_outputs();
    circ.set_lines( bw );
    copy_metadata( spec, circ );

    std::map<unsigned, unsigned> values_map;

    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      binary_truth_table::cube_type in( it->first.first, it->first.second );
      binary_truth_table::cube_type out( it->second.first, it->second.second );

      values_map.insert( std::make_pair( truth_table_cube_to_number( in ), truth_table_cube_to_number( out ) ) );
    }

    // Set of cycles
    std::vector<std::vector<unsigned> > cycles;

    while ( !values_map.empty() )
    {
      unsigned start_value = values_map.begin()->first; // first key in values_map

      std::vector<unsigned> cycle;
      unsigned target = start_value;

      do
      {
        cycle += target;
        unsigned old_target = target;
        target = values_map[target];
        values_map.erase( old_target ); // erase this entry
      } while ( target != start_value );

      cycles += cycle;
    }

    for ( auto& cycle : cycles )
    {
      unsigned max_distance = 0u;
      unsigned max_index = 0u;

      for ( unsigned i = 0u; i < cycle.size(); ++i )
      {
        unsigned first = cycle.at(i);
        unsigned second = cycle.at( (i + 1u) % cycle.size() );

        unsigned distance = hamming_distance( first, second );

        if ( distance > max_distance )
        {
          max_distance = distance;
          max_index = i;
        }
      }

      std::vector<unsigned> tmp;
      std::copy( cycle.begin() + ( max_index + 1u ), cycle.end(), std::back_inserter( tmp ) );
      std::copy( cycle.begin(), cycle.begin() + ( max_index + 1u ), std::back_inserter( tmp ) );
      std::copy( tmp.begin(), tmp.end(), cycle.begin() );
      std::reverse( cycle.begin(), cycle.end() );
    }

    // TODO create transpositions function
    for ( auto& cycle : cycles )
    {
      for ( unsigned i = 0u; i < cycle.size() - 1; ++i )
      {
        circuit transposition_circ( spec.num_inputs() );

        unsigned first = cycle.at(i);
        unsigned second = cycle.at( (i + 1) % cycle.size() );

        boost::dynamic_bitset<> first_bits( spec.num_inputs(), first );
        boost::dynamic_bitset<> second_bits( spec.num_inputs(), second );

        transposition_to_circuit( transposition_circ, first_bits, second_bits );

        append_circuit( circ, transposition_circ);
      }
    }

    return true;
  }

  truth_table_synthesis_func transposition_based_synthesis_func( properties::ptr settings, properties::ptr statistics )
  {
    truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
      return transposition_based_synthesis( circ, spec, settings, statistics );
    };
    f.init( settings, statistics );
    return f;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
