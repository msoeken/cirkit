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

#include "read_pattern.hpp"

#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <reversible/circuit.hpp>
#include <reversible/pattern.hpp>

using namespace boost::assign;

namespace cirkit
{

  bool read_pattern( pattern& p, const std::string& filename, std::string* error )
  {
    std::string line;
    std::ifstream patternfile( filename.c_str() );
    if ( patternfile.is_open() == false )
    {
      if ( error )
      {
        *error = "Couldn't open patternfile.";
      }
      return false;
    }

    while ( patternfile.good() && getline( patternfile, line ) )
    {
      boost::algorithm::trim( line );

      /* Skip empty lines */
      if ( !line.size() )
      {
        continue;
      }

      std::vector<std::string> params;
      boost::algorithm::split( params, line, boost::algorithm::is_any_of( " " ) );

      /* It is possible that there are empty elements in params,
         e.g. when line contains two spaces between identifiers instead of one.
         These should be removed. */
      std::vector<std::string>::iterator newEnd = std::remove( params.begin(), params.end(), "" );
      params.erase( newEnd, params.end() );

      /* By means of the first element we can determine the command */
      std::string command = params.front();
      params.erase( params.begin() );

      if ( command == ".version" )
      {
        // Do nothing right now
      }

      else if ( command == ".init" )
      {
        if ( params.size() != 2u )
        {
          if ( error )
          {
            *error = "Too many arguments for .init command";
          }
          return false;
        }

        p.add_initializer( params.at( 0u ), boost::lexical_cast<unsigned>( params.at( 1u ) ) );
      }

      else if (command == ".inputs")
      {
        for ( unsigned i = 0u; i < params.size(); ++i )
        {
          // input is quoted?
          if ( params.at( i ).at( 0u ) == '"' )
          {
            std::string temp = params.at( i );
            // search next input with quote and concatenate
            for ( unsigned j = i + 1u; j < params.size(); ++j )
            {
              temp += " " + params.at( j );
              if ( params.at( j ).at( params.at( j ).size() - 1u ) == '"' )
              {
                i = j;
                break;
              }
            }

            // erase quotes
            temp.erase( 0u, 1u );
            temp.erase( temp.length() - 1u, temp.length() );

            p.add_input( temp );
          }

          // input without quote
          else
          {
            p.add_input( params.at( i ) );
          }
        }
      }

      else if ( command == ".begin" )
      {
        while ( true )
        {
          /* Get next line */
          std::vector<std::string> tact;
          getline(patternfile, line);
          boost::algorithm::split(tact, line, boost::algorithm::is_any_of( " " ) );

          /* Abort? */
          if ( tact.at( 0u ) == ".end" )
          {
            break;
          }

          /* Correct number of pattern? */
          if ( tact.size() != p.inputs().size() )
          {
            if ( error )
            {
              *error = "Wrong number of simulation pattern";
            }
            return false;
          }

          /* Make array */
          using boost::adaptors::transformed;

          std::vector<unsigned> tmp( tact.size() );
          std::transform( tact.begin(), tact.end(), tmp.begin(), []( const std::string& s ) { return boost::lexical_cast<unsigned>( s ); } );

          /* Push array */
          p.add_pattern( tmp );
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
