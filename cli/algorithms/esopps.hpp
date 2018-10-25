#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/synthesis/esop_phase.hpp>

namespace alice
{

class esopps_command : public alice::command
{
public:
  esopps_command( const environment::ptr& env ) : command( env, "ESOP phase synthesis" )
  {
    add_flag( "-n,--new", "adds new store entry" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<kitty::dynamic_truth_table>( env )};
  }

  void execute() override
  {
    auto const& tts = store<kitty::dynamic_truth_table>();
    auto& circs = store<qcircuit_t>();
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }

    circs.current() = qcircuit_t();
    tweedledum::esop_phase_synthesis( circs.current(), tts.current() );
  }
};

ALICE_ADD_COMMAND( esopps, "Synthesis" );

}
