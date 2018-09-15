#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/synthesis/esop_based.hpp>

namespace alice
{

class esopbs_command : public alice::command
{
public:
  esopbs_command( const environment::ptr& env ) : command( env, "ESOP-based synthesis" )
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
    auto& circs = store<small_mct_circuit_t>();
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }

    circs.current() = small_mct_circuit_t();
    tweedledum::esop_based_synthesis( circs.current(), tts.current() );
  }
};

ALICE_ADD_COMMAND( esopbs, "Synthesis" );

}
