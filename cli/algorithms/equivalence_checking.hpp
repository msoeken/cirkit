#include <alice/alice.hpp>

#include <optional>

#include <mockturtle/algorithms/equivalence_checking.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class equivalence_checking_command : public cirkit::cirkit_command<equivalence_checking_command, aig_t, mig_t, xag_t, xmg_t, klut_t>
{
public:
  equivalence_checking_command( environment::ptr& env ) : cirkit::cirkit_command<equivalence_checking_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Combinational equivalence checking of miter", "miter is {0}" )
  {
    add_option( "--conflict_limit", ps.conflict_limit, "conflict limit (0 to disable)", true );
  }

  template<class Store>
  inline void execute_store()
  {
    result_ = mockturtle::equivalence_checking( *( store<Store>().current() ), ps, &st );
    if ( result_ )
    {
      std::cout << "[i] miter is" << ( *result_ ? "" : " not" ) << " equivalent\n";
    }
    else
    {
      std::cout << "[i] resource limit reached, result undefined\n";
    }
  }

  nlohmann::json log() const override
  {
    nlohmann::json _log = {
      {"time_total", mockturtle::to_seconds( st.time_total )},
    };

    if ( result_ ) {
      _log["result"] = *result_;
    } else {
      _log["result"] = nullptr;
    }

    return _log;
  }

private:
  mockturtle::equivalence_checking_params ps;
  mockturtle::equivalence_checking_stats st;

  std::optional<bool> result_;
};

ALICE_ADD_COMMAND( equivalence_checking, "Verification" )

}
