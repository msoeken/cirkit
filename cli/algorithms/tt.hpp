#include <alice/alice.hpp>

#include <cmath>
#include <cstdint>

#include <kitty/constructors.hpp>

namespace alice
{

class tt_command : public alice::command
{
public:
  tt_command( const environment::ptr& env ) : command( env, "Creates permutation" )
  {
    add_option( "table,--table", table, "truth table (prefix with 0x to read as hexadecimal)" )->required();
    add_flag( "-n,--new", "adds new store entry" );
  }

  void execute() override
  {
    auto& tts = store<kitty::dynamic_truth_table>();
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

private:
  std::string table;
};

ALICE_ADD_COMMAND( tt, "Loading" );

}
