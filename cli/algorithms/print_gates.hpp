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

#include <mockturtle/traits.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class print_gates_command : public cirkit::cirkit_command<print_gates_command, aig_t, mig_t, xag_t, xmg_t, klut_t>
{
public:
  print_gates_command( environment::ptr& env ) : cirkit::cirkit_command<print_gates_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Print gate summary", "gate summary for {0}" )
  {
  }

  template<class Store>
  inline void execute_store()
  {
    num_and = num_or = num_xor = num_maj = num_ite = num_unknown = 0u;

    using Ntk = typename Store::element_type;
    const auto& ntk = *store<Store>().current();
    ntk.foreach_gate( [&]( auto const& node ) {
      if constexpr ( mockturtle::has_is_and_v<Ntk> )
      {
        if ( ntk.is_and( node ) )
        {
          ++num_and;
          return;
        }
      }
      if constexpr ( mockturtle::has_is_or_v<Ntk> )
      {
        if ( ntk.is_or( node ) )
        {
          ++num_or;
          return;
        }
      }
      if constexpr ( mockturtle::has_is_xor_v<Ntk> )
      {
        if ( ntk.is_xor( node ) )
        {
          ++num_xor;
          return;
        }
      }
      if constexpr ( mockturtle::has_is_maj_v<Ntk> )
      {
        if ( ntk.is_maj( node ) )
        {
          ++num_maj;
          return;
        }
      }
      if constexpr ( mockturtle::has_is_ite_v<Ntk> )
      {
        if ( ntk.is_ite( node ) )
        {
          ++num_ite;
          return;
        }
      }
      if constexpr ( mockturtle::has_is_xor3_v<Ntk> )
      {
        if ( ntk.is_xor3( node ) )
        {
          ++num_xor;
          return;
        }
      }
      ++num_unknown;
    } );

    env->out() << fmt::format( "[i] AND     = {}\n"
                               "[i] OR      = {}\n"
                               "[i] XOR     = {}\n"
                               "[i] MAJ     = {}\n"
                               "[i] ITE     = {}\n"
                               "[i] Unknown = {}\n",
                               num_and, num_or, num_xor, num_maj, num_ite, num_unknown );
  }

  nlohmann::json log() const override
  {
    return {
      {"and", num_and},
      {"or", num_or},
      {"xor", num_xor},
      {"maj", num_maj},
      {"ite", num_ite},
      {"unknown", num_unknown}
    };
  }

private:
  unsigned num_and{0u}, num_or{0u}, num_xor{0u}, num_maj{0u}, num_ite{0u}, num_unknown{0u};
};

ALICE_ADD_COMMAND( print_gates, "I/O" )

} // namespace alice
