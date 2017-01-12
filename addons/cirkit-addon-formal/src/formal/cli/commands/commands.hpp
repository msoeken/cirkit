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
 * @file commands.hpp
 *
 * @brief Formal commands for CLI
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_FORMAL_COMMANDS_HPP
#define CLI_FORMAL_COMMANDS_HPP

#include <formal/cli/commands/exact_mig.hpp>
#include <formal/cli/commands/exact_xmg.hpp>
#include <formal/cli/commands/xmglut.hpp>
#include <formal/cli/commands/xmgmine.hpp>

#define CIRKIT_FORMAL_Z3_COMMANDS  \
  cli.set_category( "Synthesis" ); \
  ADD_COMMAND( exact_mig );        \
  ADD_COMMAND( exact_xmg );        \
  ADD_COMMAND( xmglut );           \
  ADD_COMMAND( xmgmine );

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
