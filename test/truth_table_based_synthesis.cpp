#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table_based_synthesis

#include <boost/test/unit_test.hpp>

#if ADDON_REVLIB

#include <vector>

#include <boost/format.hpp>

#include <core/truth_table.hpp>
#include <core/io/read_pla.hpp>
#include <core/io/write_realization.hpp>
#include <core/utils/benchmark_table.hpp>
#include <core/utils/foreach_function.hpp>

#include <algorithms/synthesis/embed_truth_table.hpp>
#include <algorithms/synthesis/reed_muller_synthesis.hpp>
#include <algorithms/synthesis/transformation_based_synthesis.hpp>
#include <algorithms/synthesis/young_subgroup_synthesis.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace revkit;

  /* store results */
  benchmark_table<std::string, unsigned, unsigned, double, unsigned, double, unsigned, double>
    table( { "Benchmark", "n", "TBS", "t", "RMS", "t", "YSG", "t" } );

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
    circuit circ_tbs, circ_rms, circ_ysg;
    properties::ptr tbs_statistics( new properties );
    transformation_based_synthesis( circ_tbs, spec, properties::ptr(), tbs_statistics );
    properties::ptr rms_statistics( new properties );
    reed_muller_synthesis( circ_rms, spec, properties::ptr(), rms_statistics );
    properties::ptr ysg_statistics( new properties );
    young_subgroup_synthesis( circ_ysg, spec, properties::ptr(), ysg_statistics );

    write_realization( circ_ysg, boost::str( boost::format( "/tmp/%s.real" ) % benchmark ) );

    table.add( benchmark, spec.num_inputs(), circ_tbs.num_gates(), tbs_statistics->get<double>( "runtime" ), circ_rms.num_gates(), rms_statistics->get<double>( "runtime" ), circ_ysg.num_gates(), ysg_statistics->get<double>( "runtime" ) );
  });

  /* generate table */
  table.print();
}

#else

BOOST_AUTO_TEST_CASE(simple) {}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
