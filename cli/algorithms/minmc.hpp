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

#include <memory>
#include <string>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_minmc.hpp>
#include <mockturtle/utils/stopwatch.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

namespace detail
{
template<class Ntk>
struct mc_cost
{
  uint32_t operator()( Ntk const& ntk, mockturtle::node<Ntk> const& n ) const
  {
    return ntk.is_and( n ) ? 1 : 0;
  }
};
} // namespace detail

class minmc_command : public cirkit::cirkit_command<minmc_command, xag_t>
{
public:
  minmc_command( environment::ptr& env ) : cirkit::cirkit_command<minmc_command, xag_t>( env, "Optimizes for multiplicative complexity", "applies optimization to {0}" )
  {
    add_option( "-k,--lutsize", ps.cut_enumeration_ps.cut_size, "cut size", true );
    add_option( "--lutcount", ps.cut_enumeration_ps.cut_limit, "cut limit", true );
    add_flag( "--progress,-p", ps.progress, "show progress" );
    add_option( "--load", db, "load database" );
    add_flag( "--verify" , "verify database when loading" );
    add_flag( "--verbose,-v", ps.verbose, "be verbose" );

    ps.min_cand_cut_size = 2u;
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
      mockturtle::xag_minmc_resynthesis_params params;
      if ( is_set( "verify" ) )
      {
        params.verify_database = true;
      }
      resyn.reset( new mockturtle::xag_minmc_resynthesis( db, params ) );
    }

    if ( store<xag_t>().current_index() >= 0 )
    {
      resyn->ps.print_stats = ps.verbose;

      auto* xag_p = static_cast<mockturtle::xag_network*>( store<xag_t>().current().get() );
      mockturtle::cut_rewriting( *xag_p, *resyn, ps, &st, detail::mc_cost<mockturtle::xag_network>() );
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
