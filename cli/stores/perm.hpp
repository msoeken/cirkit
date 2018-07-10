#include <alice/alice.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

#include <fmt/format.h>

namespace alice
{

using perm_t = std::vector<uint16_t>;

ALICE_ADD_STORE( perm_t, "perm", "p", "permutation", "permutations" );

ALICE_DESCRIBE_STORE( perm_t, perm )
{
  return fmt::format( "{} qubits", static_cast<uint32_t>( std::log2( perm.size() ) ) );
}

}
