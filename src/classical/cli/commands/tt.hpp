/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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
 * @file tt.hpp
 *
 * @brief Truth table manipulation
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_TT_COMMAND_HPP
#define CLI_TT_COMMAND_HPP

#include <string>

#include <core/cli/command.hpp>
#include <core/cli/store.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

class tt_command : public command
{
  public:
  tt_command( const environment::ptr& env );

protected:
  rules_t validity_rules() const;
  bool execute();

public:
  log_opt_t log() const;

private:
  std::string    load;
  std::string    planame;
  unsigned       extend;
  cli_store<tt>& tts;

};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
