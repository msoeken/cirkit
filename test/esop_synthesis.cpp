#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE esop_synthesis

#include <boost/test/unit_test.hpp>

#include <reversible/circuit.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;

  circuit circ;
  properties::ptr settings( new properties );
  settings->set( "negative_control_lines", true );
  esop_synthesis( circ, "../test/example.esop", settings );

  print_circuit( circ );
}

// Local Variables:
// c-basic-offset: 2
// End:
