#include <alice/alice.hpp>

#include <fmt/format.h>
#include <mockturtle/properties/mccost.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class mccost_command : public cirkit::cirkit_command<mccost_command, aig_t, xag_t, mig_t, xmg_t>
{
public:
  mccost_command( environment::ptr& env ) : cirkit::cirkit_command<mccost_command, aig_t, xag_t, mig_t, xmg_t>( env, "Prints multiplicative complexity costs", "costs for {0}" )
  {
  }

  template<class Store>
  inline void execute_store()
  {
    const auto size = mockturtle::multiplicative_complexity( *store<Store>().current() );
    const auto depth = mockturtle::multiplicative_complexity_depth( *store<Store>().current() );

    std::cout << fmt::format( "[i] mult. compl. size  = {}\n", size ? std::to_string( *size ) : std::string( "N/A" ) );
    std::cout << fmt::format( "[i] mult. compl. depth = {}\n", depth ? std::to_string( *depth ) : std::string( "N/A" ) );
  }
};

ALICE_ADD_COMMAND( mccost, "Various" )

} // namespace alice
