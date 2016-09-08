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
 * @file blif_to_bench.hpp
 *
 * @brief BLIF to BENCH converter
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_BLIF_TO_BENCH_COMMAND_HPP
#define CLI_BLIF_TO_BENCH_COMMAND_HPP

#include <string>

#include <core/cli/cirkit_command.hpp>

namespace cirkit
{

class blif_to_bench_command : public cirkit_command
{
public:
  blif_to_bench_command( const environment::ptr& env );

protected:
  rules_t validity_rules() const;
  bool execute();

private:
  std::string blif_name;
  std::string bench_name;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
