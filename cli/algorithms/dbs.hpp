#include <alice/alice.hpp>

#include <iostream>

#include <td/algorithms/decomposition_based_synthesis.hpp>

namespace alice
{

class dbs_command : public alice::command
{
public:
  dbs_command( const environment::ptr& env ) : command( env, "Decomposition-based synthesis" )
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

    circs.current() = decomposition_based_synthesis( f );
  }
};

ALICE_ADD_COMMAND( dbs, "Synthesis" );

} // namespace alice
