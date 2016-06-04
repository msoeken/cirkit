/* alice: A C++ EDA command line interface API
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file rules.hpp
 *
 * @brief Some pre-defined rules
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <string>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <alice/command.hpp>

namespace alice
{

inline command::rule_t file_exists( const std::string& filename, const std::string& argname )
{
  return { [filename, argname]() { return boost::filesystem::exists( filename ); }, argname + " does not exist" };
}

inline command::rule_t file_exists_if_set( const command& cmd, const std::string& filename, const std::string& argname )
{
  return { [&cmd, filename, argname]() { return !cmd.is_set( argname ) || boost::filesystem::exists( filename ); }, argname + " does not exist" };
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

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
