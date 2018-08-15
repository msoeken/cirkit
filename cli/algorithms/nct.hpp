#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/mapping/nct.hpp>

namespace alice
{
using small_mct_circuit_t = tweedledum::netlist<tweedledum::mct_gate>;
using qc_circuit_t = tweedledum::dag_path<tweedledum::qc_gate>;

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
    auto const& circs = store<small_mct_circuit_t>();
    auto& qcircs = store<qc_circuit_t>();
    if ( qcircs.empty() || is_set( "new" ) )
    {
      qcircs.extend();
    }

    tweedledum::nct_mapping( qcircs.current(), circs.current() );
  }

private:
  uint32_t strategy{0u};
};

ALICE_ADD_COMMAND( nct, "Mapping" );

} // namespace alice
