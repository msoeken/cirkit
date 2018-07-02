#include <alice/alice.hpp>

#include <mockturtle/algorithms/lut_mapping.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class lut_mapping_command : public cirkit::cirkit_command<lut_mapping_command, aig_t, mig_t, klut_t>
{
public:
  lut_mapping_command( environment::ptr& env ) : cirkit::cirkit_command<lut_mapping_command, aig_t, mig_t, klut_t>( env, "Performs k-LUT mapping", "apply LUT-mapping to {0}" )
  {
    add_option( "-k,--lutsize", ps.cut_enumeration_ps.cut_size, "cut size", true );
    add_option( "--lutcount", ps.cut_enumeration_ps.cut_limit, "number of cuts per node", true );
    add_flag( "--nofun", "do not compute cut functions" );
  }

  template<class Store>
  inline void execute_store()
  {
    if ( is_set( "nofun" ) )
    {
      mockturtle::lut_mapping( *( store<Store>().current() ), ps );
    }
    else
    {
      mockturtle::lut_mapping<typename Store::element_type, true>( *( store<Store>().current() ), ps );
    }
  }

private:
  mockturtle::lut_mapping_params ps;
};

ALICE_ADD_COMMAND( lut_mapping, "Mapping" )

}
