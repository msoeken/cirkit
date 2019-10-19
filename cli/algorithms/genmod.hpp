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

#include <cstdint>
#include <string>
#include <vector>

#include <mockturtle/generators/modular_arithmetic.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class genmod_command : public cirkit::cirkit_command<genmod_command, aig_t, mig_t, xmg_t, xag_t, klut_t>
{
public:
  genmod_command( environment::ptr& env ) : cirkit::cirkit_command<genmod_command, aig_t, mig_t, xmg_t, xag_t, klut_t>( env, "Generates modular arithmetic operations", "generate operation as {0}" )
  {
    add_option( "--bitwidth,-w", bitwidth, "bitwidth of the operation", true );
    add_option( "--add", add, "modular addition with given modulus" );
    add_option( "--sub", sub, "modular subtraction with given modulus" );
    add_option( "--mult", mult, "modular multiplication with given modulus" );
    add_option( "--dbl", dbl, "modular doubling with given modulus" );
    add_new_option();
  }

  rules validity_rules() const override
  {
    /* override default implementation with no rules */
    return {};
  }

  template<class Store>
  inline void execute_store()
  {
    if ( is_set( "add" ) )
    {
      std::vector<bool> mod( bitwidth );
      mockturtle::bool_vector_from_hex( mod, add );

      typename Store::element_type::base_type ntk;
      std::vector<typename Store::element_type::signal> as( bitwidth ), bs( bitwidth );
      std::generate( as.begin(), as.end(), [&]() { return ntk.create_pi(); } );
      std::generate( bs.begin(), bs.end(), [&]() { return ntk.create_pi(); } );
      mockturtle::modular_adder_inplace( ntk, as, bs, mod );
      std::for_each( as.begin(), as.end(), [&]( auto const& f ) { ntk.create_po( f ); } );

      extend_if_new<Store>();
      typename Store::element_type view{ntk};
      store<Store>().current() = std::make_shared<typename Store::element_type>( view );
    }

    if ( is_set( "sub" ) )
    {
      std::vector<bool> mod( bitwidth );
      mockturtle::bool_vector_from_hex( mod, sub );

      typename Store::element_type::base_type ntk;
      std::vector<typename Store::element_type::signal> as( bitwidth ), bs( bitwidth );
      std::generate( as.begin(), as.end(), [&]() { return ntk.create_pi(); } );
      std::generate( bs.begin(), bs.end(), [&]() { return ntk.create_pi(); } );
      mockturtle::modular_subtractor_inplace( ntk, as, bs, mod );
      std::for_each( as.begin(), as.end(), [&]( auto const& f ) { ntk.create_po( f ); } );

      extend_if_new<Store>();
      typename Store::element_type view{ntk};
      store<Store>().current() = std::make_shared<typename Store::element_type>( view );
    }

    if ( is_set( "mult" ) )
    {
      std::vector<bool> mod( bitwidth );
      mockturtle::bool_vector_from_hex( mod, mult );

      typename Store::element_type::base_type ntk;
      std::vector<typename Store::element_type::signal> as( bitwidth ), bs( bitwidth );
      std::generate( as.begin(), as.end(), [&]() { return ntk.create_pi(); } );
      std::generate( bs.begin(), bs.end(), [&]() { return ntk.create_pi(); } );
      mockturtle::modular_multiplication_inplace( ntk, as, bs, mod );
      std::for_each( as.begin(), as.end(), [&]( auto const& f ) { ntk.create_po( f ); } );

      extend_if_new<Store>();
      typename Store::element_type view{ntk};
      store<Store>().current() = std::make_shared<typename Store::element_type>( view );
    }

    if ( is_set( "dbl" ) )
    {
      std::vector<bool> mod( bitwidth );
      mockturtle::bool_vector_from_hex( mod, dbl );

      typename Store::element_type::base_type ntk;
      std::vector<typename Store::element_type::signal> as( bitwidth ), bs( bitwidth );
      std::generate( as.begin(), as.end(), [&]() { return ntk.create_pi(); } );
      mockturtle::modular_doubling_inplace( ntk, as, mod );
      std::for_each( as.begin(), as.end(), [&]( auto const& f ) { ntk.create_po( f ); } );

      extend_if_new<Store>();
      typename Store::element_type view{ntk};
      store<Store>().current() = std::make_shared<typename Store::element_type>( view );
    }
  }

private:
  uint32_t bitwidth = 32u;
  std::string add, sub, mult, dbl;
};

ALICE_ADD_COMMAND( genmod, "Generation" )

} // namespace alice
