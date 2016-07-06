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

#include <alice/alice.hpp>

#include <core/cli/stores.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/cli/commands/expr.hpp>
#include <classical/cli/commands/abc.hpp>
#include <classical/cli/commands/cec.hpp>
#include <classical/cli/commands/simulate.hpp>
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
#include <reversible/cli/commands/nct.hpp>
#include <reversible/cli/commands/pos.hpp>
#include <reversible/cli/commands/qbs.hpp>
#include <reversible/cli/commands/random_circuit.hpp>
#include <reversible/cli/commands/rec.hpp>
#include <reversible/cli/commands/required_lines.hpp>
#include <reversible/cli/commands/revsim.hpp>
#include <reversible/cli/commands/revsimp.hpp>
#include <reversible/cli/commands/rms.hpp>
#include <reversible/cli/commands/tbs.hpp>
#include <reversible/cli/commands/tof.hpp>
#include <reversible/cli/commands/unique_names.hpp>

#ifdef USE_EXPERIMENTAL_REVERSIBLE_COMMANDS
#include <reversible/cli/commands/commands.hpp>
#endif

using namespace cirkit;

#define STORES circuit, binary_truth_table, expression_t::ptr, bdd_function_t, rcbdd, aig_graph

int main( int argc, char ** argv )
{
  cli_main<STORES> cli( "revkit" );

  cli.set_category ("I/O" );

  ADD_READ_COMMAND( aiger, "Aiger" );
  ADD_READ_COMMAND( bench, "Bench" );
  ADD_READ_COMMAND( pla, "PLA" );
  ADD_READ_COMMAND( real, "realization" );
  ADD_READ_COMMAND( spec, "specification" );
  ADD_READ_COMMAND( verilog, "Verilog" );
  ADD_WRITE_COMMAND( pla, "PLA" );
  ADD_WRITE_COMMAND( qpic, "qpic" );
  ADD_WRITE_COMMAND( quipper, "Quipper" );
  ADD_WRITE_COMMAND( real, "realization" );
  ADD_WRITE_COMMAND( spec, "specification" );
  ADD_WRITE_COMMAND( tikz, "TikZ" );

  cli.set_category ("Embedding" );

  ADD_COMMAND( embed );
  ADD_COMMAND( required_lines );

  cli.set_category( "Synthesis" );

  ADD_COMMAND( cbs );
  ADD_COMMAND( dbs );
  ADD_COMMAND( esopbs );
  ADD_COMMAND( exs );
  ADD_COMMAND( hdbs );
  ADD_COMMAND( qbs );
  ADD_COMMAND( rms );
  ADD_COMMAND( tbs );

  cli.set_category( "Optimization" );

  ADD_COMMAND( adding_lines );
  ADD_COMMAND( revsimp );

  cli.set_category( "Mapping and rewriting" );

  ADD_COMMAND( nct );
  ADD_COMMAND( pos );
  ADD_COMMAND( tof );
  ADD_COMMAND( unique_names );

  cli.set_category( "Verification" );

  ADD_COMMAND( cec );
  ADD_COMMAND( rec );
  ADD_COMMAND( revsim );
  ADD_COMMAND( simulate );

  cli.set_category( "Various" );

  ADD_COMMAND( abc );
  ADD_COMMAND( bdd );
  ADD_COMMAND( enumerate );
  ADD_COMMAND( expr );
  ADD_COMMAND( random_circuit );

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
