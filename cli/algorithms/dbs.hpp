#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/synthesis/decomposition_based.hpp>
#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>

namespace alice
{
  using small_mct_circuit_t = tweedledum::netlist<tweedledum::mct_gate>;
  using qc_circuit_t = tweedledum::dag_path<tweedledum::qc_gate>;

class dbs_command : public alice::command
{
public:
  dbs_command( const environment::ptr& env ) : command( env, "Decomposition-based synthesis" )
  {
    add_option( "--stg", strategy, "synthesis strategy", true )->set_type_name( "strategy in {pprm=0, spectrum=1}" );
    add_flag( "-n,--new", "adds new store entry" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<perm_t>( env )};
  }

  void execute() override
  {
    auto const& perms = store<perm_t>();
    auto f = perms.current(); // will be modified by dbs

    if (strategy == 0u)
    {
      auto& circs = store<small_mct_circuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_pprm() );
    }
    else if (strategy == 1u)
    {
      auto& circs = store<qc_circuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_spectrum() );
    }
  }

private:
  uint32_t strategy{0u};
};

ALICE_ADD_COMMAND( dbs, "Synthesis" );

} // namespace alice
