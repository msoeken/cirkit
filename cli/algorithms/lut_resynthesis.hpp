#include <alice/alice.hpp>

#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/dsd.hpp>
#include <mockturtle/algorithms/node_resynthesis/exact.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/shannon.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xmg_npn.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class lut_resynthesis_command : public cirkit::cirkit_command<lut_resynthesis_command, klut_t>
{
public:
  lut_resynthesis_command( environment::ptr& env ) : cirkit::cirkit_command<lut_resynthesis_command, klut_t>( env, "Performs LUT resynthesis", "apply LUT resynthesis to {0}" )
  {
    add_option( "--strategy", strategy, "resynthesis strategy", true )->set_type_name( "strategy in {dsd=0, shannon=1, dsd+exact=2, npn=3 (LUT size must be <= 4)}" );
    add_flag_helper<aig_t>( "store result in {0}" );
    add_flag_helper<xag_t>( "store result in {0}" );
    add_flag_helper<mig_t>( "store result in {0}" );
    add_flag_helper<xmg_t>( "store result in {0}" );
    add_new_option();
  }

  template<class Store>
  inline void execute_store()
  {
    switch ( strategy )
    {
    default:
    case 0:
      if ( execute_dsd<Store, aig_t>() ) return;
      if ( execute_dsd<Store, xag_t>() ) return;
      if ( execute_dsd<Store, mig_t>() ) return;
      if ( execute_dsd<Store, xmg_t>() ) return;
      break;
    case 1:
      if ( execute_shannon<Store, aig_t>() ) return;
      if ( execute_shannon<Store, xag_t>() ) return;
      if ( execute_shannon<Store, mig_t>() ) return;
      if ( execute_shannon<Store, xmg_t>() ) return;
      break;
    case 2:
      if ( execute_dsdexact<Store, aig_t>() ) return;
      if ( execute_dsdexact<Store, xag_t>() ) return;
      if ( execute_dsdexact<Store, mig_t>() ) return;
      if ( execute_dsdexact<Store, xmg_t>() ) return;
      break;
    case 3:
      if ( execute_npn<Store, aig_t>() ) return;
      if ( execute_npn<Store, xag_t>() ) return;
      if ( execute_npn<Store, mig_t>() ) return;
      if ( execute_npn<Store, xmg_t>() ) return;
      break;
    }
  }

private:
  template<class Store>
  bool is_store_set()
  {
    constexpr auto option = store_info<Store>::option;

    if ( is_set( option ) )
    {
      set_default_option<Store>();
      return true;
    }

    return false;
  }

  template<class Store, class Dest>
  bool execute_dsd()
  {
    const auto& ntk = *( store<Store>().current() );
    using network_type = typename Dest::element_type;
    using base_type = typename network_type::base_type;

    if ( is_store_set<Dest>() )
    {
      mockturtle::shannon_resynthesis<base_type> sresyn;
      mockturtle::dsd_resynthesis<base_type, decltype( sresyn )> resyn( sresyn );
      const auto dest = mockturtle::node_resynthesis<base_type>( ntk, resyn );
      network_type wrapped( dest );
      extend_if_new<Dest>();
      store<Dest>().current() = std::make_shared<network_type>( wrapped );

      return true;
    }

    return false;
  }

  template<class Store, class Dest>
  bool execute_shannon()
  {
    const auto& ntk = *( store<Store>().current() );
    using network_type = typename Dest::element_type;
    using base_type = typename network_type::base_type;

    if ( is_store_set<Dest>() )
    {
      mockturtle::shannon_resynthesis<base_type> resyn;
      const auto dest = mockturtle::node_resynthesis<base_type>( ntk, resyn );
      network_type wrapped( dest );
      extend_if_new<Dest>();
      store<Dest>().current() = std::make_shared<network_type>( wrapped );

      return true;
    }

    return false;
  }

  template<class Store, class Dest>
  bool execute_dsdexact()
  {
    const auto& ntk = *( store<Store>().current() );
    using network_type = typename Dest::element_type;
    using base_type = typename network_type::base_type;

    constexpr bool with_xor = std::is_same_v<Dest, xag_t> || std::is_same_v<Dest, xmg_t>;

    if ( is_store_set<Dest>() )
    {
      mockturtle::exact_resynthesis_params esps;
      esps.cache = exact_aig_cache;
      mockturtle::exact_aig_resynthesis<base_type> eresyn( with_xor, esps );
      mockturtle::dsd_resynthesis<base_type, decltype( eresyn )> resyn( eresyn );
      const auto dest = mockturtle::node_resynthesis<base_type>( ntk, resyn );
      network_type wrapped( dest );
      extend_if_new<Dest>();
      store<Dest>().current() = std::make_shared<network_type>( wrapped );

      return true;
    }

    return false;
  }

  template<class Store, class Dest>
  bool execute_npn()
  {
    const auto& ntk = *( store<Store>().current() );
    using network_type = typename Dest::element_type;
    using base_type = typename network_type::base_type;

    if ( is_store_set<Dest>() )
    {
      const auto dest = [&]() -> base_type {
        if constexpr ( std::is_same_v<Dest, xag_t> || std::is_same_v<Dest, aig_t> )
        {
          mockturtle::xag_npn_resynthesis<base_type> resyn;
          return mockturtle::node_resynthesis<base_type>( ntk, resyn );
        }
        else if constexpr ( std::is_same_v<Dest, mig_t> )
        {
          mockturtle::mig_npn_resynthesis resyn;
          return mockturtle::node_resynthesis<base_type>( ntk, resyn );
        }
        else if constexpr ( std::is_same_v<Dest, xmg_t> )
        {
          mockturtle::xmg_npn_resynthesis resyn;
          return mockturtle::node_resynthesis<base_type>( ntk, resyn );
        }
        else
        {
          assert( false );
        }
        return base_type();
      }();
      network_type wrapped( dest );
      extend_if_new<Dest>();
      store<Dest>().current() = std::make_shared<network_type>( wrapped );

      return true;
    }

    return false;
  }

private:
  unsigned strategy{0u};
  mockturtle::exact_resynthesis_params::cache_t exact_aig_cache{std::make_shared<mockturtle::exact_resynthesis_params::cache_map_t>()};
};

ALICE_ADD_COMMAND( lut_resynthesis, "Synthesis" )

} // namespace alice
