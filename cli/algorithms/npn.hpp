#include <alice/alice.hpp>

#include <fmt/format.h>
#include <kitty/npn.hpp>

namespace alice
{

class npn_command : public alice::command
{
public:
  npn_command( const environment::ptr& env ) : command( env, "NPN canonization" )
  {
    add_flag( "--store", "store compute representative in store" );
    add_flag( "--trans", "print transformation sequence (when verbose)" );
    add_flag( "-n,--new", "create new store element for representative" );
    add_flag( "-v,--verbose", "be verbose" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<kitty::dynamic_truth_table>( env )};
  }

public:
  void execute() override
  {
    auto& tts = store<kitty::dynamic_truth_table>();

    auto result = kitty::exact_npn_canonization( tts.current() );
    auto representative = std::get<0>( result );

    if ( is_set( "verbose" ) )
    {
      env->out() << fmt::format( "[i] input:          {}\n[i] representative: {}\n",
                                 kitty::to_hex( tts.current() ),
                                 kitty::to_hex( representative ) );

      if ( is_set( "trans" ) )
      {
        std::cout << fmt::format( "[i] negations = {1:0{0}b}\n", tts.current().num_vars() + 1, std::get<1>( result ) );
        std::cout << fmt::format( "[i] permutation = {}\n", fmt::join( std::get<2>( result ), ", " ) );
      }
    }

    if ( is_set( "store" ) )
    {
      if ( is_set( "new" ) )
      {
        tts.extend();
      }
      tts.current() = representative;
    }
  }
};

ALICE_ADD_COMMAND( npn, "Classification" );

} // namespace alice
