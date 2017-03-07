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

/**
 * @file xmg_command.hpp
 *
 * @brief Special command for (single) XMGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_XMG_BASE_COMMAND_HPP
#define CLI_XMG_BASE_COMMAND_HPP

#include <alice/rules.hpp>
#include <core/cli/cirkit_command.hpp>

#include <classical/cli/stores.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

inline command::rule_t has_xmg( const environment::ptr& env )
{
  return has_store_element<xmg_graph>( env );
}

class xmg_base_command : public cirkit_command
{
public:
  xmg_base_command( const environment::ptr& env, const std::string& caption )
    : cirkit_command( env, caption ),
      store( env->store<xmg_graph>() )
  {
  }

protected:
  virtual rules_t validity_rules() const
  {
    return { has_xmg( env ) };
  }

  inline xmg_graph& xmg()
  {
    return store.current();
  }

  inline const xmg_graph& xmg() const
  {
    return store.current();
  }

protected:
  cli_store<xmg_graph>& store;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
