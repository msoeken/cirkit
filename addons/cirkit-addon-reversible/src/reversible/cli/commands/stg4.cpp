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

#include "stg4.hpp"

#include <alice/rules.hpp>

#include <classical/utils/truth_table_utils.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/rewrite_circuit.hpp>
#include <reversible/synthesis/optimal_quantum_circuits.hpp>

namespace cirkit
{

stg4_command::stg4_command( const environment::ptr& env )
  : cirkit_command( env, "Replace STG gates by optimal Clifford+T networks" )
{
  add_new_option();
}

command::rules_t stg4_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool stg4_command::execute()
{
  auto& circuits = env->store<circuit>();

  auto ancilla = -1;

  const auto circ = rewrite_circuit( circuits.current(), {
      [&ancilla]( const gate& g, circuit& circ ) {
        if ( is_stg( g ) )
        {
          const auto& stg = boost::any_cast<stg_tag>( g.type() );
          const auto& f = stg.function;
          const auto num_vars = tt_num_vars( f );
          if ( num_vars >= 2u && num_vars <= 5u )
          {
            const auto it = optimal_quantum_circuits::spectral_classification_index[num_vars - 2u].find( f.to_ulong() );
            if ( it != optimal_quantum_circuits::spectral_classification_index[num_vars - 2u].end() )
            {
              const auto subcirc = circuit_from_string( optimal_quantum_circuits::spectral_classification[num_vars - 2u][it->second] );

              std::vector<unsigned> line_mapping;
              for ( const auto& c : g.controls() )
              {
                line_mapping.push_back( c.line() );
              }
              line_mapping.push_back( g.targets().front() );

              if ( subcirc.lines() > g.size() )
              {
                if ( ancilla == -1 )
                {
                  ancilla = circ.lines();
                  add_line_to_circuit( circ, "h", "h" );
                }
                line_mapping.push_back( ancilla );
              }

              append_circuit( circ, subcirc, {}, line_mapping );
              return true;
            }
          }
        }

        return false;
      }
    } );

  extend_if_new( circuits );
  circuits.current() = circ;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
