/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "calculate_additional_lines.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/timer.hpp>

#include <reversible/synthesis/dd_synthesis_p.hpp>
#include <reversible/synthesis/synthesis_utils_p.hpp>

#include <cuddInt.h>
#include <gmpxx.h>

using namespace boost::assign;

namespace cirkit
{

void count_output_pattern_recurse( DdManager* mgr, DdNode* node, unsigned num_inputs, unsigned num_outputs,
                                   const std::string& pattern, unsigned depth,
                                   std::vector<mpz_class>& counts, std::vector<std::string>& patterns,
                                   bool verbose )
{
  if ( Cudd_IsConstant( node ) ) { return; }

  if ( depth == num_outputs )
  {
    if ( verbose )
    {
      std::cout << pattern << " has " << Cudd_CountMinterm( mgr, node, num_inputs ) << std::endl;
    }
    patterns += pattern;
    counts += mpz_class( Cudd_CountMinterm( mgr, node, num_inputs ) );
  }
  else
  {
    count_output_pattern_recurse( mgr, Cudd_Regular( cuddT( node ) ), num_inputs, num_outputs, pattern + "1", depth + 1u, counts, patterns, verbose );
    count_output_pattern_recurse( mgr, Cudd_Regular( cuddE( node ) ), num_inputs, num_outputs, pattern + "0", depth + 1u, counts, patterns, verbose );
  }
}

unsigned calculate_additional_lines( const std::string& filename, properties::ptr settings, properties::ptr statistics )
{
  /* Settings */
  auto verbose        = get( settings, "verbose",        false         );
  auto dotname        = get( settings, "dotname",        std::string() );
  auto dumpadd        = get( settings, "dumpadd",        false         );
  auto explicit_zeros = get( settings, "explicit_zeros", false         );

  /* Timer */
  properties_timer t( statistics );

  BDDTable bdd;
  read_pla_to_characteristic_bdd( bdd, filename, false, explicit_zeros );

  std::vector<mpz_class>   counts;
  std::vector<std::string> patterns;

  DdNode *add = Cudd_BddToAdd( bdd.cudd, bdd.outputs.at( 0u ).second );
  Cudd_Ref( add );

  if ( !dotname.empty() )
  {
    FILE * fp = fopen( dotname.c_str(), "w" );
    char ** inames = new char*[bdd.inputs.size()];
    boost::transform( bdd.inputs, inames, []( const std::pair<std::string, DdNode*>& p ) { return const_cast<char*>( p.first.c_str() ); } );
    char* onames[] = { const_cast<char*>( "f" ) };
    if ( dumpadd )
    {
      Cudd_DumpDot( bdd.cudd, 1, &add, inames, onames, fp );
    }
    else
    {
      Cudd_DumpDot( bdd.cudd, 1, &bdd.outputs.at( 0u ).second, inames, onames, fp );
    }
    fclose( fp );
  }

  unsigned n = bdd.inputs.size() - *bdd.num_real_outputs;
  unsigned m = *bdd.num_real_outputs;

  count_output_pattern_recurse( bdd.cudd, add, n, m, "", 0u, counts, patterns, verbose );

  if ( verbose )
  {
    std::cout << "total: " << boost::accumulate( counts, mpz_class( 0u ), []( mpz_class d1, mpz_class d2 ) { return d1 + d2; } ) << std::endl;
    std::cout << "pow2(n): " << pow2(n) << std::endl;
  }
  counts += pow2( n ) - boost::accumulate( counts, mpz_class( 0u ), []( mpz_class d1, mpz_class d2 ) { return d1 + d2; } );
  if ( verbose )
  {
    std::cout << "max: " << *boost::max_element( counts ) << std::endl;
  }

  /* Statistics */
  set( statistics, "num_inputs", n );
  set( statistics, "num_outputs", m );
  set( statistics, "patterns", patterns );

  return calculate_required_lines( n, m, *boost::max_element( counts ) ) - n;
}

unsigned calculate_additional_lines( const bdd_function_t& bdd, properties::ptr settings, properties::ptr statistics )
{
  /* Settings */
  const auto verbose = get( settings, "verbose", false );

  /* Timer */
  properties_timer t( statistics );

  const auto cf = compute_characteristic( bdd, false );
  const auto add = cf.second[0].Add();

  std::vector<mpz_class> counts;
  std::vector<std::string> patterns;
  const unsigned n = bdd.first.ReadSize();
  const unsigned m = bdd.second.size();

  count_output_pattern_recurse( cf.first.getManager(), add.getNode(), n, m, "", 0u, counts, patterns, verbose );

  /* statistics */
  set( statistics, "num_inputs", n );
  set( statistics, "num_outputs", m );
  set( statistics, "patterns", patterns );

  return calculate_required_lines( n, m, *boost::max_element( counts ) ) - n;
}

}


// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
