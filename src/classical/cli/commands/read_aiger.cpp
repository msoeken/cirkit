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

#include "read_aiger.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <lscli/rules.hpp>

#include <core/utils/program_options.hpp>
#include <classical/cli/stores.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/read_symmetries.hpp>
#include <classical/io/read_unateness.hpp>

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

read_aiger_command::read_aiger_command( const environment::ptr& env )
  : cirkit_command( env, "Reads an AIG from file (in AIGER format)" )
{
  add_positional_option( "filename" );
  opts.add_options()
    ( "filename", value( &filename ), "AIGER filename" )
    ( "nostrash",                     "Do not strash the AIG when reading (only for binary AIGs)" )
    ;
  add_new_option();
  be_verbose();
}

command::rules_t read_aiger_command::validity_rules() const
{
  return { file_exists(filename, "filename") };
}

bool read_aiger_command::execute()
{
  auto& aigs = env->store<aig_graph>();
  if ( is_verbose() )
  {
    std::cout << "[i] read from " << filename << std::endl;
  }
  if ( aigs.empty() || is_set( "new" ))
  {
    aigs.extend();
  }
  else
  {
    aigs.current() = aig_graph();
  }

  try
  {
    if ( boost::ends_with( filename, "aag" ) )
    {
      read_aiger( aigs.current(), filename );
    }
    else
    {
      read_aiger_binary( aigs.current(), filename, is_set( "nostrash" ) );
    }
  }
  catch ( const char *e )
  {
    std::cerr << e << std::endl;
    return false;
  }

  /* auto-find symmetry file */
  const auto symname = filename.substr( 0, filename.size() - 3 ) + "sym";
  if ( boost::filesystem::exists( symname ) )
  {
    /* read symmetries */
    std::cout << "[i] found and read symmetries file" << std::endl;
    read_symmetries( aigs.current(), symname );
  }

  /* auto-find unateness file */
  const auto depname = filename.substr( 0, filename.size() - 3 ) + "dep";
  if ( boost::filesystem::exists( depname ) )
  {
    /* read unateness */
    std::cout << "[i] found and read unateness dependency file" << std::endl;
    read_unateness( aigs.current(), depname );
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
