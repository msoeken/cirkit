/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table_based_synthesis

#include <boost/test/unit_test.hpp>

#include <vector>

#include <boost/format.hpp>

#include <core/utils/benchmark_table.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/foreach_function.hpp>
#include <reversible/synthesis/embed_truth_table.hpp>
#include <reversible/synthesis/reed_muller_synthesis.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>
#include <reversible/synthesis/young_subgroup_synthesis.hpp>

#include <classical/optimization/esop_minimization.hpp>
#include <classical/optimization/exorcism_minimization.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;

  /* store results */
  benchmark_table<std::string, unsigned, unsigned, double, unsigned, double, unsigned, double, unsigned, double>
    table( { "Benchmark", "n", "TBS", "t", "RMS", "t", "YSG", "t", "YSG2", "t" } );

  std::vector<std::string> whitelist = { "3_17_6","4_49_7","4gt10_22","4gt11_23","4gt12_24","4gt13_25","4mod5_8","decod24_10","ham3_28","ham7_29","hwb7_15","hwb8_64","hwb9_65","mod5d1_16","mod5d2_16","rd32_19","rd73_69","sym9_71" };
  foreach_function_with_whitelist( whitelist, [&table]( const boost::filesystem::path& path ) {
    std::string benchmark = path.stem().string();

    /* output */
    std::cout << "Processing " << benchmark << "..." << std::endl;

    /* filename */
    std::string filename = path.relative_path().string();

    /* embedding */
    binary_truth_table pla, spec;
    read_pla_settings settings;
    settings.extend = true;
    read_pla( pla, path.relative_path().string(), settings );
    embed_truth_table( spec, pla );

    /* synthesis */
    circuit circ_tbs, circ_rms, circ_ysg, circ_ysg2;

    properties::ptr tbs_statistics( new properties );
    transformation_based_synthesis( circ_tbs, spec, properties::ptr(), tbs_statistics );

    properties::ptr rms_statistics( new properties );
    reed_muller_synthesis( circ_rms, spec, properties::ptr(), rms_statistics );

    properties::ptr ysg_settings( new properties );
    ysg_settings->set( "esopmin", dd_based_esop_minimization_func() );
    properties::ptr ysg_statistics( new properties );
    young_subgroup_synthesis( circ_ysg, spec, ysg_settings, ysg_statistics );

    properties::ptr ysg2_settings( new properties );
    ysg2_settings->set( "esopmin", dd_based_exorcism_minimization_func() );
    properties::ptr ysg2_statistics( new properties );
    young_subgroup_synthesis( circ_ysg2, spec, ysg2_settings, ysg2_statistics );

    table.add( benchmark, spec.num_inputs(), circ_tbs.num_gates(), tbs_statistics->get<double>( "runtime" ), circ_rms.num_gates(), rms_statistics->get<double>( "runtime" ), circ_ysg.num_gates(), ysg_statistics->get<double>( "runtime" ), circ_ysg2.num_gates(), ysg2_statistics->get<double>( "runtime" ) );
  });

  /* generate table */
  table.print();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
