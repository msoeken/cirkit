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

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/print.hpp>

namespace alice
{

ALICE_ADD_STORE( kitty::dynamic_truth_table, "tt", "t", "truth table", "truth tables" );

ALICE_DESCRIBE_STORE( kitty::dynamic_truth_table, tt )
{
  return fmt::format( "{} vars", tt.num_vars() );
}

ALICE_PRINT_STORE_STATISTICS( kitty::dynamic_truth_table, os, tt )
{
  os << fmt::format( "{} vars\n", tt.num_vars() );
}

ALICE_LOG_STORE_STATISTICS( kitty::dynamic_truth_table, tt )
{
  return {
    {"vars", tt.num_vars()},
    {"hex", kitty::to_hex(tt)},
    {"binary", kitty::to_binary(tt)}
  };
}

ALICE_PRINT_STORE( kitty::dynamic_truth_table, os, tt )
{
  kitty::print_hex( tt, os );
  os << "\n";
}

}
