#include <alice/alice.hpp>

#include <mockturtle/algorithms/lut_resynthesis.hpp>
#include <mockturtle/algorithms/lut_resynthesis/mig_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class miglut_command : public cirkit::cirkit_command<miglut_command, aig_t, mig_t, klut_t>
{
public:
  miglut_command( environment::ptr& env ) : cirkit::cirkit_command<miglut_command, aig_t, mig_t, klut_t>( env, "Performs LUT resynthesis", "apply LUT resynthesis to {0}" )
  {
    add_new_option();
  }

  template<class Store>
  inline void execute_store()
  {
    const auto& ntk = *( store<Store>().current() );
    mockturtle::mig_npn_resynthesis resyn;
    extend_if_new<mig_t>();
    const auto mig = mockturtle::lut_resynthesis<mockturtle::mig_network>( ntk, resyn );
    store<mig_t>().current() = std::make_shared<mig_nt>( mig );
  }
};

ALICE_ADD_COMMAND( miglut, "Synthesis" )

}
