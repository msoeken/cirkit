#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/resubstitution.hpp>
#include <mockturtle/algorithms/aig_resub.hpp>
#include <mockturtle/algorithms/mig_resub.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class resub_command : public cirkit::cirkit_command<resub_command, aig_t, mig_t, xag_t, xmg_t>
{
public:
  resub_command( environment::ptr& env ) : cirkit::cirkit_command<resub_command, aig_t, mig_t, xag_t, xmg_t>( env, "Performs resubstitution", "apply resubstitution to {0}" )
  {
    add_option( "--max_pis", ps.max_pis, "maximum number of PIs in reconvergence-driven window", true );
    add_option( "--max_divisors", ps.max_divisors, "maximum number of divisors to consider", true );
    add_option( "--skip_fanout_limit_for_roots", ps.skip_fanout_limit_for_roots, "maximum fanout of a node to be considered as root", true );
    add_option( "--skip_fanout_limit_for_divisors", ps.skip_fanout_limit_for_divisors, "maximum fanout of a node to be considered as divisor", true );
    add_option( "--depth", ps.max_inserts, "maximum number of nodes inserted by resubstitution", true );
    // add_flag( "-z,--zero_gain", ps.zero_gain, "enable zero-gain resubstitution" );
    add_flag( "-p,--progress", ps.progress, "show progress" );
    add_flag( "-v,--verbose", ps.verbose, "show statistics" );
  }

  template<class Store>
  inline void execute_store()
  {
    if constexpr ( std::is_same_v<Store, aig_t> )
    {
      using view_t = mockturtle::depth_view<mockturtle::fanout_view<mockturtle::aig_network>>;
      auto* aig_p = static_cast<mockturtle::aig_network*>( store<Store>().current().get() );
      mockturtle::fanout_view<mockturtle::aig_network> fanout_view{*aig_p};
      view_t resub_view{fanout_view};
      mockturtle::aig_resubstitution( resub_view, ps, &st );
      *aig_p = cleanup_dangling( *aig_p );
    }
    else if constexpr ( std::is_same_v<Store, mig_t> )
    {
      using view_t = mockturtle::depth_view<mockturtle::fanout_view<mockturtle::mig_network>>;
      auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
      mockturtle::fanout_view<mockturtle::mig_network> fanout_view{*mig_p};
      view_t resub_view{fanout_view};
      mockturtle::mig_resubstitution( resub_view, ps, &st );
      *mig_p = cleanup_dangling( *mig_p );
    }
    else if constexpr ( std::is_same_v<Store, xag_t> )
    {
      using view_t = mockturtle::depth_view<mockturtle::fanout_view<mockturtle::xag_network>>;
      auto* xag_p = static_cast<mockturtle::xag_network*>( store<Store>().current().get() );
      mockturtle::fanout_view<mockturtle::xag_network> fanout_view{*xag_p};
      view_t resub_view{fanout_view};
      mockturtle::resubstitution( resub_view, ps, &st );
      *xag_p = cleanup_dangling( *xag_p );
    }
    else if constexpr ( std::is_same_v<Store, xmg_t> )
    {
      using view_t = mockturtle::depth_view<mockturtle::fanout_view<mockturtle::xmg_network>>;
      auto* xmg_p = static_cast<mockturtle::xmg_network*>( store<Store>().current().get() );
      mockturtle::fanout_view<mockturtle::xmg_network> fanout_view{*xmg_p};
      view_t resub_view{fanout_view};
      mockturtle::resubstitution( resub_view, ps, &st );
      *xmg_p = cleanup_dangling( *xmg_p );
    }
  }

  nlohmann::json log() const override
  {
   return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::resubstitution_params ps;
  mockturtle::resubstitution_stats st;
};

ALICE_ADD_COMMAND( resub, "Synthesis" )

} // namespace alice
