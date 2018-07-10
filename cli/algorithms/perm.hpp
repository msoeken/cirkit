#include <alice/alice.hpp>
#include <alice/detail/utils.hpp>

#include <algorithm>
#include <iostream>
#include <string>

namespace alice
{

class perm_command : public alice::command
{
public:
  perm_command( const environment::ptr& env ) : command( env, "Creates permutation" )
  {
    add_option( "permutation,--permutation", permutation, "creates a new permutation from space seperated list of numbers" )->required();
    add_flag( "-n,--new", "adds new store entry" );
  }

  void execute() override
  {
    auto& perms = store<perm_t>();
    if ( perms.empty() || is_set( "new" ) )
    {
      perms.extend();
    }

    auto slist = detail::split( permutation, " " );
    auto& perm = perms.current();
    perm.resize( slist.size() );
    std::transform( slist.begin(), slist.end(), perm.begin(), []( auto const& str ) { return std::stoul( str ); } );
  }

private:
  std::string permutation;
};

ALICE_ADD_COMMAND( perm, "Loading" );

}
