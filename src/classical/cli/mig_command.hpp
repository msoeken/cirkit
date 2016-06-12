/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file mig_command.hpp
 *
 * @brief Special command for (single) MIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_MIG_BASE_COMMAND_HPP
#define CLI_MIG_BASE_COMMAND_HPP

#include <core/cli/cirkit_command.hpp>

#include <classical/mig/mig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

inline command::rule_t has_mig( const command* cmd )
{
  return { [&]() { return cmd->env->store<mig_graph>().current_index() >= 0; }, "no current MIG available" };
}

class mig_base_command : public cirkit_command
{
public:
  mig_base_command( const environment::ptr& env, const std::string& caption )
    : cirkit_command( env, caption ),
      store( env->store<mig_graph>() )
  {
  }

protected:
  virtual rules_t validity_rules() const
  {
    return { has_mig( this ) };
  }

  inline mig_graph& mig()
  {
    return store.current();
  }

  inline const mig_graph& mig() const
  {
    return store.current();
  }

  inline const mig_graph_info& info() const
  {
    return mig_info( store.current() );
  }

protected:
  cli_store<mig_graph>& store;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
