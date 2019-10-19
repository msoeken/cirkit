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

#include <fmt/format.h>
#include <mockturtle/properties/mccost.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class mccost_command : public cirkit::cirkit_command<mccost_command, aig_t, xag_t, mig_t, xmg_t>
{
public:
  mccost_command( environment::ptr& env ) : cirkit::cirkit_command<mccost_command, aig_t, xag_t, mig_t, xmg_t>( env, "Prints multiplicative complexity costs", "costs for {0}" )
  {
  }

  template<class Store>
  inline void execute_store()
  {
    const auto size = mockturtle::multiplicative_complexity( *store<Store>().current() );
    const auto depth = mockturtle::multiplicative_complexity_depth( *store<Store>().current() );

    std::cout << fmt::format( "[i] mult. compl. size  = {}\n", size ? std::to_string( *size ) : std::string( "N/A" ) );
    std::cout << fmt::format( "[i] mult. compl. depth = {}\n", depth ? std::to_string( *depth ) : std::string( "N/A" ) );
  }
};

ALICE_ADD_COMMAND( mccost, "Various" )

} // namespace alice
