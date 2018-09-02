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
    add_option( "--stg", strategy, "synthesis strategy", true )->set_type_name( "strategy in {pprm=0, spectrum=1, pkrm=2, db=3}" );
    add_flag( "-n,--new", "adds new store entry" );
    add_flag( "-v,--verbose", "be verbose" );
  }

  rules validity_rules() const override
  {
    return {has_store_element<perm_t>( env )};
  }

  void execute() override
  {
    auto const& perms = store<perm_t>();
    auto f = perms.current(); // will be modified by dbs

    tweedledum::decomposition_based_synthesis_params ps;
    ps.verbose = is_set( "verbose" );

    if (strategy == 0u)
    {
      auto& circs = store<small_mct_circuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = small_mct_circuit_t();
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_pprm(), ps );
    }
    else if (strategy == 1u)
    {
      auto& circs = store<qc_circuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = qc_circuit_t();
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_spectrum(), ps );
    }
    else if (strategy == 2u)
    {
      auto& circs = store<small_mct_circuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = small_mct_circuit_t();
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_pkrm(), ps );
    }
    else if (strategy == 3u)
    {
      auto& circs = store<qc_circuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = qc_circuit_t();
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_db(), ps );
    }
  }

private:
  uint32_t strategy{0u};
};

ALICE_ADD_COMMAND( dbs, "Synthesis" );

} // namespace alice
