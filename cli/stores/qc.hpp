#include <alice/alice.hpp>

#include <fmt/format.h>

#include <tweedledee/dotqc/dotqc.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/dotqc.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/io/write_cirq.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/io/write_qasm.hpp>
#include <tweedledum/io/write_quirk.hpp>
#include <tweedledum/networks/gg_network.hpp>

namespace alice
{
using qc_circuit_t = tweedledum::gg_network<tweedledum::mcst_gate>;

ALICE_ADD_STORE( qc_circuit_t, "qc", "q", "Quantum circuit", "Quantum circuits" );

ALICE_DESCRIBE_STORE( qc_circuit_t, circ )
{
  return fmt::format( "{} qubits, {} gates", circ.num_qubits(), circ.num_gates() );
}

ALICE_LOG_STORE_STATISTICS( qc_circuit_t, circ )
{
  return {
      {"qubits", circ.num_qubits()},
      {"gates", circ.num_gates()}};
}

ALICE_WRITE_FILE( qc_circuit_t, cirq, circ, filename, cmd )
{
  write_cirq( circ, filename );
}

template<>
inline void write<qc_circuit_t, io_cirq_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
{
  write_cirq( circ, os );
}

ALICE_WRITE_FILE( qc_circuit_t, projectq, circ, filename, cmd )
{
  write_projectq( circ, filename );
}

template<>
inline void write<qc_circuit_t, io_projectq_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
{
  write_projectq( circ, os );
}

ALICE_READ_FILE( qc_circuit_t, quil, filename, cmd )
{
  qc_circuit_t circ;
  read_quil_file( circ, filename );
  return circ;
}

ALICE_WRITE_FILE( qc_circuit_t, quil, circ, filename, cmd )
{
  write_quil( circ, filename );
}

template<>
inline void write<qc_circuit_t, io_quil_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
{
  write_quil( circ, os );
}

ALICE_WRITE_FILE( qc_circuit_t, qasm, circ, filename, cmd )
{
  write_qasm( circ, filename );
}

template<>
inline void write<qc_circuit_t, io_qasm_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
{
  write_qasm( circ, os );
}

ALICE_READ_FILE( qc_circuit_t, dotqc, filename, cmd )
{
  qc_circuit_t circ;

  tweedledum::dotqc_reader reader( circ );
  tweedledee::dotqc_read( filename, reader, tweedledum::identify_gate_kind() );

  return circ;
}

ALICE_WRITE_FILE( qc_circuit_t, quirk, circ, filename, cmd )
{
  write_quirk_encoded_json( circ, filename );
}

template<>
inline void write<qc_circuit_t, io_quirk_tag_t>( qc_circuit_t const& circ, std::ostream& os, const command& )
{
  write_quirk_encoded_json( circ, os );
}

} // namespace alice
