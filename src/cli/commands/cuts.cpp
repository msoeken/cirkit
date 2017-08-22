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

#include "cuts.hpp"

#include <iostream>

#include <boost/format.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <classical/functions/cuts/paged.hpp>
#include <classical/mig/mig_cuts_paged.hpp>
#include <classical/functions/cuts/traits.hpp>
#include <classical/utils/cut_enumeration.hpp>
#include <classical/utils/truth_table_utils.hpp>

using namespace boost::program_options;
using boost::format;

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

cuts_command::cuts_command( const environment::ptr& env ) : aig_mig_command( env, "Computes cuts of an AIG", "Enumerate cuts for %s" )
{
  opts.add_options()
    ( "node_count,k", value_with_default( &node_count ), "Number of nodes in a cut" )
    ( "truthtable,t",                                    "Prints truth tables when verbose" )
    ( "cone_count,c",                                    "Prints nodes in cut cone when verbose" )
    ( "depth,d",                                         "Prints depth of cut when verbose " )
    ( "parallel",                                        "Parallel cut enumeration for AIGs" )
    ;
  be_verbose();
}

bool cuts_command::execute_aig()
{
  paged_aig_cuts cuts( aig(), node_count, is_set( "parallel" ) );
  std::cout << boost::format( "[i] found %d cuts in %.2f secs (%d KB)" ) % cuts.total_cut_count() % cuts.enumeration_time() % ( cuts.memory() >> 10u ) << std::endl;

  if ( is_verbose() )
  {
    for ( const auto& p : boost::make_iterator_range( vertices( aig() ) ) )
    {
      std::cout << boost::format( "[i] node %d has %d cuts" ) % p % cuts.count( p ) << std::endl;
      for ( const auto& cut : cuts.cuts( p ) )
      {
        std::cout << "[i] - {" << any_join( cut.range(), ", " ) << "}";

        if ( is_set( "cone_count" ) )
        {
          std::cout << format( " (size: %d)" ) % cut.size();
        }

        if ( is_set( "truthtable" ) )
        {
          std::cout << " " << tt_to_hex( cuts.simulate( p, cut ) );
        }

        std::cout << std::endl;
      }
    }
  }

  return true;
}

bool cuts_command::execute_mig()
{
  mig_cuts_paged cuts( mig(), node_count );
  std::cout << boost::format( "[i] found %d cuts in %.2f secs (%d KB)" ) % cuts.total_cut_count() % cuts.enumeration_time() % ( cuts.memory() >> 10u ) << std::endl;

  if ( is_verbose() )
  {
    for ( const auto& p : boost::make_iterator_range( vertices( mig() ) ) )
    {
      std::cout << boost::format( "[i] node %d has %d cuts" ) % p % cuts.count( p ) << std::endl;
      for ( const auto& cut : cuts.cuts( p ) )
      {
        std::cout << "[i] - {" << any_join( cut.range(), ", " ) << "}";

        if ( is_set( "cone_count" ) )
        {
          std::cout << format( " (size: %d)" ) % cuts.size( p, cut );
        }

        if ( is_set( "depth" ) )
        {
          std::cout << format( " (depth: %d)" ) % cuts.depth( p, cut );
        }

        if ( is_set( "truthtable" ) )
        {
          std::cout << " " << tt_to_hex( cuts.simulate( p, cut ) );
        }

        std::cout << std::endl;
      }
    }
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
