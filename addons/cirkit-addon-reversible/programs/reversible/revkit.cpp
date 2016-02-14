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
 * @author Mathias Soeken
 */

#include <memory>

#include <core/cli/command.hpp>
#include <core/cli/environment.hpp>
#include <core/cli/rules.hpp>
#include <core/cli/store.hpp>
#include <core/cli/stores.hpp>
#include <core/cli/utils.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/cli/commands/read_pla.hpp>
#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/cli/commands/aig.hpp>
#include <classical/cli/commands/read_aiger.hpp>
#include <classical/cli/commands/write_aiger.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/cli/commands/adding_lines.hpp>
#include <reversible/cli/commands/embed.hpp>
#include <reversible/cli/commands/exs.hpp>
#include <reversible/cli/commands/dbs.hpp>
#include <reversible/cli/commands/hdbs.hpp>
#include <reversible/cli/commands/random_circuit.hpp>
#include <reversible/cli/commands/rcbdd.hpp>
#include <reversible/cli/commands/read_real.hpp>
#include <reversible/cli/commands/read_spec.hpp>
#include <reversible/cli/commands/required_lines.hpp>
#include <reversible/cli/commands/spec.hpp>
#include <reversible/cli/commands/write_real.hpp>
#include <reversible/cli/commands/write_spec.hpp>
#include <reversible/cli/commands/write_pla.hpp>

#ifdef USE_EXPERIMENTAL_REVERSIBLE_COMMANDS
#include <reversible/cli/commands/commands.hpp>
#endif

using namespace cirkit;

int main( int argc, char ** argv )
{
  cli_main<circuit, binary_truth_table, bdd_function_t, rcbdd, aig_graph> cli( "revkit" );

  ADD_COMMAND( adding_lines );
  cli.env->commands.insert( {"aig", std::make_shared<aig_command<circuit, binary_truth_table, bdd_function_t, rcbdd, aig_graph>>( cli.env ) } );
  ADD_COMMAND( bdd );
  ADD_COMMAND( dbs );
  ADD_COMMAND( embed );
  ADD_COMMAND( exs );
  ADD_COMMAND( hdbs );
  ADD_COMMAND( random_circuit );
  ADD_COMMAND( rcbdd );
  ADD_COMMAND( read_aiger );
  ADD_COMMAND( read_pla );
  ADD_COMMAND( read_real );
  ADD_COMMAND( read_spec );
  ADD_COMMAND( required_lines );
  ADD_COMMAND( spec );
  ADD_COMMAND( write_aiger );
  ADD_COMMAND( write_real );
  ADD_COMMAND( write_spec );
  ADD_COMMAND( write_pla );

#ifdef USE_EXPERIMENTAL_REVERSIBLE_COMMANDS
  EXPERIMENTAL_REVERSIBLE_COMMANDS
#endif

  return cli.run( argc, argv );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
