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

#include <cli/stores.hpp>
#include <cli/stores_io.hpp>
#include <cli/reversible_stores.hpp>
#include <cli/reversible_stores_io.hpp>
#include <cli/commands/abc.hpp>
#include <cli/commands/adding_lines.hpp>
#include <cli/commands/bdd.hpp>
#include <cli/commands/cbs.hpp>
#include <cli/commands/circuit_matrix.hpp>
#include <cli/commands/circuit_str.hpp>
#include <cli/commands/concat.hpp>
#include <cli/commands/cone.hpp>
#include <cli/commands/d1s.hpp>
#include <cli/commands/dbs.hpp>
#include <cli/commands/dxs.hpp>
#include <cli/commands/embed.hpp>
#include <cli/commands/enumerate.hpp>
#include <cli/commands/esop.hpp>
#include <cli/commands/esopbs.hpp>
#include <cli/commands/esopps.hpp>
#include <cli/commands/exorcism.hpp>
#include <cli/commands/expr.hpp>
#include <cli/commands/exs.hpp>
#include <cli/commands/filter.hpp>
#include <cli/commands/gates.hpp>
#include <cli/commands/gen_reciprocal.hpp>
#include <cli/commands/hdbs.hpp>
#include <cli/commands/is_identity.hpp>
#include <cli/commands/lhrs.hpp>
#include <cli/commands/mitm.hpp>
#include <cli/commands/nct.hpp>
#include <cli/commands/perm.hpp>
#include <cli/commands/pos.hpp>
#include <cli/commands/print_io.hpp>
#include <cli/commands/propagate.hpp>
#include <cli/commands/qbs.hpp>
#include <cli/commands/qec.hpp>
#include <cli/commands/random_circuit.hpp>
#include <cli/commands/rec.hpp>
#include <cli/commands/reduce_lines.hpp>
#include <cli/commands/required_lines.hpp>
#include <cli/commands/reverse.hpp>
#include <cli/commands/revgen.hpp>
#include <cli/commands/revsim.hpp>
#include <cli/commands/revsimp.hpp>
#include <cli/commands/rms.hpp>
#include <cli/commands/rptm.hpp>
#include <cli/commands/simulate.hpp>
#include <cli/commands/spectral.hpp>
#include <cli/commands/stg4.hpp>
#include <cli/commands/stg_as.hpp>
#include <cli/commands/tbs.hpp>
#include <cli/commands/tof.hpp>
#include <cli/commands/tpar.hpp>
#include <cli/commands/tt.hpp>
#include <cli/commands/unique_names.hpp>
#ifdef ADDON_FORMAL
#include <cli/commands/xmglut.hpp>
#endif

#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>

using namespace cirkit;

namespace alice
{

ALICE_ADD_COMMAND( embed, "Embedding" )
ALICE_ADD_COMMAND( required_lines, "Embedding" )

ALICE_ADD_COMMAND( cbs, "Synthesis" )
ALICE_ADD_COMMAND( d1s, "Synthesis" )
ALICE_ADD_COMMAND( dbs, "Synthesis" )
ALICE_ADD_COMMAND( dxs, "Synthesis" )
ALICE_ADD_COMMAND( esopbs, "Synthesis" )
ALICE_ADD_COMMAND( esopps, "Synthesis" )
#ifdef ADDON_FORMAL
ALICE_ADD_COMMAND( exs, "Synthesis" )
#endif
ALICE_ADD_COMMAND( hdbs, "Synthesis" )
ALICE_ADD_COMMAND( lhrs, "Synthesis" )
ALICE_ADD_COMMAND( qbs, "Synthesis" )
ALICE_ADD_COMMAND( rms, "Synthesis" )
ALICE_ADD_COMMAND( tbs, "Synthesis" )

ALICE_ADD_COMMAND( adding_lines, "Optimization" )
ALICE_ADD_COMMAND( esop, "Optimization" )
ALICE_ADD_COMMAND( exorcism, "Optimization" )
ALICE_ADD_COMMAND( reduce_lines, "Optimization" )
ALICE_ADD_COMMAND( revsimp, "Optimization" )
ALICE_ADD_COMMAND( tpar, "Optimization" )

ALICE_ADD_COMMAND( concat, "Mapping and rewriting" )
ALICE_ADD_COMMAND( filter, "Mapping and rewriting" )
ALICE_ADD_COMMAND( mitm, "Mapping and rewriting" )
ALICE_ADD_COMMAND( nct, "Mapping and rewriting" )
ALICE_ADD_COMMAND( pos, "Mapping and rewriting" )
ALICE_ADD_COMMAND( reverse, "Mapping and rewriting" )
ALICE_ADD_COMMAND( rptm, "Mapping and rewriting" )
ALICE_ADD_COMMAND( stg4, "Mapping and rewriting" )
ALICE_ADD_COMMAND( tof, "Mapping and rewriting" )
ALICE_ADD_COMMAND( unique_names, "Mapping and rewriting" )

ALICE_ADD_COMMAND( cone, "AIG and XMG synthesis" )
ALICE_ADD_COMMAND( propagate, "AIG and XMG synthesis" )
#ifdef ADDON_FORMAL
ALICE_ADD_COMMAND( xmglut, "AIG and XMG synthesis" )
#endif

ALICE_ADD_COMMAND( is_identity, "Verification" )
ALICE_ADD_COMMAND( qec, "Verification" )
ALICE_ADD_COMMAND( rec, "Verification" )
ALICE_ADD_COMMAND( revsim, "Verification" )
ALICE_ADD_COMMAND( simulate, "Verification" )

ALICE_ADD_COMMAND( gen_reciprocal, "Generator" )
ALICE_ADD_COMMAND( revgen, "Generator" )
ALICE_ADD_COMMAND( stg_as, "Generator" )

ALICE_ADD_COMMAND( abc, "Various" )
ALICE_ADD_COMMAND( bdd, "Various" )
ALICE_ADD_COMMAND( circuit_matrix, "Various" )
ALICE_ADD_COMMAND( circuit_str, "Various" )
ALICE_ADD_COMMAND( enumerate, "Various" )
ALICE_ADD_COMMAND( expr, "Various" )
ALICE_ADD_COMMAND( gates, "Various" )
ALICE_ADD_COMMAND( perm, "Various" )
ALICE_ADD_COMMAND( print_io, "Various" )
ALICE_ADD_COMMAND( random_circuit, "Various" )
ALICE_ADD_COMMAND( spectral, "Various" )
ALICE_ADD_COMMAND( tt, "Various" )

}

ALICE_MAIN(revkit)

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
