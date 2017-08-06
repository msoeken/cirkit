/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
#include <classical/cli/commands/cone.hpp>
#include <classical/cli/commands/esop.hpp>
#include <classical/cli/commands/exorcism.hpp>
#include <classical/cli/commands/print_io.hpp>
#include <classical/cli/commands/propagate.hpp>
#include <classical/cli/commands/simulate.hpp>
#include <classical/cli/commands/spectral.hpp>
#include <classical/cli/commands/tt.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <formal/cli/commands/xmglut.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/cli/commands/adding_lines.hpp>
#include <reversible/cli/commands/cbs.hpp>
#include <reversible/cli/commands/circuit_matrix.hpp>
#include <reversible/cli/commands/circuit_str.hpp>
#include <reversible/cli/commands/concat.hpp>
#include <reversible/cli/commands/d1s.hpp>
#include <reversible/cli/commands/dbs.hpp>
#include <reversible/cli/commands/dxs.hpp>
#include <reversible/cli/commands/embed.hpp>
#include <reversible/cli/commands/enumerate.hpp>
#include <reversible/cli/commands/esopbs.hpp>
#include <reversible/cli/commands/exs.hpp>
#include <reversible/cli/commands/filter.hpp>
#include <reversible/cli/commands/gen_reciprocal.hpp>
#include <reversible/cli/commands/hdbs.hpp>
#include <reversible/cli/commands/is_identity.hpp>
#include <reversible/cli/commands/lhrs.hpp>
#include <reversible/cli/commands/maslov234.hpp>
#include <reversible/cli/commands/mitm.hpp>
#include <reversible/cli/commands/nct.hpp>
#include <reversible/cli/commands/perm.hpp>
#include <reversible/cli/commands/pos.hpp>
#include <reversible/cli/commands/qbs.hpp>
#include <reversible/cli/commands/qec.hpp>
#include <reversible/cli/commands/random_circuit.hpp>
#include <reversible/cli/commands/rec.hpp>
#include <reversible/cli/commands/reduce_lines.hpp>
#include <reversible/cli/commands/required_lines.hpp>
#include <reversible/cli/commands/reverse.hpp>
#include <reversible/cli/commands/revgen.hpp>
#include <reversible/cli/commands/revsim.hpp>
#include <reversible/cli/commands/revsimp.hpp>
#include <reversible/cli/commands/rms.hpp>
#include <reversible/cli/commands/stg4.hpp>
#include <reversible/cli/commands/stg_as.hpp>
#include <reversible/cli/commands/tbs.hpp>
#include <reversible/cli/commands/tof.hpp>
#include <reversible/cli/commands/tpar.hpp>
#include <reversible/cli/commands/unique_names.hpp>

#ifdef USE_EXPERIMENTAL_REVERSIBLE_COMMANDS
#include <reversible/cli/commands/commands.hpp>
#endif

using namespace cirkit;

#define STORES circuit, binary_truth_table, tt, expression_t::ptr, bdd_function_t, rcbdd, aig_graph, xmg_graph

ALICE_BEGIN(revkit)

  cli_main<STORES> cli( "revkit" );

  cli.set_category ("I/O" );

  ADD_READ_COMMAND( aiger, "Aiger" );
  ADD_READ_COMMAND( bench, "Bench" );
  ADD_READ_COMMAND( pla, "PLA" );
  ADD_READ_COMMAND( qc, "QC" );
  ADD_READ_COMMAND( real, "realization" );
  ADD_READ_COMMAND( spec, "specification" );
  ADD_READ_COMMAND( verilog, "Verilog" );
  ADD_WRITE_COMMAND( aiger, "Aiger" );
  ADD_WRITE_COMMAND( liquid, "LIQUi|>" );
  ADD_WRITE_COMMAND( numpy, "NumPy" );
  ADD_WRITE_COMMAND( pla, "PLA" );
  ADD_WRITE_COMMAND( projectq, "ProjectQ" );
  ADD_WRITE_COMMAND( qc, "QC" );
  ADD_WRITE_COMMAND( qcode, "QCode" );
  ADD_WRITE_COMMAND( qpic, "qpic" );
  ADD_WRITE_COMMAND( quipper, "Quipper" );
  ADD_WRITE_COMMAND( real, "realization" );
  ADD_WRITE_COMMAND( spec, "specification" );
  ADD_WRITE_COMMAND( tikz, "TikZ" );
  ADD_WRITE_COMMAND( verilog, "Verilog" );

  cli.set_category ("Embedding" );

  ADD_COMMAND( embed );
  ADD_COMMAND( required_lines );

  cli.set_category( "Synthesis" );

  ADD_COMMAND( cbs );
  ADD_COMMAND( d1s );
  ADD_COMMAND( dbs );
  ADD_COMMAND( dxs );
  ADD_COMMAND( esopbs );
  ADD_COMMAND( exs );
  ADD_COMMAND( hdbs );
  ADD_COMMAND( lhrs );
  ADD_COMMAND( qbs );
  ADD_COMMAND( rms );
  ADD_COMMAND( tbs );

  cli.set_category( "Optimization" );

  ADD_COMMAND( adding_lines );
  ADD_COMMAND( esop );
  ADD_COMMAND( exorcism );
  ADD_COMMAND( reduce_lines );
  ADD_COMMAND( revsimp );
  ADD_COMMAND( tpar );

  cli.set_category( "Mapping and rewriting" );

  ADD_COMMAND( concat );
  ADD_COMMAND( filter );
  ADD_COMMAND( maslov234 );
  ADD_COMMAND( mitm );
  ADD_COMMAND( nct );
  ADD_COMMAND( pos );
  ADD_COMMAND( reverse );
  ADD_COMMAND( stg4 );
  ADD_COMMAND( tof );
  ADD_COMMAND( unique_names );

  cli.set_category( "AIG and XMG synthesis" );

  ADD_COMMAND( cone );
  ADD_COMMAND( propagate );
  ADD_COMMAND( xmglut );

  cli.set_category( "Verification" );

  ADD_COMMAND( is_identity );
  ADD_COMMAND( qec );
  ADD_COMMAND( rec );
  ADD_COMMAND( revsim );
  ADD_COMMAND( simulate );

  cli.set_category( "Generator" );

  ADD_COMMAND( gen_reciprocal );
  ADD_COMMAND( revgen );
  ADD_COMMAND( stg_as );

  cli.set_category( "Various" );

  ADD_COMMAND( abc );
  ADD_COMMAND( bdd );
  ADD_COMMAND( circuit_matrix );
  ADD_COMMAND( circuit_str );
  ADD_COMMAND( enumerate );
  ADD_COMMAND( expr );
  ADD_COMMAND( perm );
  ADD_COMMAND( print_io );
  ADD_COMMAND( random_circuit );
  ADD_COMMAND( spectral );
  ADD_COMMAND( tt );

#ifdef USE_EXPERIMENTAL_REVERSIBLE_COMMANDS
  EXPERIMENTAL_REVERSIBLE_COMMANDS
#endif

ALICE_END

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
