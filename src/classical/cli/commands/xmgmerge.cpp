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

#include "xmgmerge.hpp"

#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <classical/xmg/xmg.hpp>
#include <classical/xmg/xmg_rewrite.hpp>

namespace cirkit
{

xmgmerge_command::xmgmerge_command( const environment::ptr& env )
  : cirkit_command( env, "Merges two XMGs into one" )
{
  opts.add_options()
    ( "id1", value_with_default( &id1 ), "id of the first XMG" )
    ( "id2", value_with_default( &id2 ), "id of the second XMG" )
    ;
  add_new_option();
}

command::rules_t xmgmerge_command::validity_rules() const
{
  return {
    {[this]() { return id1 < env->store<xmg_graph>().size(); }, "id1 points to no valid store entry"},
    {[this]() { return id2 < env->store<xmg_graph>().size(); }, "id2 points to no valid store entry"}
  };
}

bool xmgmerge_command::execute()
{
  auto& xmgs = env->store<xmg_graph>();

  const auto& xmg1 = xmgs[id1];
  const auto& xmg2 = xmgs[id2];

  const auto xmg = xmg_merge( xmg1, xmg2 );

  extend_if_new( xmgs );
  xmgs.current() = xmg;

  return true;
}

command::log_opt_t xmgmerge_command::log() const
{
  return log_opt_t({
      {"id1", id1},
      {"id2", id2}
    });
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
