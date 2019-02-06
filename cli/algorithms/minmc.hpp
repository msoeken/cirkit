#include <alice/alice.hpp>

#include <memory>
#include <string>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc.hpp>
#include <mockturtle/utils/stopwatch.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class minmc_command : public cirkit::cirkit_command<minmc_command, xag_t>
{
public:
  minmc_command( environment::ptr& env ) : cirkit::cirkit_command<minmc_command, xag_t>( env, "Optimizes for multiplicative complexity", "applies optimization to {0}" )
  {
    opts.add_flag( "--progress,-p", ps.progress, "show progress" );
    opts.add_option( "--load", db, "load database" );
    opts.add_flag( "--verbose,-v", ps.verbose, "be verbose" );
  }

  rules validity_rules() const override
  {
    return {
      {[this]() { return store<xag_t>().current_index() >= 0 || is_set( "load" ); }, "no current XAG available" },
      {[this]() { return store<xag_t>().current_index() < 0 || is_set( "load" ) || resyn; }, "no database loaded" }
    };
  }

  template<class Store>
  inline void execute_store()
  {
    if ( is_set( "load" ) )
    {
      resyn.reset( new mockturtle::xag_minmc_resynthesis( db ) );
    }

    if ( store<xag_t>().current_index() >= 0 )
    {
      resyn->ps.print_stats = ps.verbose;

      auto* xag_p = static_cast<mockturtle::xag_network*>( store<xag_t>().current().get() );
      mockturtle::cut_rewriting( *xag_p, *resyn, ps, &st );
      *xag_p = mockturtle::cleanup_dangling( *xag_p );
    }
  }

  nlohmann::json log() const override
  {
    return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  std::string db;
  std::shared_ptr<mockturtle::xag_minmc_resynthesis> resyn;
  mockturtle::cut_rewriting_params ps;
  mockturtle::cut_rewriting_stats st;
};

ALICE_ADD_COMMAND( minmc, "Synthesis" )

} // namespace alice
