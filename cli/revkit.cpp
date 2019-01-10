#if !defined ALICE_PYTHON
#define ALICE_SETTINGS_WITH_DEFAULT_OPTION true
#endif

#include <alice/alice.hpp>

#include "filetypes_revkit.hpp"
#include "stores/perm.hpp"
#include "stores/qc.hpp"
#include "stores/aig.hpp"
#include "stores/klut.hpp"
#include "stores/mig.hpp"
#include "stores/tt.hpp"
#include "stores/xag.hpp"
#include "stores/xmg.hpp"
#include "algorithms/collapse_mapping.hpp"
#include "algorithms/dbs.hpp"
#include "algorithms/esopbs.hpp"
#include "algorithms/esopps.hpp"
#include "algorithms/lut_mapping.hpp"
#include "algorithms/nct.hpp"
#include "algorithms/perm.hpp"
#include "algorithms/lns.hpp"
#include "algorithms/rptm.hpp"
#include "algorithms/stg.hpp"
#include "algorithms/tbs.hpp"
#include "algorithms/tofsim.hpp"
#include "algorithms/tt.hpp"

ALICE_MAIN( revkit )
