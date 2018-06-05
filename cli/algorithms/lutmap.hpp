#include <alice/alice.hpp>

#include <mockturtle/algorithms/lut_mapping.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class lutmap_command : public cirkit::cirkit_command<lutmap_command, aig_t, mig_t, klut_t>
{
public:
  lutmap_command( environment::ptr& env ) : cirkit::cirkit_command<lutmap_command, aig_t, mig_t, klut_t>( env, "Performs k-LUT mapping", "apply LUT-mapping to {0}" )
  {
    add_flag( "--nofun", "do not compute cut functions" );
  }

  template<class Store>
  inline void execute_store()
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
};

ALICE_ADD_COMMAND( lutmap, "Mapping" )

}
