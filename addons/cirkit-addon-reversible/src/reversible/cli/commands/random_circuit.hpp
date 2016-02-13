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
 * @file random_circuit.hpp
 *
 * @brief Generates a random reversible circuit
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef RANDOM_CIRCUIT_HPP
#define RANDOM_CIRCUIT_HPP

#include <core/cli/command.hpp>

namespace cirkit
{

class random_circuit_command : public command
{
public:
  random_circuit_command( const environment::ptr& env );

  rules_t validity_rules() const;

protected:
  bool execute();

public:
  log_opt_t log() const;

private:
  unsigned lines = 4u;
  unsigned gates = 10u;
  unsigned seed;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
