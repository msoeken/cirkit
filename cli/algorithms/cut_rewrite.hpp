#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/exact.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xmg_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class cut_rewrite_command : public cirkit::cirkit_command<cut_rewrite_command, aig_t, mig_t, xmg_t, klut_t>
{
public:
  cut_rewrite_command( environment::ptr& env ) : cirkit::cirkit_command<cut_rewrite_command, aig_t, mig_t, xmg_t, klut_t>( env, "Performs cut rewriting", "apply cut rewriting to {0}" )
  {
    ps.cut_enumeration_ps.cut_size = 4;

    add_option( "-k,--lutsize", ps.cut_enumeration_ps.cut_size, "cut size", true );
    add_option( "--lutcount", ps.cut_enumeration_ps.cut_limit, "cut limit", true );
    add_option( "--strategy", strategy, "resynthesis strategy", true )->set_type_name( "strategy in {mignpn=0, akers=1, exact=2, exact_aig=3, xmgnpn=4}" );
    add_flag( "-z", ps.allow_zero_gain, "enable zero-gain rewriting" );
    add_flag( "--multiple", "try multiple candidates if possible" );
    add_flag( "--dont_cares", "use don't cares if possible" );
    add_flag( "--clear_cache", "clear network cache" );
    add_option( "--exact_lutsize", exact_lutsize, "LUT size for exact resynthesis", true );
    add_option( "--conflict_limit", conflict_limit, "conflict limit for exact resynthesis", true );
    add_flag( "-p,--progress", ps.progress, "show progress" );
    add_flag( "-v,--verbose", ps.verbose, "show statistics" );
  }

  template<class Store>
  inline void execute_store()
  {
    ps.use_dont_cares = is_set( "dont_cares" );
    switch ( strategy )
    {
    default:
    case 0:
    {
      if constexpr ( std::is_same_v<Store, mig_t> )
      {
        auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
        mockturtle::mig_npn_resynthesis resyn( is_set( "multiple" ) );
        mockturtle::cut_rewriting( *mig_p, resyn, ps, &st );
        *mig_p = cleanup_dangling( *mig_p );
      }
    }
    break;
    case 1:
    {
      if constexpr ( std::is_same_v<Store, mig_t> )
      {
        auto* mig_p = static_cast<mockturtle::mig_network*>( store<Store>().current().get() );
        mockturtle::akers_resynthesis resyn;
        mockturtle::cut_rewriting( *mig_p, resyn, ps, &st );
        *mig_p = cleanup_dangling( *mig_p );
      }
    }
    break;
    case 2:
    {
      if constexpr ( std::is_same_v<Store, klut_t> )
      {
        auto* klut_p = static_cast<mockturtle::klut_network*>( store<Store>().current().get() );
        if ( is_set( "clear_cache" ) )
        {
          exact_cache = std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>();
        }
        mockturtle::exact_resynthesis_params esps;
        esps.cache = exact_cache;
        esps.conflict_limit = conflict_limit;
        mockturtle::exact_resynthesis resyn( exact_lutsize, esps );
        mockturtle::cut_rewriting( *klut_p, resyn, ps, &st );
        *klut_p = cleanup_dangling( *klut_p );
      }
    }
    break;
    case 3:
    {
      if constexpr ( std::is_same_v<Store, aig_t> )
      {
        auto* aig_p = static_cast<mockturtle::aig_network*>( store<Store>().current().get() );
        if ( is_set( "clear_cache" ) )
        {
          exact_aig_cache = std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>();
        }
        mockturtle::exact_resynthesis_params esps;
        esps.cache = exact_cache;
        esps.conflict_limit = conflict_limit;
        mockturtle::exact_aig_resynthesis resyn( esps );
        mockturtle::cut_rewriting( *aig_p, resyn, ps, &st );
        *aig_p = cleanup_dangling( *aig_p );
      }
    }
    break;
    case 4:
    {
      if constexpr ( std::is_same_v<Store, xmg_nt> )
      {
        auto* xmg_p = static_cast<mockturtle::xmg_network*>( store<Store>().current().get() );
        mockturtle::xmg_npn_resynthesis resyn;
        mockturtle::cut_rewriting( *xmg_p, resyn, ps, &st );
        *xmg_p = cleanup_dangling( *xmg_p );
      }
    }
    break;
    }
  }

  nlohmann::json log() const override
  {
    return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::cut_rewriting_params ps;
  mockturtle::cut_rewriting_stats st;
  mockturtle::exact_resynthesis_params::cache_t exact_cache{std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>()}; 
  mockturtle::exact_resynthesis_params::cache_t exact_aig_cache{std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>()}; 
  unsigned strategy{0u};
  unsigned exact_lutsize{3u};
  int conflict_limit{0};
};

ALICE_ADD_COMMAND( cut_rewrite, "Synthesis" )

} // namespace alice
