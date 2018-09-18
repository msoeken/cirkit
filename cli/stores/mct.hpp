#include <alice/alice.hpp>

#include <fmt/format.h>

#include <tweedledum/io/quil.hpp>
#include <tweedledum/io/write_cirq.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/io/write_qasm.hpp>
#include <tweedledum/networks/gates/pmct_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

namespace alice
{
  using small_mct_circuit_t = tweedledum::netlist<tweedledum::pmct_gate>;

ALICE_ADD_STORE( small_mct_circuit_t, "mct", "c", "MCT circuit", "MCT circuits" );

ALICE_DESCRIBE_STORE( small_mct_circuit_t, circ )
{
  return fmt::format( "{} qubits, {} gates", circ.num_qubits(), circ.num_gates() );
}

ALICE_LOG_STORE_STATISTICS( small_mct_circuit_t, circ )
{
  return {
    {"qubits", circ.num_qubits()},
    {"gates", circ.num_gates()}
  };
}

ALICE_WRITE_FILE(small_mct_circuit_t, cirq, circ, filename, cmd)
{
  write_cirq( circ, filename );
}

template<>
inline void write<small_mct_circuit_t, io_cirq_tag_t>( small_mct_circuit_t const& circ, std::ostream& os, const command& )
{
  write_cirq( circ, os );
}

ALICE_WRITE_FILE(small_mct_circuit_t, projectq, circ, filename, cmd)
{
  write_projectq( circ, filename );
}

template<>
inline void write<small_mct_circuit_t, io_projectq_tag_t>( small_mct_circuit_t const& circ, std::ostream& os, const command& )
{
  write_projectq( circ, os );
}

ALICE_WRITE_FILE(small_mct_circuit_t, quil, circ, filename, cmd)
{
  write_quil( circ, filename );
}

template<>
inline void write<small_mct_circuit_t, io_quil_tag_t>( small_mct_circuit_t const& circ, std::ostream& os, const command& )
{
  write_quil( circ, os );
}

ALICE_WRITE_FILE(small_mct_circuit_t, qasm, circ, filename, cmd)
{
  write_qasm( circ, filename );
}

template<>
inline void write<small_mct_circuit_t, io_qasm_tag_t>( small_mct_circuit_t const& circ, std::ostream& os, const command& )
{
  write_qasm( circ, os );
}

}
