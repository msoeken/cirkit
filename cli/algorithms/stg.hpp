#include <alice/alice.hpp>

#include <cstdint>
#include <iostream>
#include <numeric>

#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>

namespace alice
{

class stg_command : public alice::command
{
public:
  stg_command( const environment::ptr& env ) : command( env, "Decomposition-based synthesis" )
  {
    add_option( "--stg", strategy, "synthesis strategy", true )->set_type_name( "strategy in {pprm=0, spectrum=1, pkrm=2, db=3}" );
    add_flag( "-n,--new", "adds new store entry" );
    add_flag( "-v,--verbose", "be verbose" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<kitty::dynamic_truth_table>( env )};
  }

private:
  template<class Network>
  Network& get_circuit( uint32_t num_vars )
  {
    auto& circs = store<Network>();
    if ( circs.empty() || is_set( "new" ) )
    {
      circs.extend();
    }
    circs.current() = Network();
    for ( auto i = 0u; i <= num_vars; ++i )
    {
      circs.current().add_qubit();
    }
    return circs.current();
  }

public:
  void execute() override
  {
    auto const& tts = store<kitty::dynamic_truth_table>();
    auto const& f = tts.current();

    std::vector<uint32_t> qubit_map( f.num_vars() + 1u );
    std::iota( qubit_map.begin(), qubit_map.end(), 0u );

    if ( strategy == 0u )
    {
      tweedledum::stg_from_pprm()( get_circuit<qcircuit_t>( f.num_vars() ), f, qubit_map );
    }
    else if ( strategy == 1u )
    {
      tweedledum::stg_from_spectrum()( get_circuit<qcircuit_t>( f.num_vars() ), f, qubit_map );
    }
    else if ( strategy == 2u )
    {
      tweedledum::stg_from_pkrm()( get_circuit<qcircuit_t>( f.num_vars() ), f, qubit_map );
    }
    else if ( strategy == 3u )
    {
      //tweedledum::stg_from_db()( get_circuit<qcircuit_t>( f.num_vars() ), f, qubit_map );
    }
  }

private:
  uint32_t strategy{0u};
};

ALICE_ADD_COMMAND( stg, "Synthesis" );

} // namespace alice
