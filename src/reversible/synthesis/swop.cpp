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

#include "swop.hpp"

#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include "transformation_based_synthesis.hpp"

namespace cirkit
{

  bool swop( circuit& circ, const binary_truth_table& spec,
             properties::ptr settings,
             properties::ptr statistics )
  {
    bool enable                      = get( settings, "enable",        true );
    bool exhaustive                  = get( settings, "exhaustive",    false );
    truth_table_synthesis_func synth = get( settings, "synthesis",     truth_table_synthesis_func( transformation_based_synthesis_func() ) );
    cost_function cf                 = get( settings, "cost_function", cost_function( costs_by_circuit_func( gate_costs() ) ) );
    swop_step_func stepfunc          = get( settings, "stepfunc",      swop_step_func( swop_step_func() ) );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    /* copy truth table since we want to change it (permutation) */
    binary_truth_table spec2 = spec;

    clear_circuit( circ );

    if ( exhaustive )
    {
      do
      {
        circuit tmp;
        bool r = synth( tmp, spec2 );
        if ( r && ( !circ.num_gates() || costs( tmp, cf ) < costs( circ, cf ) ) )
        {
          clear_circuit( circ );
          copy_circuit( tmp, circ );
        }

        if ( stepfunc )
        {
          stepfunc();
        }
      } while ( enable && spec2.permute() );
    }
    else
    {
      std::vector<unsigned> perm( spec2.num_outputs() );
      boost::copy( boost::irange( 0u, spec2.num_outputs() ), perm.begin() );
      std::vector<unsigned> best_perm = perm;

      if ( enable )
      {
        unsigned min_costs = 0;

        for ( unsigned i = 0; i < ( spec2.num_outputs() - 1 ); ++i )
        {
          std::vector<unsigned>::iterator itCurrent = std::find( perm.begin(), perm.end(), i );
          std::vector<unsigned>::iterator itNext;
          assert( itCurrent != perm.end() );

          unsigned best_position = itCurrent - perm.begin();

          do
          {
            circuit tmp;
            spec2.set_permutation( perm );
            bool r = synth( tmp, spec2 );
            unsigned current_costs = costs( tmp, cf );
            if ( r && (min_costs == 0 || current_costs < min_costs) )
            {
              min_costs = current_costs;
              best_position = itCurrent - perm.begin();
              best_perm = perm;
            }

            itNext = std::find_if( itCurrent + 1, perm.end(), [&itCurrent]( unsigned j ) { return j > *itCurrent; } );

            if ( itNext != perm.end() )
            {
              unsigned tmp = *itCurrent;
              *itCurrent = *itNext;
              *itNext = tmp;

              itCurrent = itNext;
            }

            if ( stepfunc )
            {
              stepfunc();
            }
          }
          while ( itNext != perm.end() );

          perm.erase( std::find( perm.begin(), perm.end(), i ) );
          perm.insert( perm.begin() + best_position, i );
        }
      }

      spec2.set_permutation( best_perm );
      bool r = synth( circ, spec2 );

      if (!r)
      {
        set_error_message (statistics, synth.statistics()->get<std::string>("error"));
        return false;
      }

      if ( stepfunc )
      {
        stepfunc();
      }
    }

    return true;
  }

  truth_table_synthesis_func swop_func( properties::ptr settings, properties::ptr statistics )
  {
    truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
      return swop( circ, spec, settings, statistics );
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
