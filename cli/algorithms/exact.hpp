#include <alice/alice.hpp>

#include <algorithm>
#include <vector>

#include <mockturtle/algorithms/node_resynthesis/exact.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class exact_command : public cirkit::cirkit_command<exact_command, aig_t, xag_t, klut_t>
{
public:
  exact_command( environment::ptr& env ) : cirkit::cirkit_command<exact_command, aig_t, xag_t, klut_t>( env, "Finds optimum network", "find optimum {}" )
  {
    add_flag( "--clear_cache", "clear network cache" );
    add_option( "--lutsize", lutsize, "LUT size for k-LUT synthesis", true );
    add_option( "--conflict_limit", conflict_limit, "conflict limit", true );
    add_new_option();
  }

  rules validity_rules() const override
  {
    return {has_store_element<kitty::dynamic_truth_table>( env )};
  }

  template<class Store>
  void execute_store()
  {
    const auto& tt = store<kitty::dynamic_truth_table>().current();

    if constexpr ( std::is_same_v<Store, aig_t> || std::is_same_v<Store, xag_t> )
    {
      using network_type = typename Store::element_type;
      using base_type = typename network_type::base_type;

      if ( is_set( "clear_cache" ) )
      {
        exact_aig_cache = std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>();
      }

      mockturtle::exact_resynthesis_params esps;
      esps.cache = exact_aig_cache;
      esps.conflict_limit = conflict_limit;
      constexpr bool with_xor = std::is_same_v<Store, xag_t>;
      mockturtle::exact_aig_resynthesis<base_type> resyn( with_xor, esps );

      base_type ntk;
      std::vector<typename base_type::signal> pis( tt.num_vars() );
      std::generate( pis.begin(), pis.end(), [&]() { return ntk.create_pi(); } );

      resyn( ntk, tt, pis.begin(), pis.end(), [&]( auto const& f ) { ntk.create_po( f ); } );

      if ( ntk.num_pos() == 1u )
      {
        extend_if_new<Store>();
        store<Store>().current() = std::make_shared<network_type>( ntk );
        set_default_option<Store>();
      }
    }
    else /* klut */
    {
      if ( is_set( "clear_cache" ) )
      {
        exact_cache = std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>();
      }

      mockturtle::exact_resynthesis_params esps;
      esps.cache = exact_cache;
      esps.conflict_limit = conflict_limit;
      mockturtle::exact_resynthesis resyn( lutsize, esps );

      mockturtle::klut_network ntk;
      std::vector<mockturtle::klut_network::signal> pis( tt.num_vars() );
      std::generate( pis.begin(), pis.end(), [&]() { return ntk.create_pi(); } );

      resyn( ntk, tt, pis.begin(), pis.end(), [&]( auto const& f ) { ntk.create_po( f ); } );

      if ( ntk.num_pos() == 1u )
      {
        extend_if_new<klut_t>();
        store<klut_t>().current() = std::make_shared<klut_nt>( ntk );
        set_default_option<Store>();
      }
    }
  }

private:
  mockturtle::exact_resynthesis_params::cache_t exact_cache{std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>()};
  mockturtle::exact_resynthesis_params::cache_t exact_aig_cache{std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>()};
  unsigned lutsize{3u};
  int conflict_limit{0};
};

ALICE_ADD_COMMAND( exact, "Synthesis" )

} // namespace alice
