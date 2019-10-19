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

#include <fmt/format.h>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/npn.hpp>
#include <kitty/operations.hpp>

namespace alice
{
template<typename S>
command::rule has_store_element_if_not_set( const command& cmd, const environment::ptr& env, const std::string& argname )
{
  auto constexpr name = store_info<S>::name;
  return { [&cmd, &env, argname]() { return cmd.is_set( argname ) || env->store<S>().current_index() >= 0; }, fmt::format( "no current {} available", name ) };
}
}

namespace alice
{

class npn_command : public alice::command
{
public:
  npn_command( const environment::ptr& env ) : command( env, "NPN canonization" )
  {
    add_flag( "--store", "store compute representative in store" );
    add_flag( "--trans", "print transformation sequence (when verbose)" );
    add_option( "--all", all, "generate all NPN classes for given number of variables" );
    add_flag( "-n,--new", "create new store element for representative" );
    add_flag( "--full-support", "when generating all NPN classes, only consider those with full support" );
    add_flag( "-v,--verbose", "be verbose" );
  }

  rules validity_rules() const override
  {
    return {
      has_store_element_if_not_set<kitty::dynamic_truth_table>( *this, env, "all" ),
      {
        [this]() {
          return !is_set( "all" ) || all < 6u;
        }, "exhaustive NPN classification works for up to 5 inputs"
      }
    };
  }

public:
  void execute() override
  {
    if ( is_set( "all") )
    {
      enumerate_all();
      return;
    }

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

private:
  void enumerate_all()
  {
    using truth_table = kitty::dynamic_truth_table;
    kitty::dynamic_truth_table map( 1u << all );

    /* invert bits: 1 means not classified yet */
    std::transform( map.cbegin(), map.cend(), map.begin(), []( auto word ) { return ~word; } );

    /* hash set to store all NPN classes */
    std::unordered_set<truth_table, kitty::hash<truth_table>> classes;

    int64_t index = 0;
    truth_table tt( all );

    while ( index != -1 )
    {
      /* create truth table from index value */
      kitty::create_from_words( tt, &index, &index + 1 );

      /* apply NPN canonization and add resulting representative to set;
         while canonization, mark all encountered truth tables in map
      */
      const auto res = kitty::exact_npn_canonization( tt, [&map]( const auto& tt ) { kitty::clear_bit( map, *tt.cbegin() ); } );

      bool insert = true;
      if ( is_set( "full-support" ) )
      {
        for ( auto i = 0u; i < all; ++i )
        {
          if ( !kitty::has_var( tt, i ) )
          {
            insert = false;
            break;
          }
        }
      }
      if ( insert )
      {
        classes.insert( std::get<0>( res ) );
      }

      /* find next non-classified truth table */
      index = find_first_one_bit( map );
    }

    env->out() << "[i] enumerated "
               << map.num_bits() << " functions into "
               << classes.size() << " classes:" << std::endl;

    for ( const auto& tt : classes ) {
      env->out() << kitty::to_hex( tt ) << "\n";
    }
  }

  private:
    uint32_t all;
};

ALICE_ADD_COMMAND( npn, "Classification" );

} // namespace alice
