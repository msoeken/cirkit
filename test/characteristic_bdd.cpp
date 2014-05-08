#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE characteristic_bdd

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/test/unit_test.hpp>

#include <core/io/read_pla_to_bdd.hpp>

#include <algorithms/synthesis/dd_synthesis_p.hpp>

#include <cuddInt.h>
#include <gmpxx.h>

using namespace boost::assign;
using namespace revkit;

mpz_class pow2(unsigned n)
{
  return mpz_class("1" + std::string(n, '0'), 2);
}

unsigned calculate_required_lines(unsigned n, unsigned m, mpz_class maxmu)
{
  unsigned exp = 0u;

  while (pow2(exp) < maxmu) {
    ++exp;
    std::cout << "exp: " << exp << std::endl;
  }

  std::cout << "n: " << n << "; m: " << m << std::endl;

  return n > m + exp ? n : m + exp;
}

void count_output_pattern_recurse( const BDDTable& bdd, DdNode* node, const std::string& pattern, unsigned depth, std::vector<mpz_class>& counts )
{
  if ( Cudd_IsConstant( node ) ) { return; }

  if ( depth == *bdd.num_real_outputs )
  {
    std::cout << pattern << " has " << Cudd_CountMinterm( bdd.cudd, node, bdd.inputs.size() - *bdd.num_real_outputs ) << std::endl;
    counts += mpz_class( Cudd_CountMinterm( bdd.cudd, node, bdd.inputs.size() - *bdd.num_real_outputs ) );
  }
  else
  {
    DdNode *t = cuddT( node ), *e = cuddE( node );
    //count_output_pattern_recurse( bdd, Cudd_IsComplement( t ) ? Cudd_Not( t ) : t, depth + 1u, counts );
    //count_output_pattern_recurse( bdd, Cudd_IsComplement( e ) ? Cudd_Not( e ) : e, depth + 1u, counts );
    count_output_pattern_recurse( bdd, Cudd_Regular( t ), pattern + "1", depth + 1u, counts );
    count_output_pattern_recurse( bdd, Cudd_Regular( e ), pattern + "0", depth + 1u, counts );
  }
}

unsigned count_output_pattern( const BDDTable& bdd, bool debug = false )
{
  std::vector<mpz_class> counts;

  DdNode *add = Cudd_BddToAdd( bdd.cudd, bdd.outputs.at( 0 ).second );
  Cudd_Ref( add );

  if ( debug )
  {
    FILE * fp = fopen( "/tmp/test.dot", "w" );
    char ** inames = new char*[bdd.inputs.size()];
    boost::transform( bdd.inputs, inames, []( const std::pair<std::string, DdNode*>& p ) { return const_cast<char*>( p.first.c_str() ); } );
    char* onames[] = { const_cast<char*>( "f" ) };
    Cudd_DumpDot( bdd.cudd, 1, &add, inames, onames, fp );
    fclose( fp );
  }

  count_output_pattern_recurse( bdd, add, "", 0u, counts );

  unsigned n = bdd.inputs.size() - *bdd.num_real_outputs;
  unsigned m = *bdd.num_real_outputs;
  std::cout << "total: " << boost::accumulate( counts, mpz_class( 0u ), []( mpz_class d1, mpz_class d2 ) { return d1 + d2; } ) << std::endl;
  std::cout << "pow2(n): " << pow2(n) << std::endl;
  counts += pow2( n ) - boost::accumulate( counts, mpz_class( 0u ), []( mpz_class d1, mpz_class d2 ) { return d1 + d2; } );
  std::cout << "max: " << *boost::max_element( counts ) << std::endl;
  return calculate_required_lines( n, m, *boost::max_element( counts ) ) - n;
}

BOOST_AUTO_TEST_CASE(simple)
{
  using boost::unit_test::framework::master_test_suite;

  if ( master_test_suite().argc != 2u ) return;

  BDDTable bdd;
  std::cout << "parse ..." << std::endl;
  read_pla_to_characteristic_bdd( bdd, master_test_suite().argv[1], false );

  std::cout << "compute ..." << std::endl;
  unsigned lines = count_output_pattern( bdd, false );

  std::cout << "Additional lines: " << lines << std::endl;

  std::filebuf fb;
  fb.open( "/tmp/lines_characteristic_bdd", std::ios::out );

  std::ostream os( &fb );
  os << lines << std::endl;
  fb.close();
}
