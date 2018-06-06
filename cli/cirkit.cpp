#define ALICE_SETTINGS_WITH_DEFAULT_OPTION true

#include <alice/alice.hpp>

#include "filetypes.hpp"
#include "stores/aig.hpp"
#include "stores/klut.hpp"
#include "stores/mig.hpp"

#include "algorithms/clpmap.hpp"
#include "algorithms/lutmap.hpp"
#include "algorithms/mighty.hpp"
#include "algorithms/miglut.hpp"

ALICE_MAIN( cirkit )
