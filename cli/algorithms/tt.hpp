#include <alice/alice.hpp>

#include <cmath>
#include <cstdint>

#include <kitty/constructors.hpp>

namespace alice
{

class tt_command : public alice::command
{
public:
  tt_command( const environment::ptr& env ) : command( env, "Truth table creation and manipulation" )
  {
    add_option( "table,--table", table, "truth table (prefix with 0x to read as hexadecimal)" );
    add_option( "--expression", expression, "creates truth table from expression" );
    add_flag( "-n,--new", "adds new store entry" );
  }

  void execute() override
  {
    auto& tts = store<kitty::dynamic_truth_table>();

    if ( is_set( "table" ) )
    {
      if ( tts.empty() || is_set( "new" ) )
      {
        tts.extend();
      }

      if ( table.size() > 2u && table[0] == '0' && table[1] == 'x' )
      {
        table = table.substr( 2u );
        uint32_t num_vars = std::log2( table.size() << 2 );
        kitty::dynamic_truth_table tt( num_vars );
        kitty::create_from_hex_string( tt, table );
        tts.current() = tt;
      }
      else
      {
        uint32_t num_vars = std::log2( table.size() );
        kitty::dynamic_truth_table tt( num_vars );
        kitty::create_from_binary_string( tt, table );
        tts.current() = tt;
      }
    }
    else if ( is_set( "expression" ) )
    {
      if ( tts.empty() || is_set( "new" ) )
      {
        tts.extend();
      }

      /* find max var */
      uint32_t num_vars{0u};
      for (auto c : expression)
      {
        if (c >= 'a' && c <= 'p')
        {
          num_vars = std::max<uint32_t>( num_vars, c - 'a' + 1u );
        }
      }
      kitty::dynamic_truth_table tt( num_vars );
      if ( kitty::create_from_expression( tt, expression ) )
      {
        tts.current() = tt;
      }
    }
  }

private:
  std::string table;
  std::string expression;
};

ALICE_ADD_COMMAND( tt, "Loading" );

} // namespace alice
