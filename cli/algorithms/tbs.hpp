#include <alice/alice.hpp>

#include <iostream>

#include <td/algorithms/transformation_based_synthesis.hpp>

namespace alice
{

class tbs_command : public alice::command
{
public:
  tbs_command( const environment::ptr& env ) : command( env, "Transformation-based synthesis" )
  {
    add_flag( "-n,--new", "adds new store entry" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<perm_t>( env )};
  }

  void execute() override
  {
    auto const& perms = store<perm_t>();
    auto& circs = store<small_mct_circuit>();
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }

    auto f = perms.current(); // will be modified by tbs
    circs.current() = transformation_based_synthesis_bidirectional( f );
  }
};

ALICE_ADD_COMMAND( tbs, "Synthesis" );

}
