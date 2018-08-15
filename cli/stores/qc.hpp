#include <alice/alice.hpp>

#include <fmt/format.h>

#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/dag_path.hpp>

namespace alice
{
  using qc_circuit_t = tweedledum::dag_path<tweedledum::qc_gate>;

ALICE_ADD_STORE( qc_circuit_t, "qc", "q", "Quantum circuit", "Quantum circuits" );

ALICE_DESCRIBE_STORE( qc_circuit_t, circ )
{
  return fmt::format( "{} qubits, {} gates", circ.num_qubits(), circ.num_gates() );
}

ALICE_LOG_STORE_STATISTICS( qc_circuit_t, circ )
{
  return {
    {"qubits", circ.num_qubits()},
    {"gates", circ.num_gates()}
  };
}

//ALICE_WRITE_FILE(qc_circuit_t, projectq, circ, filename, cmd)
//{
//  write_projectq( circ, filename );
//}

ALICE_WRITE_FILE(qc_circuit_t, quil, circ, filename, cmd)
{
  write_quil( circ, filename );
}

//template<>
//inline void write<qc_circuit_t, io_projectq_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
//{
//  write_projectq( circ, os );
//}

template<>
inline void write<qc_circuit_t, io_quil_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
{
  write_quil( circ, os );
}

}
