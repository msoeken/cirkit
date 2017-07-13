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
 * @file costs.hpp
 *
 * @brief Cost calculation for circuits
 *
 * @author Mathias Soeken
 * @author Nabila Abdessaied
 * @since  1.0
 */

#ifndef COSTS_HPP
#define COSTS_HPP

#include <cstdint>
#include <limits>

#include <boost/variant.hpp>

#include <reversible/circuit.hpp>

namespace cirkit
{

/**
 * @brief Type for costs
 *
 * Costs are measured as uint_64_t.
 *
 * @since  1.2
 */
using cost_t = uint64_t;

/**
 * @brief Undefined costs
 *
 * We'll use the max value of uint64_t as undefined costs
 *
 * @since 2.3
 */
inline constexpr cost_t cost_invalid() { return std::numeric_limits<cost_t>::max(); }

/**
 * @brief Functor for cost function measured by the whole circuit
 *
 * @since  1.0
 */
typedef std::function<cost_t( const circuit& circ )> costs_by_circuit_func;

/**
 * @brief Functor for cost function measured by each gate
 *
 * @since  1.0
 */
typedef std::function<cost_t( const gate& gate, unsigned lines )> costs_by_gate_func;

/**
 * @brief Cost Function type
 *
 * It can either be a costs_by_circuit_func or a costs_by_gate_func
 *
 * @since  1.0
 */
typedef boost::variant<costs_by_circuit_func, costs_by_gate_func> cost_function;

/**
 * @brief Calculates the gate costs
 *
 * This costs class is basically a wrapper around num_gates()
 * and is given for convenience use with other cost
 * functions.
 *
 * @sa \ref sub_cost_functions
 *
 * @since  1.0
 */
struct gate_costs
{
  /**
   * @brief Returns the number of gates
   *
   * @param circ Circuit
   *
   * @return Number of gates
   *
   * @since  1.0
   */
  cost_t operator()( const circuit& circ ) const;
};

/**
 * @brief Calculates the line costs
 *
 * This costs class is basically a wrapper around lines()
 * and is given for convenience use with other cost
 * functions.
 *
 * @sa \ref sub_cost_functions
 *
 * @since  1.0
 */
struct line_costs
{
  /**
   * @brief Returns the number of lines
   *
   * @param circ Circuit
   *
   * @return Number of lines
   *
   * @since  1.0
   */
  cost_t operator()( const circuit& circ ) const;
};

/**
 * @brief Calculates the transistor costs
 *
 * This class calculates the so called transistor
 * costs for a gate. They are the number of
 * control lines multiplied by 8.
 *
 * @sa \ref sub_cost_functions
 *
 * @since  1.0
 */
struct transistor_costs
{
  /**
   * @brief Returns the transistor costs for gate \p g
   *
   * @param g Gate
   * @param lines Number of lines in the circuit
   *
   * @return Transistor Costs for gate \p g
   *
   * @since  1.0
   */
  cost_t operator()( const gate& g, unsigned lines ) const;
};

/**
 * @brief Returns the depth of the rev. circuit
 */
struct depth_costs
{
  cost_t operator()( const circuit& circ ) const;
};

/**
 * @brief Quantum costs from SK:2013
 *
 * Reference: Marek Szyprowski and Pawel Kerntopf, Low quantum
 * cost realization of generalized peres and toffoli gates with
 * multiple-control signals, IEEE Nano 2013, 802--807.
 */
struct sk2013_quantum_costs
{
  cost_t operator()( const gate& g, unsigned lines ) const;
};

/**
 * @brief Quantum costs from Barenco:1995 based on the NCV Library
 *
 * Reference: {Barenco, Adriano and Bennett, Charles H and Cleve, Richard
 * and DiVincenzo, David P and Margolus, Norman and Shor, Peter
 * and Sleator, Tycho and Smolin, John A and Weinfurter, Harald
 * Elementary gates for quantum computation
 * Physical Review A, 1995
 *
 */
struct ncv_quantum_costs {
  cost_t operator()(const gate& g, unsigned lines) const;
};

/**
 * @brief Quantum costs from Barenco:1995 based on the Clifford+T Library
 *
 * Reference: {Barenco, Adriano and Bennett, Charles H and Cleve, Richard
 * and DiVincenzo, David P and Margolus, Norman and Shor, Peter
 * and Sleator, Tycho and Smolin, John A and Weinfurter, Harald
 * Elementary gates for quantum computation
 * Physical Review A, 1995
 */
struct clifford_t_quantum_costs {
  cost_t operator()(const gate& g, unsigned lines) const;
};

/**
 * @brief T depth from Barenco:1995 based on the Clifford+T Library
 *
 * Reference: {Barenco, Adriano and Bennett, Charles H and Cleve, Richard
 * and DiVincenzo, David P and Margolus, Norman and Shor, Peter
 * and Sleator, Tycho and Smolin, John A and Weinfurter, Harald
 * Elementary gates for quantum computation
 * Physical Review A, 1995
 */
struct t_depth_costs {
  cost_t operator()(const gate& g, unsigned lines) const;
};

/**
 * @brief T cost from Barenco:1995 based on the Clifford+T Library
 *
 * Reference: {Barenco, Adriano and Bennett, Charles H and Cleve, Richard
 * and DiVincenzo, David P and Margolus, Norman and Shor, Peter
 * and Sleator, Tycho and Smolin, John A and Weinfurter, Harald
 * Elementary gates for quantum computation
 * Physical Review A, 1995
 */
struct t_costs {
  cost_t operator()(const gate& g, unsigned lines) const;
};

/**
 * @brief H cost from Barenco:1995 based on the Clifford+T Library
 *
 * Reference: {Barenco, Adriano and Bennett, Charles H and Cleve, Richard
 * and DiVincenzo, David P and Margolus, Norman and Shor, Peter
 * and Sleator, Tycho and Smolin, John A and Weinfurter, Harald
 * Elementary gates for quantum computation
 * Physical Review A, 1995
 */
struct h_costs {
  cost_t operator()(const gate& g, unsigned lines) const;
};

/**
 * @brief Calculates the costs of a circuit by a given cost function
 *
 * With this function the costs for a circuit can be calculated.
 * Thereby this functions is generic and calls a cost_function for determine the costs.
 * The costs function can either be derived from costs_by_circuit, whereby
 * the costs are calculated on base by the whole circuit or it can be
 * derived from costs_by_gate, whereby the costs are of each gate are calculated
 * and the sum is returned.
 *
 * @param circ Circuit
 * @param f Cost function
 *
 * @return The costs for the circuit in respect to the given cost function
 *
 * @sa \ref sub_cost_functions
 *
 * @since  1.0
 */
cost_t costs( const circuit& circ, const cost_function& f );

/**
 * @brief Computes the costs  in a given range [begin, end)
 */
cost_t costs( const circuit& circ, unsigned begin, unsigned end, const costs_by_gate_func& f );

}

#endif /* COSTS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
