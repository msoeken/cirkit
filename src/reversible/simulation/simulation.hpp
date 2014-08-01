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
 * @file simulation.hpp
 *
 * @brief General Simulation type definitions
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <core/functor.hpp>
#include <reversible/circuit.hpp>

namespace cirkit
{

  /**
   * @brief Simulation functor
   *
   * @since  1.0
   */
  typedef functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)> simulation_func;

  /**
   * @brief Multi step simulation functor
   *
   * @since  1.2
   */
  typedef functor<bool(std::vector<boost::dynamic_bitset<> >&, const circuit&, const std::vector<boost::dynamic_bitset<> >& )> multi_step_simulation_func;

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
