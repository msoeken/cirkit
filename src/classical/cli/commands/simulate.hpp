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
 * @file simulate.hpp
 *
 * @brief Simulates an AIG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_SIMULATE_COMMAND_HPP
#define CLI_SIMULATE_COMMAND_HPP

#include <string>
#include <vector>

#include <core/utils/bdd_utils.hpp>
#include <classical/cli/aig_mig_command.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

class simulate_command : public aig_mig_command
{
public:
  simulate_command( const environment::ptr& env );

protected:
  rules_t validity_rules() const;
  bool execute_aig();
  bool execute_mig();

private:
  rule_t one_simulation_method() const;
  rule_t check_pattern_size() const;

  template<typename S>
  void store( const S& element )
  {
    if ( env->has_store<S>() && is_set( "store" ) )
    {
      auto& store = env->store<S>();

      store.extend();
      store.current() = element;
    }
  }

public:
  log_opt_t log() const;

private:
  std::string pattern;
  std::string assignment;

  std::vector<std::string> tts;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
