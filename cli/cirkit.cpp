#define ALICE_SETTINGS_WITH_DEFAULT_OPTION true

#include <alice/alice.hpp>

#include "filetypes.hpp"
#include "stores/aig.hpp"
#include "stores/klut.hpp"
#include "stores/mig.hpp"

#include "algorithms/collapse_mapping.hpp"
#include "algorithms/cut_rewrite.hpp"
#include "algorithms/lut_mapping.hpp"
#include "algorithms/mighty.hpp"
#include "algorithms/lut_resynthesis.hpp"
#include "algorithms/refactor.hpp"

ALICE_MAIN( cirkit )
