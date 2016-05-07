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

#include <lscli/utils.hpp>

#include <core/cli/stores.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/cli/commands/read_pla.hpp>
#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/cli/commands/expr.hpp>
#include <classical/cli/commands/read_aiger.hpp>
#include <classical/cli/commands/write_aiger.hpp>
#include <classical/utils/expression_parser.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/cli/commands/adding_lines.hpp>
#include <reversible/cli/commands/cbs.hpp>
#include <reversible/cli/commands/embed.hpp>
#include <reversible/cli/commands/enumerate.hpp>
#include <reversible/cli/commands/esopbs.hpp>
#include <reversible/cli/commands/exs.hpp>
#include <reversible/cli/commands/dbs.hpp>
#include <reversible/cli/commands/hdbs.hpp>
#include <reversible/cli/commands/qbs.hpp>
#include <reversible/cli/commands/random_circuit.hpp>
#include <reversible/cli/commands/rec.hpp>
#include <reversible/cli/commands/required_lines.hpp>
#include <reversible/cli/commands/tbs.hpp>
#include <reversible/cli/commands/write_pla.hpp>

#ifdef USE_EXPERIMENTAL_REVERSIBLE_COMMANDS
#include <reversible/cli/commands/commands.hpp>
#endif

using namespace cirkit;

#define STORES circuit, binary_truth_table, expression_t::ptr, bdd_function_t, rcbdd, aig_graph

int main( int argc, char ** argv )
{
  cli_main<STORES> cli( "revkit" );

  cli.insert_command( "read_real",     std::make_shared<read_io_command<io_real_tag_t,     STORES>>( cli.env, "realization" ) );
  cli.insert_command( "read_spec",     std::make_shared<read_io_command<io_spec_tag_t,     STORES>>( cli.env, "specification" ) );
  cli.insert_command( "write_qpic",    std::make_shared<write_io_command<io_qpic_tag_t,    STORES>>( cli.env, "qpic" ) );
  cli.insert_command( "write_quipper", std::make_shared<write_io_command<io_quipper_tag_t, STORES>>( cli.env, "Quipper" ) );
  cli.insert_command( "write_real",    std::make_shared<write_io_command<io_real_tag_t,    STORES>>( cli.env, "realization" ) );
  cli.insert_command( "write_spec",    std::make_shared<write_io_command<io_spec_tag_t,    STORES>>( cli.env, "specification" ) );
  cli.insert_command( "write_tikz",    std::make_shared<write_io_command<io_tikz_tag_t,    STORES>>( cli.env, "TikZ" ) );

  ADD_COMMAND( adding_lines );
  ADD_COMMAND( bdd );
  ADD_COMMAND( cbs );
  ADD_COMMAND( dbs );
  ADD_COMMAND( embed );
  ADD_COMMAND( enumerate );
  ADD_COMMAND( esopbs );
  ADD_COMMAND( expr );
  ADD_COMMAND( exs );
  ADD_COMMAND( hdbs );
  ADD_COMMAND( qbs );
  ADD_COMMAND( random_circuit );
  ADD_COMMAND( read_aiger );
  ADD_COMMAND( read_pla );
  ADD_COMMAND( rec );
  ADD_COMMAND( required_lines );
  ADD_COMMAND( tbs );
  ADD_COMMAND( write_aiger );
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
