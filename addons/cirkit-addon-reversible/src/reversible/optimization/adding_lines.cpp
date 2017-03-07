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

#include "adding_lines.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <core/functor.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/utils/costs.hpp>

using namespace boost::assign;

namespace cirkit
{
  gate::control_container make_factor( const gate::control_container& controls, const boost::dynamic_bitset<>& factor )
  {
    gate::control_container factored;
    for ( auto it : index( controls ) )
    {
      if ( factor.test( it.index ) )
      {
        factored += it.value;
      }
    }
    return factored;
  }

  unsigned find_suitable_gates( const circuit& base, unsigned index, const gate::control_container& factor )
  {
    using boost::adaptors::transformed;

    // last gate?
    if ( index == base.num_gates() )
    {
      return index;
    }

    // only Toffoli gates
    if ( !is_toffoli( base[index] ) )
    {
      return index;
    }

    // NOTE check for helper line?
    // has target in controls?
    auto container = factor | transformed( +[]( variable v ) { return v.line(); } );
    if ( boost::find( container, base[index].targets().front() ) != boost::end( container ) )
    {
      return index;
    }

    // check next gate
    return find_suitable_gates( base, index + 1, factor );
  }

  int calculate_cost_reduction( const circuit& base, unsigned start, unsigned end, const gate::control_container& factor, unsigned helper_line, const cost_function& cf )
  {
    circuit tmp;
    copy_metadata( base, tmp );

    /* original costs */
    for ( circuit::const_iterator it = base.begin() + start; it != base.begin() + end; ++it )
    {
      tmp.append_gate() = *it;
    }

    unsigned original_costs = costs( tmp, cf );

    /* modify circuit */
    for ( auto& g : tmp )
    {
      if ( !boost::includes( g.controls(), factor ) ) continue;

      g.add_control( make_var( helper_line ) );
      for ( const auto& v : factor )
      {
        g.remove_control( v );
      }
    }
    prepend_toffoli( tmp, factor, helper_line );
    append_toffoli( tmp, factor, helper_line );

    unsigned new_costs = costs( tmp, cf );

    return original_costs - new_costs;
  }

  bool adding_lines( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {

    // Settings parsing
    unsigned additional_lines = get<unsigned>( settings, "additional_lines", 1 );
    cost_function cf = get<cost_function>( settings, "cost_function", costs_by_gate_func( transistor_costs() ) );

    // Run-time measuring
    properties_timer t( statistics );

    // copy circuit
    copy_circuit( base, circ );

    for ( unsigned h = 0u; h < additional_lines; ++h )
    {
      /* add one helper line */
      unsigned helper_line = add_line_to_circuit( circ, "helper", "helper", false, true );

      /* last inserted helper gate (to be removed in the end) */
      unsigned last_helper_gate_index = 0u;

      unsigned current_index = 0u;
      while ( current_index < circ.num_gates() )
      {
        /* best factor */
        unsigned best_cost_reduction = 0u;
        unsigned best_factor = 0u;
        unsigned best_j = 0u;

        /* control lines of the current gate */
        const gate& current_gate = circ[current_index];

        /* generate each factor */
        for ( unsigned F : boost::irange( 1u, 1u << current_gate.controls().size() ) )
        {
          boost::dynamic_bitset<> factor( current_gate.controls().size(), F );
          if ( factor.count() <= 1u ) continue; /* only consider factors of size > 1 */

          /* create factor */
          gate::control_container factored = make_factor( current_gate.controls(), factor );

          /* determine upper bound */
          unsigned j = find_suitable_gates( circ, current_index, factored );

          /* calculate cost reduction */
          int cost_reduction = calculate_cost_reduction( circ, current_index, j, factored, helper_line, cf );

          /* better? */
          if ( cost_reduction > (int)best_cost_reduction )
          {
            best_cost_reduction = (unsigned)cost_reduction; /* in this case cost_reduction is greater than 0 */
            best_factor = F;
            best_j = j;
          }
        }

        /* was a factor found? */
        if ( best_factor != 0u )
        {
          gate::control_container factored = make_factor( current_gate.controls(), boost::dynamic_bitset<>( current_gate.controls().size(), best_factor) );

          /* apply factor */
          for ( unsigned i : boost::irange( current_index, best_j ) )
          {
            if ( !boost::includes( circ[i].controls(), factored ) ) continue;

            circ[i].add_control( make_var( helper_line ) );
            for ( const auto& control : factored )
            {
              circ[i].remove_control( control );
            }
          }

          /* toffoli gate at the beginning */
          insert_toffoli( circ, current_index, factored, helper_line );

          /* update best_j, since we inserted a gate before */
          ++best_j;

          /* toffoli gate at the end */
          insert_toffoli( circ, best_j, factored, helper_line );
          last_helper_gate_index = best_j;

          /* update again */
          ++best_j;

          /* update current_index */
          current_index = best_j;
        }
        else
        {
          /* check next gate */
          ++current_index;
        }
      }

      /* remove last helper gate if added */
      if ( last_helper_gate_index != 0u )
      {
        circ.remove_gate_at( last_helper_gate_index );
      }
    }

    return true;
  }

  optimization_func adding_lines_func( properties::ptr settings, properties::ptr statistics )
  {
    optimization_func f = [&settings, &statistics]( circuit& circ, const circuit& base ) {
      return adding_lines( circ, base, settings, statistics );
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
