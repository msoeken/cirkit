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

#include <alice/alice.hpp>

#include <core/cli/stores.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/cli/commands/testbdd.hpp>
#include <core/utils/bdd_utils.hpp>

#include <classical/cli/commands/abc.hpp>
#include <classical/cli/commands/bool_complex.hpp>
#include <classical/cli/commands/cec.hpp>
#include <classical/cli/commands/comb_approx.hpp>
#include <classical/cli/commands/cone.hpp>
#include <classical/cli/commands/cuts.hpp>
#include <classical/cli/commands/depth.hpp>
#include <classical/cli/commands/dsop.hpp>
#include <classical/cli/commands/expr.hpp>
#include <classical/cli/commands/feather.hpp>
#include <classical/cli/commands/gen_trans_arith.hpp>
#include <classical/cli/commands/isop.hpp>
#include <classical/cli/commands/memristor.hpp>
#include <classical/cli/commands/mig_rewrite.hpp>
#include <classical/cli/commands/migfh.hpp>
#include <classical/cli/commands/npn.hpp>
#include <classical/cli/commands/output_noise.hpp>
#include <classical/cli/commands/plim.hpp>
#include <classical/cli/commands/propagate.hpp>
#include <classical/cli/commands/read_sym.hpp>
#include <classical/cli/commands/rename.hpp>
#include <classical/cli/commands/satnpn.hpp>
#include <classical/cli/commands/shuffle.hpp>
#include <classical/cli/commands/simgraph.hpp>
#include <classical/cli/commands/simulate.hpp>
#include <classical/cli/commands/strash.hpp>
#include <classical/cli/commands/support.hpp>
#include <classical/cli/commands/tt.hpp>
#include <classical/cli/commands/unate.hpp>

#ifdef USE_FORMAL_COMMANDS
#include <formal/cli/commands/exact_mig.hpp>
#endif

#ifdef USE_FPGA_COMMANDS
#include <classical/cli/commands/fpga_commands.hpp>
#endif

#define STORE_TYPES aig_graph, mig_graph, xmg_graph, simple_fanout_graph_t, tt, bdd_function_t, expression_t::ptr, counterexample_t

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

  cli.set_category( "I/O" );

  ADD_READ_COMMAND( aiger, "Aiger" );
  ADD_READ_COMMAND( bench, "Bench" );
  ADD_READ_COMMAND( pla, "PLA" );
  ADD_READ_COMMAND( verilog, "Verilog" );
  ADD_WRITE_COMMAND( aiger, "Aiger" );
  ADD_WRITE_COMMAND( edgelist, "Edge list" );
  ADD_WRITE_COMMAND( pla, "PLA" );
  ADD_WRITE_COMMAND( verilog, "Verilog" );
  ADD_COMMAND( read_sym );

  cli.set_category( "Approximation" );
  ADD_COMMAND( comb_approx );

  cli.set_category( "Experimental" );
  ADD_COMMAND( testbdd );

  cli.set_category( "Rewriting" );
  ADD_COMMAND( cone );
  ADD_COMMAND( feather );
  ADD_COMMAND( mig_rewrite );
  ADD_COMMAND( migfh );
  ADD_COMMAND( propagate );
  ADD_COMMAND( rename );
  ADD_COMMAND( shuffle );
  ADD_COMMAND( strash );

  cli.set_category( "Verification" );
  ADD_COMMAND( cec );
  ADD_COMMAND( simulate );
  ADD_COMMAND( support );
  ADD_COMMAND( unate );

  cli.set_category( "Truth table" );
  ADD_COMMAND( bool_complex );
  ADD_COMMAND( npn );
  ADD_COMMAND( tt );

  cli.set_category( "Synthesis" );
#ifdef USE_FORMAL_COMMANDS
  ADD_COMMAND( exact_mig );
#endif
  ADD_COMMAND( isop );

  cli.set_category( "Reverse engineering" );
  ADD_COMMAND( cuts );
  ADD_COMMAND( satnpn );
  ADD_COMMAND( simgraph );

  cli.set_category( "RRAM" );
  ADD_COMMAND( memristor );
  ADD_COMMAND( plim );

  cli.set_category( "Generator" );
  ADD_COMMAND( gen_trans_arith );

  cli.set_category( "Various" );
  ADD_COMMAND( abc );
  ADD_COMMAND( bdd );
  ADD_COMMAND( depth );
  ADD_COMMAND( dsop );
  ADD_COMMAND( expr );
  ADD_COMMAND( output_noise );

#ifdef USE_FPGA_COMMANDS
  FPGA_CLASSICAL_COMMANDS
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
