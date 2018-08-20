#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/mapping/nct.hpp>

namespace alice
{
using small_mct_circuit_t = tweedledum::netlist<tweedledum::mct_gate>;

class nct_command : public alice::command
{
public:
  nct_command( const environment::ptr& env ) : command( env, "Maps MCT circuit into Quantum circuit with 2-controlled Toffoli gates" )
  {
    add_flag( "-n,--new", "adds new store entry" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<small_mct_circuit_t>( env )};
  }

  void execute() override
  {
    auto& circs = store<small_mct_circuit_t>();

    small_mct_circuit_t circ;
    tweedledum::nct_mapping( circ, circs.current() );
    if ( is_set( "new" ) )
    {
      circs.extend();
    }
    circs.current() = circ;
  }
};

ALICE_ADD_COMMAND( nct, "Mapping" );

} // namespace alice
