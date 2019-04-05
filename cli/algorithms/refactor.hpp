#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xmg_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class refactor_command : public cirkit::cirkit_command<refactor_command, mig_t, xmg_t>
{
public:
  refactor_command( environment::ptr& env ) : cirkit::cirkit_command<refactor_command, mig_t, xmg_t>( env, "Performs cut rewriting", "apply cut rewriting to {0}" )
  {
    add_option( "--max_pis", ps.max_pis, "maximum number of PIs in MFFC", true );
    add_option( "--strategy", strategy, "resynthesis strategy", true )->set_type_name( "strategy in {mignpn=0, akers=1}" );
    add_flag( "-z,--zero_gain", ps.allow_zero_gain, "enable zero-gain refactoring" );
    add_flag( "-p,--progress", ps.progress, "show progress" );
    add_flag( "-v,--verbose", ps.verbose, "show statistics" );
  }

  template<class Store>
  inline void execute_store()
  {
    switch ( strategy )
    {
    default:
    case 0:
    {
      if constexpr ( std::is_same_v<Store, mig_t> )
      {
        auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
        mockturtle::mig_npn_resynthesis resyn;
        mockturtle::refactoring( *mig_p, resyn, ps );
        *mig_p = cleanup_dangling( *mig_p );
      }
      else if constexpr ( std::is_same_v<Store, xmg_t> )
      {
        auto* xmg_p = static_cast<mockturtle::xmg_network*>( store<Store>().current().get() );
        mockturtle::xmg_npn_resynthesis resyn;
        mockturtle::refactoring( *xmg_p, resyn, ps );
        *xmg_p = cleanup_dangling( *xmg_p );
      }
    }
    break;
    case 1:
    {
      if constexpr ( std::is_same_v<Store, mig_t> )
      {
        auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
        mockturtle::akers_resynthesis<mockturtle::mig_network> resyn;
        mockturtle::refactoring( *mig_p, resyn, ps );
        *mig_p = cleanup_dangling( *mig_p );
      }
      else if constexpr ( std::is_same_v<Store, xmg_t> )
      {
        auto* xmg_p = static_cast<mockturtle::xmg_network*>( store<Store>().current().get() );
        mockturtle::akers_resynthesis<mockturtle::xmg_network> resyn;
        mockturtle::refactoring( *xmg_p, resyn, ps );
        *xmg_p = cleanup_dangling( *xmg_p );
      }
    }
    }
  }

  nlohmann::json log() const override
  {
   return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::refactoring_params ps;
  mockturtle::refactoring_stats st;
  unsigned strategy{0u};
};

ALICE_ADD_COMMAND( refactor, "Synthesis" )

} // namespace alice
