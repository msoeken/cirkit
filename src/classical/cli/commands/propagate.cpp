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

#include "propagate.hpp"

#include <map>

#include <boost/format.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>
#include <classical/functions/aig_constant_propagation.hpp>

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

propagate_command::propagate_command( const environment::ptr& env )
  : aig_base_command( env, "Propagates constant inputs in an AIG" )
{
  opts.add_options()
    ( "assignments,a", value( &assignments ), "Propagate assignment (e.g. \"!a,b,c\")" )
    ;
  be_verbose();
}

bool propagate_command::execute()
{
  auto settings = make_settings();
  auto statistics = std::make_shared<properties>();
  std::map<std::string, bool> propagation_values;
  foreach_string( assignments, ",", [&]( const std::string& s ) {
      auto name = ( s[0] == '!' ) ? s.substr( 1u ) : s;
      auto value = ( s[0] != '!' );
      if ( is_verbose() )
      {
        std::cout << "[i] assign " << name << " <- " << value << std::endl;
      }
      propagation_values.insert( {name, value} );
    });
  aig() = aig_constant_propagation( aig(), propagation_values, settings, statistics );
  print_runtime( statistics->get<double>( "runtime" ) );
  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
