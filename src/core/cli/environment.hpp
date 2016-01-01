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
 * @file environment.hpp
 *
 * @brief CLI environment
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_ENVIRONMENT_HPP
#define CLI_ENVIRONMENT_HPP

#include <chrono>
#include <fstream>
#include <map>
#include <memory>
#include <string>

#include <boost/any.hpp>

#include <core/cli/store.hpp>

namespace cirkit
{

class command;

class environment
{
public:
  using ptr = std::shared_ptr<environment>;

  template<typename T>
  void add_store( const std::string& key, const std::string& name )
  {
    stores.insert( {key, std::make_shared<cli_store<T>>( name )} );
  }

  template<typename T>
  cli_store<T>& store() const
  {
    constexpr auto key = store_info<T>::key;
    return *( boost::any_cast<std::shared_ptr<cli_store<T>>>( stores.at( key ) ) );
  }

  template<typename T>
  bool has_store() const
  {
    constexpr auto key = store_info<T>::key;
    return stores.find( key ) != stores.end();
  }

public: /* logging */
  void start_logging( const std::string& filename );
  void log_command( const std::shared_ptr<command>& cmd, const std::string& cmdstring, const std::chrono::system_clock::time_point& start );
  void log_command( const command_log_opt_t& cmdlog, const std::string& cmdstring, const std::chrono::system_clock::time_point& start );
  void stop_logging();

public:
  std::map<std::string, boost::any>               stores;
  std::map<std::string, std::shared_ptr<command>> commands;

  bool                                            log = false;
  bool                                            log_first_command = true;
  std::ofstream                                   logger;

  bool                                            quit = false;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
