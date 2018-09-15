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
    auto& circs = store<small_mct_circuit_t>();
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }

    auto f = perms.current(); // will be modified by tbs

    circs.current() = small_mct_circuit_t();
    switch ( strategy )
    {
      case 0u:
        tweedledum::transformation_based_synthesis_multidirectional( circs.current(), f );
        break;
      case 1u:
        tweedledum::transformation_based_synthesis_bidirectional( circs.current(), f );
        break;
      case 2u:
        tweedledum::transformation_based_synthesis( circs.current(), f );
        break;
    }
  }

private:
  unsigned strategy{0u};
};

ALICE_ADD_COMMAND( tbs, "Synthesis" );

}
