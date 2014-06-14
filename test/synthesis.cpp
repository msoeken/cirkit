#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE truth_table

#include <cmath>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/irange.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/synthesis/reed_muller_synthesis.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>
#include <reversible/synthesis/transposition_based_synthesis.hpp>

using namespace revkit;

void add_entries_from_permutation( binary_truth_table& spec, const std::vector<unsigned>& permutation )
{
  using boost::combine;
  using boost::irange;
  using boost::get;

  unsigned n = (unsigned)ceil( log( permutation.size() ) / log( 2 ) );

  for ( const auto& i : combine( irange( 0u, (unsigned)permutation.size() ),
                                 permutation ) )
  {
    spec.add_entry( number_to_truth_table_cube( get<0>( i ), n ),
                    number_to_truth_table_cube( get<1>( i ), n ) );
  }
}

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::test_tools::output_test_stream;

  using namespace boost::assign;

  // Construct truth table from benchmark
  std::vector<unsigned> permutation;
  permutation += 7u,0u,1u,3u,4u,2u,6u,5u; // 3_17 benchmark
  binary_truth_table spec;
  add_entries_from_permutation( spec, permutation );

  // Synthesize and compare with different methods
  std::vector<truth_table_synthesis_func> funcs;
  funcs += transformation_based_synthesis_func(),reed_muller_synthesis_func(),transposition_based_synthesis_func();

  output_test_stream output;
  output << spec;

  for ( const auto& f : funcs )
  {
    std::stringstream new_stream;

    circuit circ;
    binary_truth_table new_spec;

    f( circ, spec );
    circuit_to_truth_table( circ, new_spec, simple_simulation_func() );

    new_stream << new_spec;

    BOOST_CHECK( output.is_equal( new_stream.str(), false ) );
  }
}
