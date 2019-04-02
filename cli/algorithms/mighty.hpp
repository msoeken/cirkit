#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>
#include <mockturtle/views/depth_view.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class mighty_command : public cirkit::cirkit_command<mighty_command, mig_t>
{
public:
  mighty_command( environment::ptr& env ) : cirkit::cirkit_command<mighty_command, mig_t>( env, "Performs algebraic MIG rewriting", "applies algebraic MIG rewriting to {0}" )
  {
    opts.add_set( "--strategy", ps.strategy,
                  {mockturtle::mig_algebraic_depth_rewriting_params::dfs,
                   mockturtle::mig_algebraic_depth_rewriting_params::aggressive,
                   mockturtle::mig_algebraic_depth_rewriting_params::selective},
                  "optimization strategy", true )->set_type_name( "enum/strategy in {dfs=0, aggressive=1, selective=2}" );
    opts.add_flag( "--area_aware", "do not increase area" );
  }

  template<class Store>
  inline void execute_store()
  {
    auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
    mockturtle::depth_view depth_mig{*mig_p};
    ps.allow_area_increase = !is_set( "area_aware" );
    mockturtle::mig_algebraic_depth_rewriting( depth_mig, ps );
    *mig_p = mockturtle::cleanup_dangling( *mig_p );
  }

  nlohmann::json log() const override
  {
   return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::mig_algebraic_depth_rewriting_params ps;
  mockturtle::mig_algebraic_depth_rewriting_stats st;
};

ALICE_ADD_COMMAND( mighty, "Synthesis" )

} // namespace alice
