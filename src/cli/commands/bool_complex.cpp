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

#include "bool_complex.hpp"

#include <unordered_set>

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/functions/npn_canonization.hpp>

using namespace boost::program_options;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool_complex_command::bool_complex_command( const environment::ptr& env )
  : cirkit_command( env, "Computes complexities of Boolean functions" )
{
  opts.add_options()
    ( "numvars,n", value_with_default( &numvars ), "number of variables (n <= 4)" )
    ( "count,c",                                   "compute upper bound on size" )
    ( "lengths,l",                                 "compute normal lengths" )
    ( "depths,d",                                  "compute depths" )
    ( "lengths_maj",                               "compute normal lengths (MAJ)" )
    ( "depths_maj",                                "compute depths (MAJ)" )
    ( "npn",                                       "compute number of NPN classes" )
    ;
  add_positional_option( "numvars" );
}

bool bool_complex_command::execute()
{
  if ( is_set( "count" ) )
  {
    compute_ub_aig();
  }
  if ( is_set( "lengths" ) )
  {
    compute( true );
  }
  if ( is_set( "depths" ) )
  {
    compute( false );
  }
  if ( is_set( "lengths_maj" ) )
  {
    compute_maj( true, is_set( "npn" ) );
  }
  if ( is_set( "depths_maj" ) )
  {
    compute_maj( false, is_set( "npn" ) );
  }

  return true;
}

void bool_complex_command::compute( bool length )
{
  const auto none = (unsigned)-1;
  auto count      = 1u << ( ( 1u << numvars ) - 1u );

  std::vector<unsigned>              func_to_length( count, none );
  std::vector<std::vector<unsigned>> length_to_func;

  /* constant function */
  func_to_length[0u] = 0u;
  length_to_func.push_back( {0u} );

  /* one-variable functions */
  for ( auto k = 0u; k < numvars; ++k )
  {
    const auto xk = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (k + 1)))) + 1);
    func_to_length[xk] = 0;
    length_to_func.back().push_back( xk );
  }

  count -= ( 1u + numvars );
  auto current_length = 0u;

  while ( true )
  {
    ++current_length;

    length_to_func.push_back( {} );

    int j = 0u;
    int k = current_length - 1;

    do
    {
      for ( auto g = 0u; g < length_to_func[j].size(); ++g )
      {
        const auto& fg = length_to_func[j][g];
        for ( auto h = 0u; h < length_to_func[k].size(); ++h )
        {
          const auto& fh = length_to_func[k][h];

          if ( fg == fh ) { continue; }

          for ( const auto& f : { fg & fh, ~fg & fh, fg & ~fh, fg | fh, fg ^ fh } )
          {
            if ( func_to_length[f] == none )
            {
              func_to_length[f] = current_length;
              length_to_func.back().push_back( f );

              if ( --count == 0u ) { goto done; }
            }
          }
        }
      }

      ++j;
      if ( length ) { --k; }
    } while ( j <= k );
  }

done:
  for ( const auto& entry : index( length_to_func ) )
  {
    std::cout << boost::format( "[i] %s %3d: %12d" ) % ( length ? "length" : "depth" ) % entry.index % ( entry.value.size() << 1u ) << std::endl;
  }
}

