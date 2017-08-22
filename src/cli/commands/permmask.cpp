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

#include "permmask.hpp"

#include <boost/pending/integer_log2.hpp>
#include <boost/program_options.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/utils/small_truth_table_utils.hpp>

using boost::program_options::value;

namespace cirkit
{

permmask_command::permmask_command( const environment::ptr& env )
  : cirkit_command( env, "Compute permutation masks" )
{
  opts.add_options()
    ( "permutation,p", value( &permutation ), "permutation, space separatd (put quotes around it)" )
    ;
  add_positional_option( "permutation" );
}

command::rules_t permmask_command::validity_rules() const
{
  return {
    {[this]() { return !permutation.empty(); }, "permutation must not be empty" }
  };
}

bool permmask_command::execute()
{
  std::vector<unsigned> perm;
  parse_string_list( perm, permutation, " " );

  masks = stt_compute_mask_sequence( perm, boost::integer_log2( perm.size() ) );

  std::cout << "[i] masks: " << any_join( masks, " " ) << std::endl;

  return true;
}

command::log_opt_t permmask_command::log() const
{
  return log_opt_t({
      {"permutation", permutation},
      {"masks", masks}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
