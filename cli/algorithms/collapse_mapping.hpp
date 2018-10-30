#include <alice/alice.hpp>

#include <mockturtle/algorithms/collapse_mapped.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class collapse_mapping_command : public cirkit::cirkit_command<collapse_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>
{
public:
  collapse_mapping_command( environment::ptr& env ) : cirkit::cirkit_command<collapse_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Collapses mapped network", "collapse mapped {}" )
  {
    add_new_option();
  }

  template<class Store>
  void execute_store()
  {
    const auto ntk = mockturtle::collapse_mapped_network<mockturtle::klut_network>( *( env->store<Store>().current() ) );
    if ( ntk )
    {
      extend_if_new<klut_t>();
      store<klut_t>().current() = std::make_shared<klut_nt>( *ntk );
    }
    else
    {
      env->out() << "[w] network has no mapping\n";
    }
  }
};

ALICE_ADD_COMMAND( collapse_mapping, "Mapping" )

}
