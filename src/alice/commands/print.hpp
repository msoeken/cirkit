/* alice: A C++ EDA command line interface API
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
 * @file print.hpp
 *
 * @brief Prints current data structure
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <sstream>

#include <boost/format.hpp>

#include <alice/command.hpp>

namespace alice
{

template<typename S>
int print_helper( const command& cmd, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( cmd.is_set( option ) )
  {
    if ( env->store<S>().current_index() == -1 )
    {
      std::cout << "[w] no " << name << " in store" << std::endl;
    }
    else
    {
      print_store_entry<S>( std::cout, env->store<S>().current() );
    }
  }
  return 0;
}

template<typename S>
int print_log_helper( const command& cmd, const environment::ptr& env, command::log_map_t& map )
{
  constexpr auto option = store_info<S>::option;
  constexpr auto name   = store_info<S>::name;

  if ( cmd.is_set( option ) )
  {
    if ( env->store<S>().current_index() == -1 )
    {
      map["__repr__"] = boost::str( boost::format( "[w] no %s in store" ) % name );
    }
    else
    {
      std::stringstream strs;
      print_store_entry<S>( strs, env->store<S>().current() );
      map["__repr__"] = strs.str();

      if ( store_has_repr_html<S>() )
      {
        map["_repr_html_"] = store_repr_html<S>( env->store<S>().current() );
      }
    }
  }
  return 0;
}

template<class... S>
class print_command : public command
{
public:
  print_command( const environment::ptr& env ) : command( env, "Prints current data structure" )
  {
    [](...){}( add_option_helper<S>( opts )... );
  }

protected:
  rules_t validity_rules() const
  {
    return {
      {[this]() { return exactly_one_true_helper( { is_set( store_info<S>::option )... } ); }, "exactly one store needs to be specified" }
    };
  }

  bool execute()
  {
#ifndef ALICE_PYTHON
    [](...){}( print_helper<S>( *this, env )... );
#endif

    return true;
  }

  log_opt_t log() const
  {
#ifdef ALICE_PYTHON
    log_map_t map;
    [](...){}( print_log_helper<S>( *this, env, map )... );
    return map;
#else
    return boost::none;
#endif
  }
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
