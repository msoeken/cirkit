#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/synthesis/transformation_based.hpp>

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
    auto& circs = store<qcircuit_t>();
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }

    auto f = perms.current(); // will be modified by tbs

    switch ( strategy )
    {
      case 0u:
        circs.current() = tweedledum::transformation_based_synthesis_multidirectional<qcircuit_t>( f );
        break;
      case 1u:
        circs.current() = tweedledum::transformation_based_synthesis_bidirectional<qcircuit_t>( f );
        break;
      case 2u:
        circs.current() = tweedledum::transformation_based_synthesis<qcircuit_t>( f );
        break;
    }
  }

private:
  unsigned strategy{0u};
};

ALICE_ADD_COMMAND( tbs, "Synthesis" );

}
