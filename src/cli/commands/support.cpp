/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
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

#include "support.hpp"

#include <iostream>

#include <core/utils/bitset_utils.hpp>
#include <classical/functions/aig_support.hpp>

#include <fmt/format.h>

namespace cirkit
{

support_command::support_command( const environment::ptr& env ) : aig_base_command( env, "Computes the structural support of the AIG" )
{
  be_verbose();
}

void support_command::execute()
{
  auto settings = make_settings();

  auto support = aig_structural_support( aig(), settings, statistics );

  std::cout << "[i] structural support" << std::endl;

  const auto& _info = info();
  for ( const auto& output : _info.outputs )
  {
    std::cout << fmt::format( "    {} :", output.second );
    foreach_bit( support.at( output.first ), [&]( unsigned pos ) {
        std::cout << " " << _info.node_names.at( _info.inputs.at( pos ) );
      } );
    std::cout << std::endl;
  }
  print_runtime();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
