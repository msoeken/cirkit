#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE rcbdd

#include <boost/test/unit_test.hpp>

#if ADDON_REVLIB

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>

#include <core/utils/timer.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/utils/foreach_function.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/synthesis/embed_pla.hpp>
#include <reversible/synthesis/rcbdd_synthesis.hpp>
#include <reversible/synthesis/young_subgroup_synthesis.hpp>
#include <classical/optimization/exorcism_minimization.hpp>

using namespace boost::assign;

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;

  std::vector<std::string> whitelist;
  //whitelist += "sym6_63","urf2_73","con1_136","hwb9_65","urf1_72","urf5_76","sym9_71","urf3_75","rd84_70","sym10_207","urf4_89";
  //whitelist += "sym6_63","urf2_73","con1_136","hwb9_65","urf1_72","urf5_76","sym9_71","urf3_75","rd84_70","sym10_207","urf4_89","adr4_93","cycle_10_2_61","clip_124","dc2_143","misex1_178","co14_135","urf6_77","dk27_146","t481_208","5xp1_90","C7552_119","apla_107","bw_116";
  whitelist += "C7552_119";
  // extra alu1_94
  foreach_function_with_whitelist( whitelist, []( const boost::filesystem::path& path ) {
    circuit circ;
    rcbdd cf;
    binary_truth_table pla, extended;
    binary_truth_table spec;

    std::cout << "Process: " << path.relative_path().string() << std::endl;
    std::string name = path.stem().string();

    {
      print_timer pt( std::cout );
      timer<print_timer> timer( pt );
      std::cout << " - read:   ";

      read_pla_settings settings;
      settings.extend = false;
      read_pla( pla, path.relative_path().string(), settings );
    }

    {
      print_timer pt( std::cout );
      timer<print_timer> timer( pt );
      std::cout << " - extend: ";

      extend_pla_settings settings;
      extend_pla( pla, extended, settings );

      write_pla( extended, boost::str( boost::format( "logs/rcbdd/%s-extended.pla" ) % name ) );
    }

    {
      print_timer pt( std::cout );
      timer<print_timer> timer( pt );
      std::cout << " - embed:  ";

      //embed_pla( cf, path.relative_path().string() );
      properties::ptr settings( new properties );
      settings->set( "write_pla", boost::str( boost::format( "logs/rcbdd/%s-embedded.pla" ) % name ) );
      embed_pla( cf, boost::str( boost::format( "logs/rcbdd/%s-extended.pla" ) % name ), settings );
    }

    {
      print_timer pt( std::cout );
      timer<print_timer> timer( pt );
      std::cout << " - synth:  ";

      properties::ptr settings( new properties );
      settings->set( "verbose", true );
      settings->set( "progress", true );
      settings->set( "name", name );
      settings->set( "esopmin", dd_based_exorcism_minimization_func() );

      properties::ptr statistics( new properties );

      rcbdd_synthesis( circ, cf, settings, statistics );
      print_statistics( boost::str( boost::format( "logs/rcbdd/%s.log" ) % name ), circ, statistics->get<double>( "runtime" ) );
    }

    write_realization( circ, boost::str( boost::format( "logs/rcbdd/%s.real" ) % name ) );

    {
      print_timer pt( std::cout );
      timer<print_timer> timer( pt );
      std::cout << " - simul:  ";

      circuit_to_truth_table( circ, spec, simple_simulation_func() );
    }

    if ( false )
    {
      circuit ys_circ;

      print_timer pt( std::cout);
      timer<print_timer> timer( pt );
      std::cout << " - vshape: ";

      young_subgroup_synthesis( ys_circ, spec );

      write_realization( ys_circ, boost::str( boost::format( "logs/rcbdd/%s-ys.real" ) % name ) );
    }
  });
}

#else

BOOST_AUTO_TEST_CASE(simple) {}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
