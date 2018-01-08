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

#include "stg_as.hpp"

#include <alice/rules.hpp>
#include <cli/stores.hpp>
#include <cli/reversible_stores.hpp>

#include <classical/functions/spectral_canonization.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_stg.hpp>
#include <reversible/functions/rewrite_circuit.hpp>
#include <reversible/synthesis/optimal_quantum_circuits.hpp>
#include <reversible/utils/circuit_utils.hpp>

namespace cirkit
{

stg_as_command::stg_as_command( const environment::ptr& env )
  : cirkit_command( env, "Realize single-target from truth table" )
{
  add_option( "--func,-f", func, "id of truth table to realize", true );
  add_option( "--as,-a", real, "id of truth table to use in gate", true );
  add_flag( "--circuit,-c", "perform on single-target gates with affine annotations in circuit" );
  add_flag( "--class", "automatically compute affine classes if not annotated (only with option circuit)" );
  add_new_option();
}

command::rules stg_as_command::validity_rules() const
{
  return {
    {[this]() { return is_set( "circuit" ) || func < env->store<tt>().size(); }, "func id is invalid"},
    {[this]() { return is_set( "circuit" ) || real < env->store<tt>().size(); }, "as id is invalid"},
    has_store_element_if_set<circuit>( *this, env, "circuit" )
  };
}

void stg_as_command::execute()
{
  const auto& tts = env->store<tt>();
  auto& circuits = env->store<circuit>();

  if ( is_set( "circuit" ) )
  {
    const auto circ = rewrite_circuit( circuits.current(), {
        [this]( const gate& g, circuit& circ ) {
          std::string affine;

          if ( is_stg( g ) )
          {
            const auto& stg = boost::any_cast<stg_tag>( g.type() );
            const auto num_vars = g.controls().size();

            boost::dynamic_bitset<> cls = stg.affine_class;
            if ( cls.empty() )
            {
              if ( is_set( "class" ) && num_vars >= 2u && num_vars <= 5u )
              {
                const auto idx = get_spectral_class( stg.function );
                cls = tt( 1 << num_vars, optimal_quantum_circuits::spectral_classification_representative[num_vars - 2u][idx] );
              }
              else
              {
                return false;
              }
            }

            add_stg_as_other( circ, stg.function, cls, get_line_map( g ) );
            return true;
          }
          return false;
        }
      } );
    extend_if_new( circuits );
    circuits.current() = circ;
  }
  else
  {
    extend_if_new( circuits );

    circuit circ( tt_num_vars( tts[func] ) + 1u );
    add_stg_as_other( circ, tts[func], tts[real] );
    circuits.current() = circ;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
