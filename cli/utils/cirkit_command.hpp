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
    ( add_flag_helper<Stores>( option_text ), ... );
  }

  rules validity_rules() const override
  {
    rules r;
    ( r.push_back( has_store_element_if_set<Stores>( *this, env, store_info<Stores>::option ) ), ... );
    return r;
  }

  void execute() override
  {
    ( execute_helper<Stores>() || ... );
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

private:
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

  template<class S>
  bool execute_helper()
  {
    constexpr auto option = store_info<S>::option;

    if ( is_set( option ) )
    {
      static_cast<Command*>( this )->template execute_store<S>();
      return true;
    }

    return false;
  }
};

} // namespace cirkit
