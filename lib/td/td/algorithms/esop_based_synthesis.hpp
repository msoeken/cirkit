#pragma once

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>

#include "../networks/small_mct_circuit.hpp"

// TODO move to library
small_mct_circuit esop_based_synthesis( kitty::dynamic_truth_table const& tt )
{
  const uint32_t num_qubits = tt.num_vars() + 1;
  small_mct_circuit circ( num_qubits );
  for ( auto i = 0u; i < num_qubits; ++i )
  {
    circ.allocate_qubit();
  }

  std::vector<small_mct_circuit::qubit> targets;
  targets.emplace_back( tt.num_vars() );

  for ( const auto& cube : esop_from_pprm( tt ) )
  {
    std::vector<small_mct_circuit::qubit> controls;
    for ( auto i = 0; i < tt.num_vars(); ++i )
    {
      if ( !cube.get_mask( i ) ) continue;

      assert( cube.get_bit( i ) );
      controls.emplace_back( i );
    }
    circ.add_toffoli( controls, targets );
  }

  return circ;
}
