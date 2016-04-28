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

/**
 * @file print_statistics.hpp
 *
 * @brief Print statistics about a circuit
 *
 * @author Mathias Soeken
 * @author Nabila Abdessaied
 * @since  1.0
 */

#ifndef PRINT_STATISTICS_HPP
#define PRINT_STATISTICS_HPP

#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Settings for print_statistics
   *
   * @since  1.0
   */
  struct print_statistics_settings
  {
    /**
     * @brief Main Template for statistics output
     *
     * Use
     *  - \b \%1$s as placeholder for the run-time string (specified by \p runtime_template),
     *  - \b \%2$d as placeholder for number of lines.
     *  - \b \%3$d as placeholder for number of gates.
     *  - \b \%4$d as placeholder for NCV quantum costs.
     *  - \b \%5$d as placeholder for T-depth.
     *  - \b \%6$d as placeholder for T-count.
     *  - \b \%7$d as placeholder for H-count.
     *  - \b \%8$d as placeholder for number of qubits (#lines (+1 if there is a gate with #lines - 1 control lines))
     *  - \b \%9$d as placeholder for number of transistor costs.
     *  - \b \%10$d as placeholder for number of SK2013 quantum costs.
     *
     * @since  1.0
     */
    std::string main_template = "%1$sLines:        %2$d\nGates:        %3$d\nNCV QCosts:   %4$d\nT-depth:      %5$d\nT-count:      %6$d\nH-count:      %7$d\nQubits:       %8$d\nTransistor:   %9$d\nSK2013 costs: %10$d\n";


    /**
     * @brief Run-time Template for statistics output
     *
     * This template will only be generated when a run-time parameter is specified (i.e. not -1)
     * in the print_statistics function.
     *
     * There is only one placeholder, where you can use the C \p printf notation,
     * for example <b>\%.2f</b> for 2 digits after the comma.
     *
     * Default value is: <tt>"Runtime: %.2f\n"</tt>
     *
     * @since  1.0
     */
    std::string runtime_template = "Runtime:      %.2f\n";
  };

  /**
   * @brief Print statistics about a circuit to an arbitrary output stream
   *
   * This function prints statistics to an arbitrary output stream \p os.
   * It should provide a unique way of displaying information about circuit and run-time
   * and should be used for all single algorithm implementations.
   *
   * @param os      Output stream where to print the information
   * @param circ    Circuit to obtain information from
   * @param runtime Optional, if a run-time has been measured, it will be displayed as well
   * @param settings Settings for printing the statistics (with templates)
   *
   * @since  1.0
   */
  void print_statistics( std::ostream& os, const circuit& circ, double runtime = -1.0,
                         const print_statistics_settings& settings = print_statistics_settings() );

  void print_statistics( const std::string& filename, const circuit& circ, double runtime = -1.0,
                         const print_statistics_settings& settings = print_statistics_settings() );

  /**
   * @brief Print statistics about a circuit to standard output
   *
   * This function prints statistics to standard output.
   * It should provide a unique way of displaying information about circuit and run-time
   * and should be used for all single algorithm implementations.
   *
   * @param circ    Circuit to obtain information from
   * @param runtime Optional, if a run-time has been measured, it will be displayed as well
   * @param settings Settings for printing the statistics (with templates)
   *
   * @since  1.0
   */
  void print_statistics( const circuit& circ, double runtime = -1.0,
                         const print_statistics_settings& settings = print_statistics_settings() );

}

#endif /* PRINT_STATISTICS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
