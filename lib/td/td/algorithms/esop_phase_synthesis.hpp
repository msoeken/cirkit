#pragma once

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>

#include "../networks/small_mct_circuit.hpp"

// TODO move to library
small_mct_circuit esop_phase_synthesis( kitty::dynamic_truth_table const& tt )
{
  const uint32_t num_qubits = tt.num_vars();
  small_mct_circuit circ( num_qubits, small_mct_circuit::axis_t::Z );
  for ( auto i = 0u; i < num_qubits; ++i )
  {
    circ.allocate_qubit();
  }

  for ( const auto& cube : esop_from_pprm( tt ) )
  {
    std::vector<small_mct_circuit::qubit> controls, targets;
    for ( auto i = 0; i < tt.num_vars(); ++i )
    {
      if ( !cube.get_mask( i ) ) continue;

      assert( cube.get_bit( i ) );

      if ( targets.empty() )
      {
        targets.emplace_back( i );
      }
      else
      {
        controls.emplace_back( i );
      }
    }

    if ( !targets.empty() )
    {
      circ.add_toffoli( controls, targets );
    }
  }

  return circ;
}
