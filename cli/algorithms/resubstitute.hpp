#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/resubstitution.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class resub_command : public cirkit::cirkit_command<resub_command, mig_t>
{
public:
  resub_command( environment::ptr& env ) : cirkit::cirkit_command<resub_command, mig_t>( env, "Performs resubstitution", "apply resubstitution to {0}" )
  {
    add_option( "--max_pis", ps.max_pis, "maximum number of PIs in reconvergence-driven window", true );
    add_option( "--max_nnodes", ps.max_nodes, "maximum number of nodes in reconvergence-driven window", true );
    add_flag( "-p,--progress", ps.progress, "show progress" );
  }

  template<class Store>
  inline void execute_store()
  {
    auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
    mockturtle::resubstitution( *mig_p, ps );
    *mig_p = cleanup_dangling( *mig_p );
  }

private:
  mockturtle::resubstitution_params ps;
};

ALICE_ADD_COMMAND( resub, "Synthesis" )

} // namespace alice