void bool_complex_command::compute_ub()
{
  const auto none = (unsigned)-1;
  auto count      = 1u << ( ( 1u << numvars ) - 1u );

  std::vector<unsigned>              func_to_count( count, none );
  std::vector<std::vector<unsigned>> count_to_func;
  std::vector<unsigned>              func_to_footprint( count, 0u );

  /* constant function */
  func_to_count[0u] = 0u;
  count_to_func.push_back( {0u} );

  /* one-variable functions */
  for ( auto k = 0u; k < numvars; ++k )
  {
    const auto xk = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (k + 1)))) + 1);
    func_to_count[xk] = 0;
    count_to_func.back().push_back( xk );
  }

  /* compute initial footprints */
  count_to_func.push_back( {} );
  auto footprint = 0u;
  for ( auto op = 0u; op < 5u; ++op )
  {
    for ( auto i = 0u; i < numvars; ++i )
    {
      const auto xi = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (i + 1)))) + 1);

      for ( auto j = i + 1u; j < numvars; ++j )
      {
	const auto xj = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (j + 1)))) + 1);
	auto f = 0u;

	switch ( op )
	{
	case 0u:
	  f = xi & xj; break;
	case 1u:
	  f = ~xi & xj; break;
	case 2u:
	  f = xi & ~xj; break;
	case 3u:
	  f = xi | xj; break;
	case 4u:
	  f = xi ^ xj; break;
	default: assert( false );
	}

	func_to_footprint[f] = 1 << footprint++;
	func_to_count[f] = 1u;
	count_to_func.back().push_back( f );
      }
    }
  }
  const auto count_at_one = 5 * ( ( numvars * ( numvars - 1 ) ) / 2 );
  assert( footprint == count_at_one );

  count -= ( 1u + numvars + count_at_one );
  auto current_length = 1u;

  while ( count )
  {
    ++current_length; /* r */
    count_to_func.push_back( {} );

    for ( int j = ( current_length - 1 ) / 2; j >= 0; --j )
    {
      int k = current_length - 1 - j;

      for ( auto g = 0u; g < count_to_func[j].size(); ++g )
      {
	const auto& fg = count_to_func[j][g];
	for ( auto h = 0u; h < count_to_func[k].size(); ++h )
	{
	  const auto& fh = count_to_func[k][h];

	  if ( fg == fh ) { continue; }

	  unsigned u{}, v{};
	  if ( func_to_footprint[fg] & func_to_footprint[fh] )
	  {
	    u = current_length - 1;
	    v = func_to_footprint[fg] & func_to_footprint[fh];
	  }
	  else
	  {
	    u = current_length;
	    v = func_to_footprint[fg] | func_to_footprint[fh];
	  }

	  for ( const auto& f : { fg & fh, ~fg & fh, fg & ~fh, fg | fh, fg ^ fh } )
	  {
	    if ( func_to_count[f] == none )
	    {
	      func_to_count[f] = u;
	      func_to_footprint[f] = v;
	      count_to_func[u].push_back( f );

	      --count;
	    }
	    else if ( func_to_count[f] > u )
	    {
	      auto& old_list = count_to_func[func_to_count[f]];
	      const auto it = std::find( old_list.begin(), old_list.end(), f );
	      assert( it != old_list.end() );
	      old_list.erase( it );
	      func_to_count[f] = u;
	      func_to_footprint[f] = v;
	      count_to_func[u].push_back( f );
	    }
	    else if ( func_to_count[f] == u && func_to_footprint[f] != ( func_to_footprint[f] | v ) )
	    {
	      func_to_footprint[f] |= v;
	    }
	  }
	}
      }
    }
  }

  for ( const auto& entry : index( count_to_func ) )
  {
    std::cout << boost::format( "[i] count %3d: %12d" ) % entry.index % ( entry.value.size() << 1u ) << std::endl;
  }
}

void bool_complex_command::compute_ub_aig()
{
  const auto none = (unsigned)-1;
  auto count      = 1u << ( ( 1u << numvars ) - 1u );

  std::vector<unsigned>              func_to_count( count, none );
  std::vector<std::vector<unsigned>> count_to_func;
  std::vector<unsigned>              func_to_footprint( count, 0u );

  /* constant function */
  func_to_count[0u] = 0u;
  count_to_func.push_back( {0u} );

  /* one-variable functions */
  for ( auto k = 0u; k < numvars; ++k )
  {
    const auto xk = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (k + 1)))) + 1);
    func_to_count[xk] = 0;
    count_to_func.back().push_back( xk );
  }

  /* compute initial footprints */
  count_to_func.push_back( {} );
  auto footprint = 0u;
  for ( auto op = 0u; op < 4u; ++op )
  {
    for ( auto i = 0u; i < numvars; ++i )
    {
      const auto xi = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (i + 1)))) + 1);

      for ( auto j = i + 1u; j < numvars; ++j )
      {
	const auto xj = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (j + 1)))) + 1);
	auto f = 0u;

	switch ( op )
	{
	case 0u:
	  f = xi & xj; break;
	case 1u:
	  f = ~xi & xj; break;
	case 2u:
	  f = xi & ~xj; break;
	case 3u:
	  f = xi | xj; break;
	default: assert( false );
	}

	func_to_footprint[f] = 1 << footprint++;
	func_to_count[f] = 1u;
	count_to_func.back().push_back( f );
      }
    }
  }
  const auto count_at_one = 4 * ( ( numvars * ( numvars - 1 ) ) / 2 );
  assert( footprint == count_at_one );

  count -= ( 1u + numvars + count_at_one );
  auto current_length = 1u;

  while ( count )
  {
    ++current_length; /* r */
    count_to_func.push_back( {} );

    for ( int j = ( current_length - 1 ) / 2; j >= 0; --j )
    {
      int k = current_length - 1 - j;

      for ( auto g = 0u; g < count_to_func[j].size(); ++g )
      {
	const auto& fg = count_to_func[j][g];
	for ( auto h = 0u; h < count_to_func[k].size(); ++h )
	{
	  const auto& fh = count_to_func[k][h];

	  if ( fg == fh ) { continue; }

	  unsigned u{}, v{};
	  if ( func_to_footprint[fg] & func_to_footprint[fh] )
	  {
	    u = current_length - 1;
	    v = func_to_footprint[fg] & func_to_footprint[fh];
	  }
	  else
	  {
	    u = current_length;
	    v = func_to_footprint[fg] | func_to_footprint[fh];
	  }

	  for ( const auto& f : { fg & fh, ~fg & fh, fg & ~fh, fg | fh } )
	  {
	    if ( func_to_count[f] == none )
	    {
	      func_to_count[f] = u;
	      func_to_footprint[f] = v;
	      count_to_func[u].push_back( f );

	      --count;
	    }
	    else if ( func_to_count[f] > u )
	    {
	      auto& old_list = count_to_func[func_to_count[f]];
	      const auto it = std::find( old_list.begin(), old_list.end(), f );
	      assert( it != old_list.end() );
	      old_list.erase( it );
	      func_to_count[f] = u;
	      func_to_footprint[f] = v;
	      count_to_func[u].push_back( f );
	    }
	    else if ( func_to_count[f] == u && func_to_footprint[f] != ( func_to_footprint[f] | v ) )
	    {
	      func_to_footprint[f] |= v;
	    }
	  }
	}
      }
    }
  }

  for ( const auto& entry : index( count_to_func ) )
  {
    std::cout << boost::format( "[i] count %3d: %12d" ) % entry.index % ( entry.value.size() << 1u ) << std::endl;
  }
}

