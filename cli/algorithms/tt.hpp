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
    add_option( "--maj", maj, "creates truth table n-input majority function" );
    add_option( "--sym", sym, "creates truth table for symmetric function (e.g., tt --sym 001 for 2-input AND; tt --sym 101 for 2-input XNOR; tt --sym 000111 for 5-input MAJ" );
    add_flag( "-n,--new", "adds new store entry" );
  }

  rules validity_rules() const override
  {
    return {
        {[this]() { return !is_set( "maj" ) || ( maj > 2 && maj % 2 == 1 ); }, "majority size must be odd positive number greater than 2"},
        {[this]() {
           return !is_set( "sym" ) ||
                  std::all_of( sym.begin(), sym.end(), []( auto c ) { return c == '0' || c == '1'; } );
         },
         "symmetric pattern must contain only of 0s and 1s"}};
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
    else if ( is_set( "maj" ) )
    {
      if ( tts.empty() || is_set( "new" ) )
      {
        tts.extend();
      }

      kitty::dynamic_truth_table tt( maj );
      kitty::create_majority( tt );
      tts.current() = tt;
    }
    else if ( is_set( "sym" ) )
    {
      if ( tts.empty() || is_set( "new" ) )
      {
        tts.extend();
      }

      kitty::dynamic_truth_table tt( sym.size() - 1u );
      uint64_t counts{0u}, mask{1u};
      for ( auto c : sym )
      {
        if ( c == '1' )
        {
          counts |= mask;
        }
        mask <<= 1u;
      }
      kitty::create_symmetric( tt, counts );
      tts.current() = tt;
    }
  }

private:
  std::string table;
  std::string expression;
  uint32_t maj;
  std::string sym;
};

ALICE_ADD_COMMAND( tt, "Loading" );

} // namespace alice
