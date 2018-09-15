#include <alice/alice.hpp>

#include <kitty/dynamic_truth_table.hpp>
#include <kitty/print.hpp>

namespace alice
{

ALICE_ADD_STORE( kitty::dynamic_truth_table, "tt", "t", "truth table", "truth tables" );

ALICE_DESCRIBE_STORE( kitty::dynamic_truth_table, tt )
{
  return fmt::format( "{} vars", tt.num_vars() );
}

ALICE_PRINT_STORE_STATISTICS( kitty::dynamic_truth_table, os, tt )
{
  os << fmt::format( "{} vars\n", tt.num_vars() );
}

ALICE_LOG_STORE_STATISTICS( kitty::dynamic_truth_table, tt )
{
  return {
    {"vars", tt.num_vars()},
    {"hex", kitty::to_hex(tt)},
    {"binary", kitty::to_binary(tt)}
  };
}

ALICE_PRINT_STORE( kitty::dynamic_truth_table, os, tt )
{
  kitty::print_hex( tt, os );
  os << "\n";
}

}
