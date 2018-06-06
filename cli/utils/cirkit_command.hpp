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

    if ( is_set( option ) || default_option == option || env->default_option() == option )
    {
      static_cast<Command*>( this )->template execute_store<S>();
      env->set_default_option( option );
      return true;
    }

    return false;
  }

private:
  std::string default_option;
};

} // namespace cirkit
