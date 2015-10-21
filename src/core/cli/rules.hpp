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
 * @file rules.hpp
 *
 * @brief Some pre-defined rules
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef RULES_HPP
#define RULES_HPP

#include <string>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <core/cli/command.hpp>
#include <core/cli/store.hpp>

namespace cirkit
{

inline command::rule_t file_exists( const std::string& filename, const std::string& argname )
{
  return { [&]() { return boost::filesystem::exists( filename ); }, argname + " does not exist" };
}

inline command::rule_t has_addon( const std::string& addon_name )
{
  return { []() { return false; }, addon_name + " is not enabled" };
}

template<typename S>
command::rule_t has_store_element( const environment::ptr& env )
{
  auto constexpr name = store_info<S>::name;
  return { [&]() { return env->store<S>().current_index() >= 0; }, ( boost::format( "no current %s available" ) % name ).str() };
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