inline unsigned maj( unsigned a, unsigned b, unsigned c )
{
  return ( a & b ) | ( a & c ) | ( b & c );
}

void bool_complex_command::compute_maj( bool length, bool npn )
{
  const auto none = (unsigned)-1;
  auto count      = 1u << ( ( 1u << numvars ) - 1u );

  std::vector<unsigned>                     func_to_length( count, none );
  std::vector<std::vector<unsigned>>        length_to_func;
  std::vector<std::unordered_set<unsigned>> length_to_npn;

  /* constant function */
  func_to_length[0u] = 0u;
  length_to_func.push_back( {0u} );

  /* one-variable functions */
  for ( auto k = 0u; k < numvars; ++k )
  {
    const auto xk = ((1 << (1 << numvars)) - 1) / ((1 << (1 << (numvars - (k + 1)))) + 1);
    func_to_length[xk] = 0;
    length_to_func.back().push_back( xk );
  }

  if ( npn )
  {
    length_to_npn.push_back( std::unordered_set<unsigned>() );
    length_to_npn.back().insert( 0u );
    length_to_npn.back().insert( (1 << (1 << (numvars - 1))) - 1 );
  }

  count -= ( 1u + numvars );
  auto current_length = 0u;

  while ( true )
  {
    ++current_length;

    length_to_func.push_back( {} );

    if ( npn )
    {
      length_to_npn.push_back( std::unordered_set<unsigned>() );
    }

    auto j = 0u;
    auto k = 0u;
    int l = current_length - 1;

    do
    {
      for ( auto g = 0u; g < length_to_func[j].size(); ++g )
      {
        const auto& fg = length_to_func[j][g];
        for ( auto h = ( j == k ) ? ( g + 1u ) : 0u; h < length_to_func[k].size(); ++h )
        {
          const auto& fh = length_to_func[k][h];
          if ( fg == fh ) { continue; }

          for ( auto i = ( static_cast<int>( k ) == l ) ? ( h + 1u ) : ( ( static_cast<int>( j ) == l ) ? ( g + 1u ) : 0u ); i < length_to_func[l].size(); ++i )
          {
            const auto& fi = length_to_func[l][i];
            if ( fg == fi || fh == fi ) { continue; }

            for ( const auto& f : { maj( fg, fh, fi ), maj( fg, fh, ~fi ), maj( fg, ~fh, fi ), maj( ~fg, fh, fi ) } )
            {
              if ( func_to_length[f] == none )
              {
                func_to_length[f] = current_length;
                length_to_func.back().push_back( f );

                if ( npn )
                {
                  boost::dynamic_bitset<> phase;
                  std::vector<unsigned> perm;
                  auto tt_npn = exact_npn_canonization( tt( 1u << numvars, f ), phase, perm );

                  length_to_npn.back().insert( tt_npn.to_ulong() );
                }

                if ( --count == 0u ) { goto done; }
              }
            }
          }
        }
      }

      if ( length )
      {
        if ( j + k == ( current_length - 1 ) )
        {
          ++j;
          k = j;
        }
        else
        {
          ++k;
        }
        l = current_length - 1 - j - k;
      }
      else
      {
        if ( k == current_length - 1 )
        {
          ++j;
          k = j;
        }
        else
        {
          ++k;
        }
        if ( j == current_length )
        {
          l = -1;
        }
      }
    } while ( l >= 0 );
  }

done:
  for ( const auto& entry : index( length_to_func ) )
  {
    std::cout << boost::format( "[i] %s %3d: %12d" ) % ( length ? "length" : "depth" ) % entry.index % ( entry.value.size() << 1u );

    if ( npn )
    {
      std::cout << boost::format( ", npn classes: %5d" ) % length_to_npn[entry.index].size();
    }

    std::cout << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
