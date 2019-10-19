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

#include <mockturtle/algorithms/collapse_mapped.hpp>
#include <mockturtle/views/names_view.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class collapse_mapping_command : public cirkit::cirkit_command<collapse_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>
{
public:
  collapse_mapping_command( environment::ptr& env ) : cirkit::cirkit_command<collapse_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Collapses mapped network", "collapse mapped {}" )
  {
    add_new_option();
  }

  template<class Store>
  void execute_store()
  {
    mockturtle::klut_network ntk;
    mockturtle::names_view<mockturtle::klut_network> named_ntk( ntk );
    bool success = mockturtle::collapse_mapped_network<mockturtle::names_view<mockturtle::klut_network>>( named_ntk, *( env->store<Store>().current() ) );
    if ( success )
    {
      extend_if_new<klut_t>();
      store<klut_t>().current() = std::make_shared<klut_nt>( named_ntk );

      set_default_option<klut_t>();
    }
    else
    {
      env->out() << "[w] network has no mapping\n";
    }
  }
};

ALICE_ADD_COMMAND( collapse_mapping, "Mapping" )

}
