#if !defined ALICE_PYTHON
#define ALICE_SETTINGS_WITH_DEFAULT_OPTION true
#endif

#include <alice/alice.hpp>

#include "filetypes.hpp"
#include "stores/aig.hpp"
#include "stores/klut.hpp"
#include "stores/mig.hpp"
#include "stores/tt.hpp"
#include "stores/xag.hpp"
#include "stores/xmg.hpp"

#include "algorithms/collapse_mapping.hpp"
#include "algorithms/cut_rewrite.hpp"
#include "algorithms/exact.hpp"
#include "algorithms/lut_mapping.hpp"
#include "algorithms/migcost.hpp"
#include "algorithms/mighty.hpp"
#include "algorithms/minmc.hpp"
#include "algorithms/lut_resynthesis.hpp"
#include "algorithms/npn.hpp"
#include "algorithms/print_gates.hpp"
#include "algorithms/refactor.hpp"
#include "algorithms/resubstitute.hpp"
#include "algorithms/simulate.hpp"
#include "algorithms/spectral.hpp"
#include "algorithms/tt.hpp"

ALICE_MAIN( cirkit )
