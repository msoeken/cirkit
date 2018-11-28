#include <alice/alice.hpp>

#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class lut_resynthesis_command : public cirkit::cirkit_command<lut_resynthesis_command, klut_t>
{
public:
  lut_resynthesis_command( environment::ptr& env ) : cirkit::cirkit_command<lut_resynthesis_command, klut_t>( env, "Performs LUT resynthesis", "apply LUT resynthesis to {0}" )
  {
    add_option( "--strategy", strategy, "resynthesis strategy", true )->set_type_name( "strategy in {mignpn=0, akers=1}" );
    add_new_option();
  }

  template<class Store>
  inline void execute_store()
  {
    const auto& ntk = *( store<Store>().current() );

    extend_if_new<mig_t>();

    const auto mig = [&]() {
      switch ( strategy )
      {
      default:
      case 0:
      {
        mockturtle::mig_npn_resynthesis resyn;
        return mockturtle::node_resynthesis<mockturtle::mig_network>( ntk, resyn );
      }
      break;
      case 1:
      {
        mockturtle::akers_resynthesis<mockturtle::mig_network> resyn;
        return mockturtle::node_resynthesis<mockturtle::mig_network>( ntk, resyn );
      }
      }
    }();

    store<mig_t>().current() = std::make_shared<mig_nt>( mig );
  }

private:
  unsigned strategy{0u};
};

ALICE_ADD_COMMAND( lut_resynthesis, "Synthesis" )

} // namespace alice
