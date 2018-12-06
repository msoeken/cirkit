#include <alice/alice.hpp>

#include <fmt/format.h>
#include <kitty/spectral.hpp>

namespace alice
{

class spectral_command : public alice::command
{
public:
  spectral_command( const environment::ptr& env ) : command( env, "Spectral canonization" )
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

    std::vector<kitty::detail::spectral_operation> ops;
    auto representative = kitty::exact_spectral_canonization( tts.current(), [&]( auto const& _ops ) { ops = _ops; } );

    if ( is_set( "verbose" ) )
    {
      env->out() << fmt::format( "[i] input:          {}\n[i] representative: {}\n",
                                 kitty::to_hex( tts.current() ),
                                 kitty::to_hex( representative ) );

      if ( is_set( "trans" ) )
      {
        for ( auto const& trans : ops )
        {
          switch ( trans._kind )
          {
          default:
            break;
          case kitty::detail::spectral_operation::kind::permutation:
            env->out() << fmt::format( "[i] swap {} and {}\n", (char)( 'a' + (int)std::log2( trans._var1 ) ), (char)( 'a' + (int)std::log2( trans._var2 ) ) );
            break;
          case kitty::detail::spectral_operation::kind::input_negation:
            env->out() << fmt::format( "[i] invert {}\n", (char)( 'a' + (int)std::log2( trans._var1 ) ) );
            break;
          case kitty::detail::spectral_operation::kind::output_negation:
            env->out() << "[i] invert function\n";
            break;
          case kitty::detail::spectral_operation::kind::spectral_translation:
            env->out() << fmt::format( "[i] add {} to {}\n", (char)( 'a' + (int)std::log2( trans._var2 ) ), (char)( 'a' + (int)std::log2( trans._var1 ) ) );
            break;
          case kitty::detail::spectral_operation::kind::disjoint_translation:
            env->out() << fmt::format( "[i] add {} to output\n", (char)( 'a' + (int)std::log2( trans._var1 ) ) );
            break;
          }
        }
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

ALICE_ADD_COMMAND( spectral, "Classification" );

} // namespace alice
