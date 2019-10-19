/* CirKit: A circuit toolkit
 * Copyright (C) 2017-2019  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
#include "algorithms/equivalence_checking.hpp"
#include "algorithms/exact.hpp"
#include "algorithms/genmod.hpp"
#include "algorithms/lut_mapping.hpp"
#include "algorithms/lut_resynthesis.hpp"
#include "algorithms/mccost.hpp"
#include "algorithms/migcost.hpp"
#include "algorithms/mighty.hpp"
#include "algorithms/minmc.hpp"
#include "algorithms/miter.hpp"
#include "algorithms/npn.hpp"
#include "algorithms/print_gates.hpp"
#include "algorithms/refactor.hpp"
#include "algorithms/resubstitute.hpp"
#include "algorithms/satlut_mapping.hpp"
#include "algorithms/simulate.hpp"
#include "algorithms/spectral.hpp"
#include "algorithms/tt.hpp"

ALICE_MAIN( cirkit )
