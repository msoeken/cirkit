#include <alice/alice.hpp>

#include <vector>

#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/algorithms/simulation.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class simulate_command : public cirkit::cirkit_command<simulate_command, aig_t, xag_t, mig_t, klut_t, xmg_t>
{
public:
  simulate_command( environment::ptr& env ) : cirkit::cirkit_command<simulate_command, aig_t, xag_t, mig_t, klut_t, xmg_t>( env, "Simulates network into truth tables", "simulate {0}" )
  {
    add_flag( "--store", "store simulation results in truth table store" );
    add_flag( "--binary", "print truth tables as binary strings" );
    add_flag( "--silent", "do not print truth tables" );
    add_flag( "--log", "keep simulation results in log" );
  }

  template<class Store>
  inline void execute_store()
  {
    const auto& ntk = *( env->store<Store>().current() );
    const auto results = mockturtle::simulate<kitty::dynamic_truth_table>( ntk, mockturtle::default_simulator<kitty::dynamic_truth_table>( ntk.num_pis() ) );

    auto& tts = env->store<kitty::dynamic_truth_table>();
    tables.clear();

    if ( !is_set( "silent" ) || is_set( "store" ) || is_set( "log" ) )
    {
      for ( auto const& result : results )
      {
        if ( !is_set( "silent" ) )
        {
          if ( is_set( "binary" ) )
          {
            kitty::print_binary( result );
          }
          else
          {
            kitty::print_hex( result );
          }
          std::cout << "\n";
        }
        if ( is_set( "store" ) )
        {
          tts.extend();
          tts.current() = result;
        }
        if ( is_set( "log" ) )
        {
          tables.push_back( result );
        }
      }
    }
  }

  nlohmann::json log() const override
  {
    if ( !is_set( "log" ) )
    {
      return nullptr;
    }

    nlohmann::json j;
    for ( auto const& tt : tables )
    {
      j.push_back( kitty::to_binary( tt ) );
    }
    return {{"tables", j}};
  }

private:
  std::vector<kitty::dynamic_truth_table> tables;
};

ALICE_ADD_COMMAND( simulate, "Simulation" )

} // namespace alice
