/* CirKit: A circuit toolkit
 * Copyright (C) 2017-2019  EPFL
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

#pragma once

#include <alice/command.hpp>

#include <fmt/format.h>

namespace cirkit
{

using namespace alice;

template<class Command, class... Stores>
class cirkit_command : public command
{
public:
  cirkit_command( environment::ptr& env, const std::string& caption, const std::string& option_text = {} ) : command( env, caption )
  {
    if ( sizeof...( Stores ) == 1 )
    {
      ( ( default_option = store_info<Stores>::option ), ... );
    }
    else
    {
      ( add_flag_helper<Stores>( option_text ), ... );
    }
  }

  rules validity_rules() const override
  {
    rules r;
    if ( sizeof...( Stores ) == 1 )
    {
      ( r.push_back( has_store_element<Stores>( env ) ), ... );
    }
    else
    {
      ( r.push_back( has_store_element_if_set<Stores>( *this, env, store_info<Stores>::option ) ), ... );
    }
    return r;
  }

  void execute() override
  {
    if ( !( execute_helper<Stores>() || ... ) )
    {
      env->out() << "[w] no store specified\n";
    }
  }

protected:
  void add_new_option()
  {
    add_flag( "-n,--new", "create new store element" );
  }

  template<class Store>
  void extend_if_new()
  {
    if ( store<Store>().empty() || is_set( "new" ) )
    {
      store<Store>().extend();
    }
  }

  template<class S>
  void set_default_option()
  {
    constexpr auto option = store_info<S>::option;
    env->set_default_option( option );
    option_set = true;
  }

protected:
  template<class S>
  void add_flag_helper( const std::string& option_text )
  {
    constexpr auto option = store_info<S>::option;
    constexpr auto mnemonic = store_info<S>::mnemonic;
    constexpr auto name = store_info<S>::name;
    constexpr auto name_plural = store_info<S>::name_plural;

    if ( strlen( mnemonic ) == 1u )
    {
      add_flag( fmt::format( "-{},--{}", mnemonic, option ), fmt::format( option_text, name, name_plural ) );
    }
    else
    {
      add_flag( fmt::format( "--{}", option ), fmt::format( option_text, name, name_plural ) );
    }
  }

private:
  template<class S>
  bool execute_helper()
  {
    constexpr auto option = store_info<S>::option;

    if ( is_set( option ) || default_option == option || env->default_option() == option )
    {
      option_set = false;
      static_cast<Command*>( this )->template execute_store<S>();
      if ( !option_set )
      {
        env->set_default_option( option );
      }
      return true;
    }

    return false;
  }

private:
  std::string default_option;
  bool option_set{false};
};

} // namespace cirkit
