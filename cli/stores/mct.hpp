#include <alice/alice.hpp>

#include <fmt/format.h>

#include <td/io/write_projectq.hpp>
#include <td/io/write_quil.hpp>
#include <td/networks/small_mct_circuit.hpp>

namespace alice
{

ALICE_ADD_STORE( small_mct_circuit, "mct", "c", "MCT circuit", "MCT circuits" );

ALICE_DESCRIBE_STORE( small_mct_circuit, circ )
{
  return fmt::format( "{} qubits, {} gates", circ.num_qubits(), circ.num_gates() );
}

ALICE_LOG_STORE_STATISTICS( small_mct_circuit, circ )
{
  return {
    {"qubits", circ.num_qubits()},
    {"gates", circ.num_gates()}
  };
}

ALICE_WRITE_FILE(small_mct_circuit, projectq, circ, filename, cmd)
{
  write_projectq( circ, filename );
}

ALICE_WRITE_FILE(small_mct_circuit, quil, circ, filename, cmd)
{
  write_quil( circ, filename );
}

template<>
inline void write<small_mct_circuit, io_projectq_tag_t>( small_mct_circuit const& circ, std::ostream& os, const command& )
{
  write_projectq( circ, os );
}

template<>
inline void write<small_mct_circuit, io_quil_tag_t>( small_mct_circuit const& circ, std::ostream& os, const command& )
{
  write_quil( circ, os );
}

}
