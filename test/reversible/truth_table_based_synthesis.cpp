/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2015  University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
