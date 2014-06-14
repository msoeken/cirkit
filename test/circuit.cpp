#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE circuit

#include <boost/assign/std/vector.hpp>
#include <boost/test/unit_test.hpp>

#include <reversible/circuit.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>

BOOST_AUTO_TEST_CASE(simple)
{
  using namespace cirkit;
  using namespace boost::assign;

  circuit circ( 3u );
  BOOST_CHECK( circ.lines() == 3u );

  std::vector<unsigned> controls;
  controls += 0u,1u;
  append_toffoli( circ, controls, 2u );
  append_toffoli( circ )( 0u, 1u )( 2u );
  append_cnot( circ, 0u, 1u );
  append_not( circ, 0u );

  BOOST_CHECK( circ.num_gates() == 4u );
  unsigned i = 0u;
  for ( auto g : circ ) {
    switch (i) {
    case 0u:
      BOOST_CHECK( g.size() == 3u );
      break;
    case 1u:
      BOOST_CHECK( g.size() == 3u );
      break;
    case 2u:
      BOOST_CHECK( g.size() == 2u );
      break;
    case 3u:
      BOOST_CHECK( g.size() == 1u );
      break;
    }
    ++i;
  }
  BOOST_CHECK( i == 4u );
}
