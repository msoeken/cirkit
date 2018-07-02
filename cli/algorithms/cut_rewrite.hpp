#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class cut_rewrite_command : public cirkit::cirkit_command<cut_rewrite_command, mig_t>
{
public:
  cut_rewrite_command( environment::ptr& env ) : cirkit::cirkit_command<cut_rewrite_command, mig_t>( env, "Performs cut rewriting", "apply cut rewriting to {0}" )
  {
    ps.cut_enumeration_ps.cut_size = 4;

    add_option( "-k,--lutsize", ps.cut_enumeration_ps.cut_size, "cut size", true );
    add_option( "--strategy", strategy, "resynthesis strategy", true )->set_type_name( "strategy in {mignpn=0, akers=1}" );
    add_flag( "-p,--progress", ps.progress, "show progress" );
  }

  template<class Store>
  inline void execute_store()
  {
    auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );

    switch ( strategy )
    {
    default:
    case 0:
    {
      mockturtle::mig_npn_resynthesis resyn;
      mockturtle::cut_rewriting( *mig_p, resyn, ps );
    }
    break;
    case 1:
    {
      mockturtle::akers_resynthesis resyn;
      mockturtle::cut_rewriting( *mig_p, resyn, ps );
    }
    }

    *mig_p = cleanup_dangling( *mig_p );
  }

private:
  mockturtle::cut_rewriting_params ps;
  unsigned strategy{0u};
};

ALICE_ADD_COMMAND( cut_rewrite, "Synthesis" )

} // namespace alice
