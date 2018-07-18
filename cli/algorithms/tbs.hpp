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
    add_option( "--strategy", strategy, "algorithm strategy", true )->set_type_name( "strategy in {multidir=0, bidir=1, unidir=2}" );
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

    switch ( strategy )
    {
      case 0u:
        circs.current() = transformation_based_synthesis_multidirectional( f );
        break;
      case 1u:
        circs.current() = transformation_based_synthesis_bidirectional( f );
        break;
      case 2u:
        circs.current() = transformation_based_synthesis( f );
        break;
    }
  }

private:
  unsigned strategy{0u};
};

ALICE_ADD_COMMAND( tbs, "Synthesis" );

}
