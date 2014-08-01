/* RevKit (www.rekit.org)
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

/**
 * @file simple_simulation.hpp
 *
 * @brief Very simple simulation, only efficient for small circuits
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef SIMPLE_SIMULATION_HPP
#define SIMPLE_SIMULATION_HPP

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <reversible/circuit.hpp>

#include <reversible/simulation/simulation.hpp>

namespace cirkit
{

  class gate;

  /**
   * @brief Functor for gate-wise simulation, used as a setting for \ref revkit::simple_simulation "simple_simulation"
   *
   * For more information, see the concrete implementation \ref revkit::core_gate_simulation "core_gate_simulation"
   *
   * @since  1.0
   */
  typedef boost::function<boost::dynamic_bitset<>&( const gate&, boost::dynamic_bitset<>& )> gate_simulation_func;

  /**
   * @brief Functor which is called after a step result is calculated
   *
   * If set, this functor is called in \ref revkit::simple_simulation "simple_simulation"
   * after every gate simulation with the current gate and the calculated output pattern
   * as parameter.
   *
   * @since  1.0
   */
  typedef boost::function<void(const gate&, const boost::dynamic_bitset<>&)> step_result_func;

  /**
   * @brief A gate simulation implementation of \ref revkit::gate_simulation_func "gate_simulation_func"
   *
   * This functor performs simulation on a boost::dynamic_bitset<> for Toffoli, Fredkin, and Peres gates
   * for binary values only.
   *
   * When adding new simulation functors, you can make use of this one, for example when extending the
   * gate library. Note, that you do not have to derive from this class for this purpose, but can use the
   * following:
   *
   * @code
   * // somewhere a new target tag
   * struct my_gate_tag {};
   *
   * bool is_my_gate( const gate& g )
   * {
   *   return is_type<my_gate>( g.type() );
   * }
   *
   * // ...
   *
   * // extend simulation
   * struct extended_gate_simulation
   * {
   *
   *   boost::dynamic_bitset<>& operator()( const gate& g, boost::dynamic_bitset<>& input ) const
   *   {
   *     if ( is_my_gate( g )
   *     {
   *       // change input according to the semantics of the new gate type
   *       // ...
   *       // and return it finally
   *       return input;
   *     }
   *     else
   *     {
   *       // it is a core gate
   *       return core_simulation( g, input );
   *     }
   *   }
   *
   * private:
   *   core_gate_simulation core_simulation;
   * };
   *
   * // use it somewhere
   * circuit circ( ... );  // some circuit with my_gate gates
   *
   * boost::dynamic_bitset<> output;
   * boost::dynamic_bitset<> input( 4, 10 );   // value 1010
   *
   * properties::ptr settings( new properties() );
   * settings.set( "gate_simulation", extended_gate_simulation() );
   *
   * simple_simulation( output, circ, input, settings );
   * @endcode
   *
   * @since  1.0
   */
  struct core_gate_simulation
  {
    /**
     * @brief Simulation for a single gate \p g
     *
     * This operator performs simulation for a single gate and is called by
     * \ref revkit::simple_simulation "simple_simulation".
     *
     * \b Important: The return value always has to be \p input, and the
     * operator should modify \p input directly.
     *
     * @param g     The gate to be simulated
     * @param input An input pattern
     *
     * @return Returns a output pattern, it will be the same reference as \p input
     *
     * @since 1.0
     */
    boost::dynamic_bitset<>& operator()( const gate& g, boost::dynamic_bitset<>& input ) const;
  };

  /**
   * @brief Simple Simulation function for a single gate
   *
   * This method calls the \em gate_simulation setting's functor on
   * gate \p g using the input \p input and writes the result to
   * \p output. If a the \em step_result setting is set, it will
   * be called once, after simulating the single gate.
   *
   * @param output Output pattern. The index of the pattern corresponds to the line index.
   * @param g Gate to be simulated
   * @param input Input pattern. The index of the pattern corresponds to the line index.
   *              The bit-width of the input pattern has to be initialized properly to the
   *              number of lines.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">gate_simulation</td>
   *     <td class="indexvalue">\ref revkit::gate_simulation_func "gate_simulation_func"</td>
   *     <td class="indexvalue">\ref revkit::core_gate_simulation "core_gate_simulation()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The gate-wise simulation functor to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">step_result</td>
   *     <td class="indexvalue">\ref revkit::step_result_func "step_result_func"</td>
   *     <td class="indexvalue">\ref revkit::step_result_func "step_result_func()" <i>(empty)</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">A functor called with the last simulated gate and the last calculated output pattern.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.0
   */
  bool simple_simulation( boost::dynamic_bitset<>& output, const gate& g, const boost::dynamic_bitset<>& input,
                          properties::ptr settings = properties::ptr(),
                          properties::ptr statistics = properties::ptr() );

  /**
   * @brief Simple Simulation function for a range of gates
   *
   * This method calls the \em gate_simulation setting's functor on
   * the gate range from \p first to (exclusive) \p last. Thereby,
   * the last calculated output pattern serves as the input pattern
   * for the next gate. The last calculated output pattern is written
   * to \p output. If a the \em step_result setting is set, it will
   * be called after each gate simulation passing the gate as well
   * as the step result.
   *
   * @param output Output pattern. The index of the pattern corresponds to the line index.
   * @param first Iterator pointing to the first gate.
   * @param last Iterator pointing to the last gate (exclusive).
   * @param input Input pattern. The index of the pattern corresponds to the line index.
   *              The bit-width of the input pattern has to be initialized properly to the
   *              number of lines.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">gate_simulation</td>
   *     <td class="indexvalue">\ref revkit::gate_simulation_func "gate_simulation_func"</td>
   *     <td class="indexvalue">\ref revkit::core_gate_simulation "core_gate_simulation()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The gate-wise simulation functor to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">step_result</td>
   *     <td class="indexvalue">\ref revkit::step_result_func "step_result_func"</td>
   *     <td class="indexvalue">\ref revkit::step_result_func "step_result_func()" <i>(empty)</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">A functor called with the last simulated gate and the last calculated output pattern.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.0
   */
  bool simple_simulation( boost::dynamic_bitset<>& output, circuit::const_iterator first, circuit::const_iterator last, const boost::dynamic_bitset<>& input,
                          properties::ptr settings = properties::ptr(),
                          properties::ptr statistics = properties::ptr() );

  /**
   * @brief Simple Simulation function for a circuit
   *
   * This method calls the \em gate_simulation setting's functor on
   * all gates of the circuit \p circ. Thereby,
   * the last calculated output pattern serves as the input pattern
   * for the next gate. The last calculated output pattern is written
   * to \p output. If a the \em step_result setting is set, it will
   * be called after each gate simulation passing the gate as well
   * as the step result.
   *
   * @param output Output pattern. The index of the pattern corresponds to the line index.
   * @param circ Circuit to be simulated.
   * @param input Input pattern. The index of the pattern corresponds to the line index.
   *              The bit-width of the input pattern has to be initialized properly to the
   *              number of lines.
   * @param settings <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Setting</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Default Value</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">gate_simulation</td>
   *     <td class="indexvalue">\ref revkit::gate_simulation_func "gate_simulation_func"</td>
   *     <td class="indexvalue">\ref revkit::core_gate_simulation "core_gate_simulation()"</td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">The gate-wise simulation functor to use.</td>
   *   </tr>
   *   <tr>
   *     <td rowspan="2" class="indexvalue">step_result</td>
   *     <td class="indexvalue">\ref revkit::step_result_func "step_result_func"</td>
   *     <td class="indexvalue">\ref revkit::step_result_func "step_result_func()" <i>(empty)</i></td>
   *   </tr>
   *   <tr>
   *     <td colspan="2" class="indexvalue">A functor called with the last simulated gate and the last calculated output pattern.</td>
   *   </tr>
   * </table>
   * @param statistics <table border="0" width="100%">
   *   <tr>
   *     <td class="indexkey">Information</td>
   *     <td class="indexkey">Type</td>
   *     <td class="indexkey">Description</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">runtime</td>
   *     <td class="indexvalue">double</td>
   *     <td class="indexvalue">Run-time consumed by the algorithm in CPU seconds.</td>
   *   </tr>
   * </table>
   * @return true on success
   *
   * @since  1.0
   */
  bool simple_simulation( boost::dynamic_bitset<>& output, const circuit& circ, const boost::dynamic_bitset<>& input,
                          properties::ptr settings = properties::ptr(),
                          properties::ptr statistics = properties::ptr() );

  /**
   * @brief Functor for the \ref revkit::simple_simulation "simple_simulation" algorithm
   *
   * @param settings Settings (see \ref revkit::simple_simulation "simple_simulation")
   * @param statistics Statistics (see \ref revkit::simple_simulation "simple_simulation")
   *
   * @return A functor which complies with the \ref revkit::simulation_func "simulation_func" interface
   *
   * @since  1.0
   */
  simulation_func simple_simulation_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* SIMPLE_SIMULATION_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
