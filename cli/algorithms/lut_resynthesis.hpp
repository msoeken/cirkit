/* CirKit: A circuit toolkit
 * Copyright (C) 2017-2019  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
    using named_base_type = typename mockturtle::names_view<base_type>;

    if ( is_store_set<Dest>() )
    {
      mockturtle::shannon_resynthesis<named_base_type> sresyn;
      mockturtle::dsd_resynthesis<named_base_type, decltype( sresyn )> resyn( sresyn );
      base_type dest;
      named_base_type named_dest( dest );
      mockturtle::node_resynthesis( named_dest, ntk, resyn );
      mockturtle::names_view<network_type> wrapped( named_dest );
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
    using named_base_type = typename mockturtle::names_view<base_type>;

    if ( is_store_set<Dest>() )
    {
      mockturtle::shannon_resynthesis<named_base_type> resyn;
      base_type dest;
      named_base_type named_dest( dest );
      mockturtle::node_resynthesis( named_dest, ntk, resyn );
      mockturtle::names_view<network_type> wrapped( named_dest );
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
    using named_base_type = typename mockturtle::names_view<base_type>;

    constexpr bool with_xor = std::is_same_v<Dest, xag_t> || std::is_same_v<Dest, xmg_t>;

    if ( is_store_set<Dest>() )
    {
      mockturtle::exact_resynthesis_params esps;
      esps.cache = exact_aig_cache;
      mockturtle::exact_aig_resynthesis<named_base_type> eresyn( with_xor, esps );
      mockturtle::dsd_resynthesis<named_base_type, decltype( eresyn )> resyn( eresyn );
      base_type dest;
      named_base_type named_dest( dest );
      mockturtle::node_resynthesis( named_dest, ntk, resyn );
      mockturtle::names_view<network_type> wrapped( named_dest );
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
    using named_base_type = typename mockturtle::names_view<base_type>;

    if ( is_store_set<Dest>() )
    {
      const auto named_dest = [&]() -> named_base_type {
        if constexpr ( std::is_same_v<Dest, xag_t> || std::is_same_v<Dest, aig_t> )
        {
          mockturtle::xag_npn_resynthesis<named_base_type> resyn;
          base_type dest;
          named_base_type named_dest( dest );
          mockturtle::node_resynthesis( named_dest, ntk, resyn );
          return named_dest;
        }
        else if constexpr ( std::is_same_v<Dest, mig_t> )
        {
          mockturtle::mig_npn_resynthesis resyn;
          base_type dest;
          named_base_type named_dest( dest );
          mockturtle::node_resynthesis( named_dest, ntk, resyn );
          return named_dest;
        }
        else if constexpr ( std::is_same_v<Dest, xmg_t> )
        {
          mockturtle::xmg_npn_resynthesis resyn;
          base_type dest;
          named_base_type named_dest( dest );
          mockturtle::node_resynthesis( named_dest, ntk, resyn );
          return named_dest;
        }
        else
        {
          assert( false );
        }
        return base_type();
      }();
      mockturtle::names_view<network_type> wrapped( named_dest );
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
