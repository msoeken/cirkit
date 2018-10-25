#include <alice/alice.hpp>

#include <fmt/format.h>

#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/print.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/io/write_cirq.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/io/write_qasm.hpp>
#include <tweedledum/networks/netlist.hpp>

namespace alice
{
using qcircuit_t = tweedledum::netlist<tweedledum::mcmt_gate>;

ALICE_ADD_STORE( qcircuit_t, "qc", "q", "Quantum circuit", "Quantum circuits" );

ALICE_DESCRIBE_STORE( qcircuit_t, circ )
{
  return fmt::format( "{} qubits, {} gates", circ.num_qubits(), circ.num_gates() );
}

ALICE_PRINT_STORE( qcircuit_t, os, circ )
{
  os << tweedledum::to_unicode( circ ) << "\n";
}

ALICE_PRINT_STORE_STATISTICS( qcircuit_t, os, circ )
{
  os << fmt::format( "Quantum circuit   gates = {}   qubits = {}\n", circ.num_gates(), circ.num_qubits() );
}

ALICE_LOG_STORE_STATISTICS( qcircuit_t, circ )
{
  return {
      {"qubits", circ.num_qubits()},
      {"gates", circ.num_gates()}};
}

ALICE_WRITE_FILE( qcircuit_t, cirq, circ, filename, cmd )
{
  write_cirq( circ, filename );
}

template<>
inline void write<qcircuit_t, io_cirq_tag_t>( qcircuit_t const& circ, std::ostream& os, const command& )
{
  write_cirq( circ, os );
}

ALICE_WRITE_FILE( qcircuit_t, projectq, circ, filename, cmd )
{
  write_projectq( circ, filename );
}

template<>
inline void write<qcircuit_t, io_projectq_tag_t>( qcircuit_t const& circ, std::ostream& os, const command& )
{
  write_projectq( circ, os );
}

ALICE_WRITE_FILE( qcircuit_t, quil, circ, filename, cmd )
{
  write_quil( circ, filename );
}

template<>
inline void write<qcircuit_t, io_quil_tag_t>( qcircuit_t const& circ, std::ostream& os, const command& )
{
  write_quil( circ, os );
}

ALICE_WRITE_FILE( qcircuit_t, qasm, circ, filename, cmd )
{
  write_qasm( circ, filename );
}

template<>
inline void write<qcircuit_t, io_qasm_tag_t>( qcircuit_t const& circ, std::ostream& os, const command& )
{
  write_qasm( circ, os );
}

} // namespace alice
