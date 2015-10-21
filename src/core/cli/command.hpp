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
 * @file command.hpp
 *
 * @brief CLI general command data structure
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_COMMAND_HPP
#define CLI_COMMAND_HPP

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <core/properties.hpp>
#include <core/cli/environment.hpp>
#include <core/utils/program_options.hpp>

namespace cirkit
{

class command
{
public:
  using rule_t  = std::pair<std::function<bool()>, std::string>;
  using rules_t = std::vector<rule_t>;

  using log_var_t = boost::variant<std::string, int, double>;
  using log_map_t = std::unordered_map<std::string, log_var_t>;
  using log_opt_t = boost::optional<log_map_t>;

  command( const environment::ptr& env, const std::string& caption );

  const std::string& caption() const;
  bool run( const std::vector<std::string>& args );

  inline bool is_set( const std::string& opt ) const { return opts.is_set( opt ); }

protected:
  virtual rules_t validity_rules() const { return {}; }
  virtual bool execute() = 0;

public:
  virtual log_opt_t log() const { return boost::none; }

protected:
  /* pre-defined options */
  inline void be_verbose() { opts.add_options()( "verbose,v", "Be verbose" ); }
  inline bool is_verbose() const { return opts.is_set( "verbose" ); }

  /* get settings with often-used pre-defined options */
  properties::ptr make_settings() const;

public:
  std::shared_ptr<environment> env;

protected:
  std::string     scaption;
  program_options opts;

  properties::ptr statistics;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
