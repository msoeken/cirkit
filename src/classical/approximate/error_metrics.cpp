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

#include "error_metrics.hpp"

#include <cmath>
#include <stack>

#include <boost/algorithm/string/join.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/combine.hpp>
#include <boost/range/numeric.hpp>
#include <boost/version.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/dd/arithmetic.hpp>
#include <classical/dd/bdd_to_truth_table.hpp>
#include <classical/dd/characteristic.hpp>
#include <classical/dd/count_solutions.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

inline void assert_valid( const std::vector<bdd>& f, const std::vector<bdd>& fhat )
{
  assert( f.size() == fhat.size() );
  assert( !f.empty() );
}

std::vector<bdd> compute_diff( const std::vector<bdd>& f, const std::vector<bdd>& fhat, bool print_truthtables = false )
{
  assert_valid( f, fhat );

  const auto to = f.size() + 1u;

  const auto zf    = zero_extend( f, to );
  const auto zfhat = zero_extend( fhat, to );
  const auto subtr = bdd_subtract( zf, zfhat );
  const auto diff  = bdd_abs( subtr );

#if ( BOOST_VERSION / 100000 ) >= 1 && ( BOOST_VERSION / 100 % 1000 ) >= 56
  if ( print_truthtables )
  {
    using boost::adaptors::transformed;
    using boost::format;
    using boost::str;

    auto valid = true;
    auto max   = 0u;
    auto sum   = 0u;

    std::cout << std::endl << "    f    zf  fhat zfhat subtr  diff valid" << std::endl
              << boost::join( boost::combine( bdds_to_truth_table_unsigned( f ),
                                              bdds_to_truth_table_unsigned( zf ),
                                              bdds_to_truth_table_unsigned( fhat ),
                                              bdds_to_truth_table_unsigned( zfhat ),
                                              bdds_to_truth_table_signed( subtr ),
                                              bdds_to_truth_table_unsigned( diff ) )
                              | transformed( [&]( const boost::tuple<unsigned, unsigned, unsigned, unsigned, int, unsigned>& p ) {
                                  const auto vf     = boost::get<0>( p );
                                  const auto vzf    = boost::get<1>( p );
                                  const auto vfhat  = boost::get<2>( p );
                                  const auto vzfhat = boost::get<3>( p );
                                  const auto vsubtr = boost::get<4>( p );
                                  const auto vdiff  = boost::get<5>( p );

                                  const auto row_valid = ( vf == vzf ) && ( vfhat == vzfhat ) && ( vsubtr == static_cast<int>( vzf - vzfhat ) ) && ( abs( vsubtr ) == static_cast<int>( vdiff ) );
                                  valid = valid && row_valid;
                                  max   = std::max( max, vdiff );
                                  sum  += vdiff;

                                  return str( format( "%5d %5d %5d %5d %5d %5d     %d") % vf % vzf % vfhat % vzfhat % vsubtr % vdiff % row_valid );
                                } ), "\n" ) << std::endl
              << format( "valid: %d, max: %d, avg: %.2f" ) % valid % max % ( sum / pow( 2.0, f.front().manager->num_vars() ) ) << std::endl;
  }
#endif

  assert( diff.size() == to );
  assert( diff.back().index == 0u );

  return diff;
}

boost::multiprecision::uint256_t get_max_value( const std::vector<bdd>& f )
{
  assert( !f.empty() );

  boost::dynamic_bitset<> bs( f.size() );
  auto mask = f.front().manager->bdd_top();

  for ( int k = f.size() - 1; k >= 0; --k )
  {
    auto r = mask && f[k];
    if ( r.index != 0 )
    {
      bs.set( k );
      mask = r;
    }
  }

  return to_multiprecision<boost::multiprecision::uint256_t>( bs );
}

boost::multiprecision::uint256_t get_max_value_with_chi( const std::vector<bdd>& f )
{
  bdd_manager mgr_chi( f.front().manager->num_vars() + f.size(), 10u ); /* can we approximate the number of used nodes? */

  auto fr  = f; boost::reverse( fr );
  auto chi = characteristic_function( fr, mgr_chi );

  boost::dynamic_bitset<> bs( f.size() );

  while ( chi.var() < f.size() )
  {
    if ( chi.high().is_bot() )
    {
      chi = chi.low();
    }
    else
    {
      bs.set( f.size() - 1u - chi.var() );
      chi = chi.high();
    }
  }

  return to_multiprecision<boost::multiprecision::uint256_t>( bs );
}

boost::multiprecision::uint256_t get_weighted_sum( const std::vector<bdd>& f )
{
  auto level = f.size();
  bdd_manager mgr_chi( f.front().manager->num_vars() + level, 10u ); /* can we approximate the number of used nodes? */

  auto fr = f; boost::reverse( fr );
  auto chi = characteristic_function( fr, mgr_chi );

  boost::multiprecision::uint256_t sum = 0;
  const boost::multiprecision::uint256_t one = 1;
  std::stack<std::pair<bdd, boost::dynamic_bitset<>>> stack;

  stack.push( {chi, boost::dynamic_bitset<>( level )} );

  while ( !stack.empty() )
  {
    auto p = stack.top(); stack.pop();

    if ( p.first.var() >= level )
    {
      sum += to_multiprecision<boost::multiprecision::uint256_t>( p.second ) * ( count_solutions( p.first ) / ( one << level ) );
    }
    else
    {
      stack.push( {p.first.low(), p.second} );
      p.second.set( p.first.var() );
      stack.push( {p.first.high(), p.second} );
    }
  }

  return sum;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

boost::multiprecision::uint256_t error_rate( const std::vector<bdd>& f, const std::vector<bdd>& fhat,
                                             const properties::ptr& settings,
                                             const properties::ptr& statistics )
{
  properties_timer t( statistics );

  assert_valid( f, fhat );

  auto h = f.front().manager->bdd_bot();

  for ( auto i = 0u; i < f.size(); ++i )
  {
    h = h || ( f[i] ^ fhat[i] );
  }

  return count_solutions( h );
}

boost::multiprecision::uint256_t worst_case( const std::vector<bdd>& f, const std::vector<bdd>& fhat,
                                             const properties::ptr& settings,
                                             const properties::ptr& statistics )
{
  auto print_truthtables = get( settings, "print_truthtables", false );
  auto maximum_method    = get( settings, "maximum_method",    worst_case_maximum_method::shift );

  properties_timer t( statistics );

  auto diff = compute_diff( f, fhat, print_truthtables );

  switch ( maximum_method )
  {
  case worst_case_maximum_method::shift:
    return get_max_value( diff );
  case worst_case_maximum_method::chi:
    return get_max_value_with_chi( diff );
  default:
    assert( false );
  }
}

boost::multiprecision::cpp_dec_float_100 average_case( const std::vector<bdd>& f, const std::vector<bdd>& fhat,
                                                       const properties::ptr& settings, const properties::ptr& statistics )
{
  auto print_truthtables = get( settings, "print_truthtables", false );

  properties_timer t( statistics );

  auto diff = compute_diff( f, fhat, print_truthtables );

  const boost::multiprecision::uint256_t one = 1;
  return boost::multiprecision::cpp_dec_float_100( get_weighted_sum( diff ) ) /
         boost::multiprecision::cpp_dec_float_100( one << f.front().manager->num_vars() );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
