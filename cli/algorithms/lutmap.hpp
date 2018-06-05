#include <alice/alice.hpp>

#include <mockturtle/algorithms/lut_mapping.hpp>

namespace alice
{

class lutmap_command : public command
{
public:
  lutmap_command( environment::ptr& env ) : command( env, "Performs k-LUT mapping" )
  {
    add_flag( "-a,--aig", "apply LUT-mapping to AIG" );
    add_flag( "-m,--mig", "apply LUT-mapping to MIG" );
    add_flag( "-l,--lut", "apply LUT-mapping to LUT network" );

    add_flag( "--nofun", "do not compute cut functions" );
  }

  rules validity_rules() const
  {
    return {
      has_store_element_if_set<aig_t>( *this, env, "aig" ),
      has_store_element_if_set<mig_t>( *this, env, "mig" ),
      has_store_element_if_set<klut_t>( *this, env, "lut" )
    };
  }

  template<class Store>
  void _execute()
  {
    if ( is_set( "nofun" ) )
    {
      mockturtle::lut_mapping( *( env->store<Store>().current() ) );
    }
    else
    {
      mockturtle::lut_mapping<typename Store::element_type, true>( *( env->store<Store>().current() ) );
    }
  }

  void execute()
  {
    if ( is_set( "aig" ) )
    {
      _execute<aig_t>();
    }
    else if ( is_set( "mig" ) )
    {
      _execute<mig_t>();
    }
    else if ( is_set( "lut" ) )
    {
      _execute<klut_t>();
    }
  }
};

ALICE_ADD_COMMAND( lutmap, "Mapping" )

}
