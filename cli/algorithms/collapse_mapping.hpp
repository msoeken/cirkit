#include <alice/alice.hpp>

#include <mockturtle/algorithms/collapse_mapped.hpp>
#include <mockturtle/views/names_view.hpp>

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
    mockturtle::klut_network ntk;
    mockturtle::names_view<mockturtle::klut_network> named_ntk( ntk );
    bool success = mockturtle::collapse_mapped_network<mockturtle::names_view<mockturtle::klut_network>>( named_ntk, *( env->store<Store>().current() ) );
    if ( success )
    {
      extend_if_new<klut_t>();
      store<klut_t>().current() = std::make_shared<klut_nt>( named_ntk );

      set_default_option<klut_t>();
    }
    else
    {
      env->out() << "[w] network has no mapping\n";
    }
  }
};

ALICE_ADD_COMMAND( collapse_mapping, "Mapping" )

}
