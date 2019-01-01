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
