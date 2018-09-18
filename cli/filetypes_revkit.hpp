#include <alice/alice.hpp>

namespace alice
{
ALICE_ADD_FILE_TYPE_READ_ONLY( dotqc, ".QC" );
ALICE_ADD_FILE_TYPE_WRITE_ONLY( cirq, "Cirq" );
ALICE_ADD_FILE_TYPE_WRITE_ONLY( projectq, "ProjectQ" );
ALICE_ADD_FILE_TYPE_WRITE_ONLY( qasm, "QASM" );
ALICE_ADD_FILE_TYPE( quil, "Quil" );
ALICE_ADD_FILE_TYPE_WRITE_ONLY( quirk, "Quirk" );

ALICE_ADD_FILE_TYPE_READ_ONLY( aiger, "Aiger" );
ALICE_ADD_FILE_TYPE( bench, "BENCH" );
ALICE_ADD_FILE_TYPE( verilog, "Verilog" );
}
