#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/resubstitution.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class resub_command : public cirkit::cirkit_command<resub_command, aig_t, mig_t, xag_t, xmg_t>
{
public:
  resub_command( environment::ptr& env ) : cirkit::cirkit_command<resub_command, aig_t, mig_t, xag_t, xmg_t>( env, "Performs resubstitution", "apply resubstitution to {0}" )
  {
    add_option( "--max_pis", ps.max_pis, "maximum number of PIs in reconvergence-driven window", true );
    add_option( "--max_nodes", ps.max_nodes, "maximum number of nodes in reconvergence-driven window", true );
    add_option( "--max_compare", ps.max_compare, "maximum number of nodes compared per candidate node", true );
    add_option( "--depth", ps.max_inserts, "maximum number of nodes inserted by resubstitution", true );
    add_flag( "-w,--window", ps.extend, "extend reconvergence-driven cut to window" );
    add_flag( "-z,--zero-gain", ps.zero_gain, "enable zero-gain resubstitution" );
    add_flag( "-p,--progress", ps.progress, "show progress" );
    add_flag( "-v,--verbose", ps.verbose, "show statistics" );
  }

  template<class Store>
  inline void execute_store()
  {
    /* aig */
    if constexpr ( std::is_same_v<Store, aig_t> )
    {
      auto* aig_p = static_cast<mockturtle::aig_network*>( store<Store>().current().get() );
      mockturtle::resubstitution( *aig_p, ps );
      *aig_p = cleanup_dangling( *aig_p );
    }

    /* mig */
    if constexpr ( std::is_same_v<Store, mig_t> )
    {
      auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
      mockturtle::resubstitution( *mig_p, ps );
      *mig_p = cleanup_dangling( *mig_p );
    }

    /* xag */
    if constexpr ( std::is_same_v<Store, xag_t> )
    {
      auto* xag_p = static_cast<mockturtle::xag_network*>( store<Store>().current().get() );
      mockturtle::resubstitution( *xag_p, ps );
      *xag_p = cleanup_dangling( *xag_p );
    }

    /* xmg */
    if constexpr ( std::is_same_v<Store, xmg_t> )
    {
      auto* xmg_p = static_cast<mockturtle::xmg_network*>( store<Store>().current().get() );
      mockturtle::resubstitution( *xmg_p, ps );
      *xmg_p = cleanup_dangling( *xmg_p );
    }
  }

private:
  mockturtle::resubstitution_params ps;
};

ALICE_ADD_COMMAND( resub, "Synthesis" )

} // namespace alice
