#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/mapping/relative_phase.hpp>

namespace alice
{

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
    
    qcs.current() = tweedledum::relative_phase_mapping<qc_circuit_t>( circs.current() );
  }
};

ALICE_ADD_COMMAND( rptm, "Mapping" );

}
