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

#include <optional>

#include <mockturtle/algorithms/equivalence_checking.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class equivalence_checking_command : public cirkit::cirkit_command<equivalence_checking_command, aig_t, mig_t, xag_t, xmg_t, klut_t>
{
public:
  equivalence_checking_command( environment::ptr& env ) : cirkit::cirkit_command<equivalence_checking_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Combinational equivalence checking of miter", "miter is {0}" )
  {
    add_option( "--conflict_limit", ps.conflict_limit, "conflict limit (0 to disable)", true );
  }

  template<class Store>
  inline void execute_store()
  {
    result_ = mockturtle::equivalence_checking( *( store<Store>().current() ), ps, &st );
    if ( result_ )
    {
      std::cout << "[i] miter is" << ( *result_ ? "" : " not" ) << " equivalent\n";
    }
    else
    {
      std::cout << "[i] resource limit reached, result undefined\n";
    }
  }

  nlohmann::json log() const override
  {
    nlohmann::json _log = {
      {"time_total", mockturtle::to_seconds( st.time_total )},
    };

    if ( result_ ) {
      _log["result"] = *result_;
    } else {
      _log["result"] = nullptr;
    }

    return _log;
  }

private:
  mockturtle::equivalence_checking_params ps;
  mockturtle::equivalence_checking_stats st;

  std::optional<bool> result_;
};

ALICE_ADD_COMMAND( equivalence_checking, "Verification" )

}
