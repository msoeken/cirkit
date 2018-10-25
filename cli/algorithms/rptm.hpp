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
    return {has_store_element<qcircuit_t>( env )};
  }

  void execute() override
  {
    auto& circs = store<qcircuit_t>();
    
    auto circ = tweedledum::relative_phase_mapping<qcircuit_t>( circs.current() );
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }
    
    circs.current() = circ;
  }
};

ALICE_ADD_COMMAND( rptm, "Mapping" );

}
