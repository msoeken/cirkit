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
 * @file aig_command.hpp
 *
 * @brief Special command for (single) AIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_AIG_BASE_COMMAND_HPP
#define CLI_AIG_BASE_COMMAND_HPP

#include <alice/rules.hpp>

#include <core/cli/cirkit_command.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

class aig_base_command : public cirkit_command
{
public:
  aig_base_command( const environment::ptr& env, const std::string& caption )
    : cirkit_command( env, caption ),
      store( env->store<aig_graph>() )
  {
  }

protected:
  virtual rules_t validity_rules() const
  {
    return { has_store_element<aig_graph>( env ) };
  }

  inline aig_graph& aig()
  {
    return store.current();
  }

  inline const aig_graph& aig() const
  {
    return store.current();
  }

  inline aig_graph_info& info()
  {
    return aig_info( store.current() );
  }

  inline const aig_graph_info& info() const
  {
    return aig_info( store.current() );
  }

protected:
  cli_store<aig_graph>& store;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
