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
 * @author Heinz Riener
 */

#include <memory>

#include <lscli/utils.hpp>

#include <core/cli/stores.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/cli/commands/read_pla.hpp>
#include <core/cli/commands/testbdd.hpp>
#include <core/utils/bdd_utils.hpp>

#include <classical/cli/commands/bool_complex.hpp>
#include <classical/cli/commands/comb_approx.hpp>
#include <classical/cli/commands/cone.hpp>
#include <classical/cli/commands/dsop.hpp>
#include <classical/cli/commands/expr.hpp>
#include <classical/cli/commands/feather.hpp>
#include <classical/cli/commands/npn.hpp>
#include <classical/cli/commands/output_noise.hpp>
#include <classical/cli/commands/propagate.hpp>
#include <classical/cli/commands/read_aiger.hpp>
#include <classical/cli/commands/read_bench.hpp>
#include <classical/cli/commands/read_sym.hpp>
#include <classical/cli/commands/rename.hpp>
#include <classical/cli/commands/simgraph.hpp>
#include <classical/cli/commands/strash.hpp>
#include <classical/cli/commands/support.hpp>
#include <classical/cli/commands/tt.hpp>
#include <classical/cli/commands/write_aiger.hpp>

#include <abc/cli/commands/abc.hpp>
#include <abc/cli/commands/cec.hpp>

#define STORE_TYPES aig_graph, simple_fanout_graph_t, tt, bdd_function_t, expression_t::ptr, counterexample_t

#ifdef USE_FORMAL_COMMANDS
#include <formal/cli/commands/commands.hpp>
#endif

#ifdef USE_EXPERIMENTAL_CLASSICAL_COMMANDS
#include <classical/cli/commands/commands.hpp>
#endif

#ifdef USE_EXPERIMENTAL_FORMAL_COMMANDS
#include <formal/cli/commands/experimental_commands.hpp>
#endif

using namespace cirkit;

int main( int argc, char ** argv )
{
  cli_main<STORE_TYPES> cli( "cirkit" );

  cli.insert_command( "read_verilog", std::make_shared<read_io_command<io_verilog_tag_t, STORE_TYPES>>( cli.env, "Verilog" ) );
  cli.insert_command( "write_edgelist", std::make_shared<write_io_command<io_edgelist_tag_t, STORE_TYPES>>( cli.env, "Edge list" ) );
  cli.insert_command( "write_verilog", std::make_shared<write_io_command<io_verilog_tag_t, STORE_TYPES>>( cli.env, "Verilog" ) );

  /* core */
  ADD_COMMAND( bdd );
  ADD_COMMAND( read_pla );
  ADD_COMMAND( testbdd );

  /* classical */
  ADD_COMMAND( abc );
  ADD_COMMAND( bool_complex );
  ADD_COMMAND( cec );
  ADD_COMMAND( comb_approx );
  ADD_COMMAND( cone );
  ADD_COMMAND( dsop );
  ADD_COMMAND( expr );
  ADD_COMMAND( feather );
  ADD_COMMAND( npn );
  ADD_COMMAND( output_noise );
  ADD_COMMAND( propagate );
  ADD_COMMAND( read_aiger );
  ADD_COMMAND( read_bench );
  ADD_COMMAND( read_sym );
  ADD_COMMAND( rename );
  ADD_COMMAND( simgraph );
  ADD_COMMAND( strash );
  ADD_COMMAND( support );
  ADD_COMMAND( tt );
  ADD_COMMAND( write_aiger );

#ifdef USE_FORMAL_COMMANDS
  FORMAL_COMMANDS
#endif

#ifdef USE_EXPERIMENTAL_CLASSICAL_COMMANDS
  EXPERIMENTAL_CLASSICAL_COMMANDS
#endif

#ifdef USE_EXPERIMENTAL_FORMAL_COMMANDS
  EXPERIMENTAL_FORMAL_COMMANDS
#endif

  return cli.run( argc, argv );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
