#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <stack>
#include <vector>

enum class gate_type_t
{
  mct, mcy, mcz
};

// TODO move to library
class small_mct_circuit
{
public:
  using qubit = uint32_t;
  using gate = uint32_t;
  using node = std::list<gate>::const_iterator;

  enum class axis_t { X, Y, Z };

  explicit small_mct_circuit( uint32_t capacity = 32u, axis_t axis = axis_t::X )
    : _axis( axis )
  {
    assert( capacity <= 32u );

    for ( int32_t i = capacity - 1; i >= 0; --i )
    {
      _free_qubits.push( i );
    }
  }

  auto num_qubits() const
  {
    return _num_qubits;
  }

  auto num_gates() const
  {
    return _gates.size();
  }

  auto num_controls( gate const& g ) const
  {
    return __builtin_popcount( _gates[g].controls );
  }

  qubit allocate_qubit()
  {
    assert( !_free_qubits.empty() );
    ++_current_qubits;
    _num_qubits = std::max( _num_qubits, _current_qubits );
    auto top = _free_qubits.top();
    _free_qubits.pop();
    return top;
  }

  void free_qubit( qubit q )
  {
    --_current_qubits;
    _free_qubits.push( q );
  }

  template<typename Fn>
  void foreach_node( Fn&& fn ) const
  {
    for ( auto it = _nodes.begin(); it != _nodes.end(); ++it )
    {
      fn( it );
    }
  }

  gate const& get_gate( node const& n ) const
  {
    return *n;
  }

  template<typename Fn>
  void foreach_control( gate const& g, Fn&& fn ) const
  {
    auto const& mask = _gates[g];
    for ( auto i = 0u; i < _num_qubits; ++i )
    {
      if ( ( mask.controls >> i ) & 1 )
      {
        fn( i );
      }
    }
  }

  template<typename Fn>
  void foreach_target( gate const& g, Fn&& fn ) const
  {
    auto const& mask = _gates[g];
    for ( auto i = 0u; i < _num_qubits; ++i )
    {
      if ( ( mask.targets >> i ) & 1 )
      {
        fn( i );
      }
    }
  }

  node add_toffoli( uint32_t controls, uint32_t targets )
  {
    const auto n = _nodes.insert( _nodes.end(), _gates.size() );
    _gates.emplace_back( gate_mask_t{controls, targets} );

    return n;
  }

  node add_toffoli( std::vector<qubit> const& controls, std::vector<qubit> const& targets )
  {
    gate_mask_t mask;
    std::for_each( controls.begin(), controls.end(), [&]( auto q ) { mask.controls |= 1 << q; } );
    std::for_each( targets.begin(), targets.end(), [&]( auto q ) { mask.targets |= 1 << q; } );

    const auto n = _nodes.insert( _nodes.end(), _gates.size() );
    _gates.emplace_back( mask );

    return n;
  }

  gate_type_t gate_type( gate const& g ) const
  {
    switch ( _axis )
    {
      case axis_t::X: return gate_type_t::mct; break;
      case axis_t::Y: return gate_type_t::mcy; break;
      case axis_t::Z: return gate_type_t::mcz; break;
    }
  }

private:
  struct gate_mask_t
  {
    uint32_t controls{};
    uint32_t targets{};
  };

  axis_t _axis;
  uint32_t _current_qubits{0};
  uint32_t _num_qubits{0};
  std::stack<uint32_t> _free_qubits;
  std::list<gate> _nodes;
  std::vector<gate_mask_t> _gates;
};

