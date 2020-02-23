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
#include <mockturtle/algorithms/refactoring.hpp>
#include <mockturtle/algorithms/node_resynthesis/bidecomposition.hpp>
#include <mockturtle/utils/stopwatch.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

namespace detail
{
template<class Ntk>
struct mc_cost2
{
  uint32_t operator()( Ntk const& ntk, mockturtle::node<Ntk> const& n ) const
  {
    return ntk.is_and( n ) ? 1 : 0;
  }
};
} // namespace detail

class refactormc_command : public cirkit::cirkit_command<refactormc_command, xag_t>
{
public:
  refactormc_command( environment::ptr& env ) : cirkit::cirkit_command<refactormc_command, xag_t>( env, "Optimizes for multiplicative complexity", "applies optimization to {0}" )
  {
    add_flag( "--dc", ps.use_dont_cares, "use don't cares for optimization" );
    add_flag( "--progress,-p", ps.progress, "show progress" );
  }

  template<class Store>
  inline void execute_store()
  {
    mockturtle::bidecomposition_resynthesis<mockturtle::xag_network> bi_resyn;
    auto* xag_p = static_cast<mockturtle::xag_network*>( store<xag_t>().current().get() );
    mockturtle::refactoring( *xag_p, bi_resyn, ps, &st, detail::mc_cost2<mockturtle::xag_network>());
    *xag_p = mockturtle::cleanup_dangling( *xag_p );
  }

  nlohmann::json log() const override
  {
    return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::refactoring_params ps;
  mockturtle::refactoring_stats st;
};

ALICE_ADD_COMMAND( refactormc, "Synthesis" )

} // namespace alice
