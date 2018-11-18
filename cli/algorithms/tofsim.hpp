#include <alice/alice.hpp>

#include <cstdint>
#include <iostream>
#include <string>

#include <tweedledum/algorithms/simulation/classical_simulation.hpp>

namespace alice
{

class tofsim_command : public alice::command
{
public:
  tofsim_command( const environment::ptr& env ) : command( env, "Toffoli gate simulation" )
  {
    add_option( "--pattern,pattern", pattern, "input bit pattern" )->required();
    add_flag( "--quiet", "do not print result" );
  }

  rules validity_rules() const override
  {
    return {
        has_store_element<qcircuit_t>( env ),
        {[&]() { return std::count_if( pattern.begin(), pattern.end(), []( auto c ) { return c != '0' && c != '1'; } ) == 0; }, "input pattern must consists of only 0 and 1"},
        {[&]() { return pattern.size() == store<qcircuit_t>().current().num_qubits(); }, "input pattern size must match number of qubits"}};
  }

  void execute() override
  {
    const auto& circs = store<qcircuit_t>();
    const auto result = tweedledum::simulate_pattern_classical( circs.current(), pattern_from_string() );
    pattern_to_string( result );
    if (!is_set("quiet")) {
      std::cout << "[i] result: " << pattern_result << "\n";
    }
  }

  nlohmann::json log() const override
  {
    return {{"result", pattern_result}};
  }

private:
  uint64_t pattern_from_string() const
  {
    uint64_t ipattern{0};
    const auto n = store<qcircuit_t>().current().num_qubits();
    for ( auto i = 0; i < n; ++i )
    {
      if ( pattern[i] == '1' )
      {
        ipattern |= ( 1 << ( n - 1 - i ) );
      }
    }
    return ipattern;
  }

  void pattern_to_string( uint64_t opattern )
  {
    const auto n = store<qcircuit_t>().current().num_qubits();
    pattern_result = std::string( n, '0' );
    for ( auto i = 0; i < n; ++i )
    {
      if ( ( opattern >> i ) & 1 )
      {
        pattern_result[n - 1 - i] = '1';
      }
    }
  }

private:
  std::string pattern, pattern_result;
};

ALICE_ADD_COMMAND( tofsim, "Simulation" );

} // namespace alice
