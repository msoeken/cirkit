#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/mapping/nct.hpp>

namespace alice
{

class nct_command : public alice::command
{
public:
  nct_command( const environment::ptr& env ) : command( env, "Maps MCT circuit into Quantum circuit with 2-controlled Toffoli gates" )
  {
    add_option( "-t,--controls_threshold", ps.controls_threshold, "maximum control line threshold" );
    add_flag( "-n,--new", "adds new store entry" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<qcircuit_t>( env )};
  }

  void execute() override
  {
    auto& circs = store<qcircuit_t>();

    auto circ = tweedledum::nct_mapping<qcircuit_t>( circs.current(), ps );
    if ( is_set( "new" ) )
    {
      circs.extend();
    }
    circs.current() = circ;
  }

private:
  tweedledum::nct_mapping_params ps;
};

ALICE_ADD_COMMAND( nct, "Mapping" );

} // namespace alice
