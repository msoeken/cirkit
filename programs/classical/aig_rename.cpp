/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

/**
 * @author Mathias Soeken
 */

#include <map>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <core/properties.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>

#include <classical/aig.hpp>
#include <classical/functions/aig_rename.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/write_aiger.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string              filename, aagname;
  std::vector<std::string> inputs, outputs;

  program_options opts;
  opts.add_options()
    ( "filename",  value( &filename ),             "Input circuit" )
    ( "input,i",   value( &inputs )->composing(),  "Rename inputs, oldname=newname" )
    ( "output,o",  value( &outputs )->composing(), "Rename outputs, oldname=newname" )
    ( "aagname",   value( &aagname ),              "Output circuit" )
    ( "verbose,v",                                 "Be verbose" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* Create AIGs */
  aig_graph aig;
  read_aiger( aig, filename );

  std::map<std::string, std::string> imap, omap;
  for ( const auto& input : inputs )
  {
    imap.insert( split_string_pair( input, "=" ) );
  }
  for ( const auto& output : outputs )
  {
    omap.insert( split_string_pair( output, "=" ) );
  }

  aig_rename( aig, imap, omap );

  if ( !aagname.empty() )
  {
    write_aiger( aig, aagname );
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
