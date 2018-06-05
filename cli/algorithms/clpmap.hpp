#include <alice/alice.hpp>

#include <mockturtle/algorithms/collapse_mapped.hpp>

namespace alice
{

class clpmap_command : public command
{
public:
  clpmap_command( environment::ptr& env ) : command( env, "Collapses mapped network" )
  {
    add_flag( "-a,--aig", "collapse mapped AIG" );
    add_flag( "-m,--mig", "collapse mapped MIG" );
    add_flag( "-l,--lut", "collapse mapped LUT network" );

    add_flag( "-n,--new", "create new store element" );
  }

  rules validity_rules() const
  {
    return {
      has_store_element_if_set<aig_t>( *this, env, "aig" ),
      has_store_element_if_set<mig_t>( *this, env, "mig" ),
      has_store_element_if_set<klut_t>( *this, env, "lut" )
    };
  }

  template<class Store>
  void _execute()
  {
    const auto ntk = mockturtle::collapse_mapped_network<mockturtle::klut_network>( *( env->store<Store>().current() ) );
    if ( ntk )
    {
      if ( env->store<klut_t>().empty() || is_set( "new" ) )
      {
        env->store<klut_t>().extend();
      }
      env->store<klut_t>().current() = std::make_shared<klut_nt>( *ntk );
    }
    else
    {
      env->out() << "[w] network has no mapping\n";
    }
  }

  void execute()
  {
    if ( is_set( "aig" ) )
    {
      _execute<aig_t>();
    }
    else if ( is_set( "mig" ) )
    {
      _execute<mig_t>();
    }
    else if ( is_set( "lut" ) )
    {
      _execute<klut_t>();
    }
  }
};

ALICE_ADD_COMMAND( clpmap, "Mapping" )

}
