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
 * @author Heinz Riener
 */

#include <memory>

#include <alice/alice.hpp>

#include <cli/stores.hpp>
#include <cli/stores_io.hpp>
#include <cli/commands/abc.hpp>
#include <cli/commands/bdd.hpp>
#include <cli/commands/blif_to_bench.hpp>
#include <cli/commands/bool_complex.hpp>
#include <cli/commands/comb_approx.hpp>
#include <cli/commands/compress.hpp>
#include <cli/commands/cone.hpp>
#include <cli/commands/contrinv.hpp>
#include <cli/commands/cuts.hpp>
#include <cli/commands/dectest.hpp>
#include <cli/commands/demo.hpp>
#include <cli/commands/depth.hpp>
#include <cli/commands/dsop.hpp>
#include <cli/commands/esop.hpp>
#include <cli/commands/exorcism.hpp>
#include <cli/commands/expr.hpp>
#include <cli/commands/feather.hpp>
#include <cli/commands/gen_npn_circuit.hpp>
#include <cli/commands/gen_trans_arith.hpp>
#include <cli/commands/isop.hpp>
#include <cli/commands/memristor.hpp>
#include <cli/commands/mig_rewrite.hpp>
#include <cli/commands/migfh.hpp>
#include <cli/commands/npn.hpp>
#include <cli/commands/output_noise.hpp>
#include <cli/commands/permmask.hpp>
#include <cli/commands/plim.hpp>
#include <cli/commands/print_io.hpp>
#include <cli/commands/propagate.hpp>
#include <cli/commands/read_sym.hpp>
#include <cli/commands/rename.hpp>
#include <cli/commands/satnpn.hpp>
#include <cli/commands/shuffle.hpp>
#include <cli/commands/simgraph.hpp>
#include <cli/commands/simulate.hpp>
#include <cli/commands/spectral.hpp>
#include <cli/commands/strash.hpp>
#include <cli/commands/support.hpp>
#include <cli/commands/testbdd.hpp>
#include <cli/commands/tt.hpp>
#include <cli/commands/unate.hpp>
#include <cli/commands/worstcase.hpp>
#include <cli/commands/xmgmerge.hpp>

#include <core/utils/bdd_utils.hpp>

namespace alice
{

ALICE_ADD_COMMAND( read_sym, "I/O" );
ALICE_ADD_COMMAND( blif_to_bench, "I/O" );

ALICE_ADD_COMMAND( comb_approx, "Approximation" );
ALICE_ADD_COMMAND( worstcase, "Approximation" );

ALICE_ADD_COMMAND( testbdd, "Experimental"  );

ALICE_ADD_COMMAND( cone, "Rewriting" );
ALICE_ADD_COMMAND( feather, "Rewriting" );
ALICE_ADD_COMMAND( mig_rewrite, "Rewriting" );
ALICE_ADD_COMMAND( migfh, "Rewriting" );
ALICE_ADD_COMMAND( propagate, "Rewriting" );
ALICE_ADD_COMMAND( rename, "Rewriting" );
ALICE_ADD_COMMAND( shuffle, "Rewriting" );
ALICE_ADD_COMMAND( strash, "Rewriting" );
ALICE_ADD_COMMAND( xmgmerge, "Rewriting" );

ALICE_ADD_COMMAND( simulate, "Verification" );
ALICE_ADD_COMMAND( support, "Verification" );
ALICE_ADD_COMMAND( unate, "Verification" );

ALICE_ADD_COMMAND( bool_complex, "Truth table" );
ALICE_ADD_COMMAND( dectest, "Truth table" );
ALICE_ADD_COMMAND( npn, "Truth table" );
ALICE_ADD_COMMAND( permmask, "Truth table" );
ALICE_ADD_COMMAND( spectral, "Truth table" );
ALICE_ADD_COMMAND( tt, "Truth table" );

ALICE_ADD_COMMAND( isop, "Synthesis" );

ALICE_ADD_COMMAND( esop, "Optimization" );
ALICE_ADD_COMMAND( exorcism, "Optimization" );

ALICE_ADD_COMMAND( contrinv, "Reverse engineering" );
ALICE_ADD_COMMAND( cuts, "Reverse engineering" );
ALICE_ADD_COMMAND( satnpn, "Reverse engineering" );
ALICE_ADD_COMMAND( simgraph, "Reverse engineering" );

ALICE_ADD_COMMAND( memristor, "RRAM" );
ALICE_ADD_COMMAND( plim, "RRAM" );

ALICE_ADD_COMMAND( gen_npn_circuit, "Generator" );
ALICE_ADD_COMMAND( gen_trans_arith, "Generator" );

ALICE_ADD_COMMAND( abc, "Various" );
ALICE_ADD_COMMAND( bdd, "Various" );
ALICE_ADD_COMMAND( compress, "Various" );
ALICE_ADD_COMMAND( demo, "Various" );
ALICE_ADD_COMMAND( depth, "Various" );
ALICE_ADD_COMMAND( dsop, "Various" );
ALICE_ADD_COMMAND( expr, "Various" );
ALICE_ADD_COMMAND( output_noise, "Various" );
ALICE_ADD_COMMAND( print_io, "Various" );

}

//#include <addon_defines.hpp>

ALICE_MAIN( cirkit )

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
