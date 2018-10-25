#include <alice/alice.hpp>

#include <iostream>

#include <tweedledum/algorithms/synthesis/decomposition_based.hpp>
#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>

namespace alice
{

class dbs_command : public alice::command
{
public:
  dbs_command( const environment::ptr& env ) : command( env, "Decomposition-based synthesis" )
  {
    add_option( "--stg", strategy, "synthesis strategy", true )->set_type_name( "strategy in {pprm=0, spectrum=1, pkrm=2}" );
    add_flag( "--allow_rewiring", "allow rewiring inside single-target gates" );
    add_option( "--lin_comb_synth_behavior", lin_comb_synth_behavior, "how to use LinCombSynth in spectrum strategy", true )->set_type_name( "behavior in {always=0, never=1, complete_sepctra=2}" );
    add_option( "--lin_comb_synth_strategy", lin_comb_synth_strategy, "strategy for LinCombSynth in spectrum strategy", true )->set_type_name( "behavior in {gray=0, binary=1}" );
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

    if ( strategy == 0u )
    {
      auto& circs = store<qcircuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = tweedledum::decomposition_based_synthesis<qcircuit_t>( f, tweedledum::stg_from_pprm(), ps );
    }
    else if ( strategy == 1u )
    {
      auto& circs = store<qcircuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      tweedledum::stg_from_spectrum_params stgps;
      stgps.lin_comb_synth_behavior = static_cast<tweedledum::stg_from_spectrum_params::lin_comb_synth_behavior_t>( lin_comb_synth_behavior );
      stgps.lin_comb_synth_strategy = static_cast<tweedledum::stg_from_spectrum_params::lin_comb_synth_strategy_t>( lin_comb_synth_strategy );
      stgps.gray_synth_ps.allow_rewiring = is_set( "allow_rewiring" );
      circs.current() = tweedledum::decomposition_based_synthesis<qcircuit_t>( f, tweedledum::stg_from_spectrum( stgps ), ps );
    }
    else if ( strategy == 2u )
    {
      auto& circs = store<qcircuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = tweedledum::decomposition_based_synthesis<qcircuit_t>( f, tweedledum::stg_from_pkrm(), ps );
    }
    /*else if (strategy == 3u)
    {
      auto& circs = store<qcircuit_t>();
      if ( circs.empty() || is_set( "new" ) )
      {
        circs.extend();
      }
      circs.current() = qcircuit_t();
      tweedledum::decomposition_based_synthesis( circs.current(), f, tweedledum::stg_from_db(), ps );
    }*/
  }

private:
  uint32_t strategy{0u};
  uint32_t lin_comb_synth_behavior{2u};
  uint32_t lin_comb_synth_strategy{0u};
};

ALICE_ADD_COMMAND( dbs, "Synthesis" );

} // namespace alice
