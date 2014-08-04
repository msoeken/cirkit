/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "window_optimization.hpp"

#include <core/utils/timer.hpp>

#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/expand_circuit.hpp>
#include <reversible/functions/find_lines.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>

namespace cirkit
{

  shift_window_selection::shift_window_selection()
    : window_length( 10u ),
      offset( 1u ),
      pos( 0u )
  {
  }

  circuit_filter_pair shift_window_selection::operator()( const circuit& base )
  {
    if ( pos >= base.num_gates() )
    {
      /* dont forget to reset in case of second call */
      pos = 0u;
      return circuit();
    }

    unsigned length = std::min( window_length, base.num_gates() - pos );
    unsigned to = pos + length;

    subcircuit s( base, pos, to );
    pos += offset;

    return circuit_filter_pair( s, std::vector<unsigned>() );
  }

  line_window_selection::line_window_selection()
    : num_lines( 0u ),
      line_count( 2u ),
      pos( 0u )
  {
  }

  circuit_filter_pair line_window_selection::operator()( const circuit& base )
  {
    /* set number of lines */
    if ( !num_lines )
    {
      num_lines = base.lines();
    }

    while ( true )
    {
      /* increment line_count or reset and finish */
      if ( pos >= base.num_gates() )
      {
        pos = 0u;

        if ( line_count < num_lines - 1u )
        {
          ++line_count;
        }
        else
        {
          line_count = 2u;
          return circuit();
        }
      }

      unsigned start_pos = pos;
      std::set<unsigned> non_empty_lines;

      for ( unsigned i = pos; i < base.num_gates(); ++i )
      {
        std::set<unsigned> new_non_empty_lines;

        /* copy controls and targets */
        const gate& g = *( base.begin() + i );
        find_non_empty_lines( g, std::insert_iterator<std::set<unsigned> >( new_non_empty_lines, new_non_empty_lines.begin() ) );
        std::copy( non_empty_lines.begin(), non_empty_lines.end(), std::insert_iterator<std::set<unsigned> >( new_non_empty_lines, new_non_empty_lines.begin() ) );

        if ( new_non_empty_lines.size() <= line_count )
        {
          /* keep on trying */
          non_empty_lines.clear();
          std::copy( new_non_empty_lines.begin(), new_non_empty_lines.end(), std::insert_iterator<std::set<unsigned> >( non_empty_lines, non_empty_lines.begin() ) );
        }
        else
        {
          /* do we have a circuit for now? */
          if ( non_empty_lines.size() )
          {
            pos = i;
            std::vector<unsigned> filter( non_empty_lines.begin(), non_empty_lines.end() );

            circuit ret_circuit;
            copy_circuit( subcircuit( base, start_pos, pos ), ret_circuit, filter );
            return circuit_filter_pair( ret_circuit, filter );
          }
          else
          {
            /* new start pos */
            start_pos = i + 1;
          }
        }
      }

      /* still here? */
      pos = base.num_gates();

      /* do we have a circuit? */
      if ( non_empty_lines.size() )
      {
        std::vector<unsigned> filter( non_empty_lines.begin(), non_empty_lines.end() );
        circuit ret_circuit;
        copy_circuit( subcircuit( base, start_pos, base.num_gates() ), ret_circuit, filter );
        return circuit_filter_pair( ret_circuit, filter );
      }
      else
      {
        /* next iteration */
      }
    }
  }

  resynthesis_optimization::resynthesis_optimization()
    : synthesis( transformation_based_synthesis_func() ),
      simulation( simple_simulation_func() )
  {
  }

  bool resynthesis_optimization::operator()( circuit& new_window, const circuit& old_window ) const
  {
    binary_truth_table spec;
    circuit_to_truth_table( old_window, spec, simulation );
    return synthesis( new_window, spec );
  }

  bool window_optimization( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {
    select_window_func select_window = get<select_window_func>( settings, "select_window", shift_window_selection() );
    optimization_func  optimization  = get<optimization_func>( settings, "optimization", resynthesis_optimization() );
    cost_function cf = get<cost_function>( settings, "cost_function", costs_by_circuit_func( gate_costs() ) );

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    copy_circuit( base, circ );

    while ( true )
    {
      /* select the window */
      circuit s;
      std::vector<unsigned> filter;
      boost::tie( s, filter ) = select_window( circ );

      /* check if window is still valid */
      if ( s.num_gates() == 0 )
      {
        break;
      }

      /* obtain the new window */
      circuit new_window;
      bool ok = optimization( new_window, s );

      /* check if it is cheaper */
      bool cheaper = ok && costs( new_window, cf ) < costs( s, cf );

      if ( cheaper )
      {
        /* remove old sub-circuit */
        unsigned s_size = s.num_gates(); // save in variable since we are changing its base
        unsigned s_from = s.offset();
        for ( unsigned i = 0u; i < s_size; ++i )
        {
          circ.remove_gate_at( s_from );
        }

        circuit window_expanded;
        expand_circuit( new_window, window_expanded, circ.lines(), filter );
        insert_circuit( circ, s_from, window_expanded );
      }
    }

    return true;
  }

  optimization_func window_optimization_func( properties::ptr settings, properties::ptr statistics )
  {
    optimization_func f = [&settings, &statistics]( circuit& circ, const circuit& base ) {
      return window_optimization( circ, base, settings, statistics );
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
