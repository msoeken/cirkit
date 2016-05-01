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
  : command( env, "Computes complexities of Boolean functions" )
{
  opts.add_options()
    ( "numvars,n", value_with_default( &numvars ), "Number of variables (n <= 4)" )
    ( "lengths,l",                                 "Compute normal lengths" )
    ( "depths,d",                                  "Compute depths" )
    ( "lengths_maj",                               "Compute normal lengths (MAJ)" )
    ( "depths_maj",                                "Compute depths (MAJ)" )
    ( "npn",                                       "Compute number of NPN classes" )
    ;
}

bool bool_complex_command::execute()
{
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

    int j = 0u;
    int k = 0u;
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

          for ( auto i = ( k == l ) ? ( h + 1u ) : ( ( j == l ) ? ( g + 1u ) : 0u ); i < length_to_func[l].size(); ++i )
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
