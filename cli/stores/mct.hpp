#include <alice/alice.hpp>

#include <fmt/format.h>

#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/networks/gates/mct_gate.hpp>
#include <tweedledum/networks/netlist.hpp>

namespace alice
{
  using small_mct_circuit_t = tweedledum::netlist<tweedledum::mct_gate>;

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

ALICE_WRITE_FILE(small_mct_circuit_t, projectq, circ, filename, cmd)
{
  write_projectq( circ, filename );
}

template<>
inline void write<small_mct_circuit_t, io_projectq_tag_t>( small_mct_circuit_t const& circ, std::ostream& os, const command& )
{
  write_projectq( circ, os );
}

}
