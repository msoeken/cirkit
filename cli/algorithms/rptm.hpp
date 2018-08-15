#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/mapping/relative_phase.hpp>
#include <tweedledum/networks/dag_path.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>

namespace alice
{
  using small_mct_circuit_t = tweedledum::netlist<tweedledum::mct_gate>;
  using qc_circuit_t = tweedledum::dag_path<tweedledum::qc_gate>;

class rptm_command : public alice::command
{
public:
  rptm_command( const environment::ptr& env ) : command( env, "Relative-phase Toffoli mapping" )
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
    auto& qcs = store<qc_circuit_t>();
    
    if ( qcs.empty() || is_set( "new" ) )
    {
      qcs.extend();
    }
    
    tweedledum::relative_phase_mapping( qcs.current(), circs.current() );
  }
};

ALICE_ADD_COMMAND( rptm, "Mapping" );

}
