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

#include "tpar.hpp"

#include <alice/rules.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/io/read_qc.hpp>
#include <reversible/io/write_qc.hpp>

namespace cirkit
{

tpar_command::tpar_command( const environment::ptr& env )
  : cirkit_command( env, "Runs Matthew Amy's T-par algorithm", "Polynomial-time T-depth Optimization of Clifford+T circuits via Matroid Partitioning (arXiv:1303.2042)\ninstall with ./utils/tools.py install tpar" )
{
  add_new_option();
}

command::rules_t tpar_command::validity_rules() const
{
  return {has_store_element<circuit>( env )};
}

bool tpar_command::execute()
{
  auto& circuits = env->store<circuit>();

  temporary_filename filename_in( "/tmp/tpar-%d-in.qc" );
  temporary_filename filename_out( "/tmp/tpar-%d-out.qc" );

  write_qc( circuits.current(), filename_in.name(), true );
  execute_and_omit( boost::str( boost::format( "t-par < %s > %s" ) % filename_in.name() % filename_out.name() ) );
  const auto circ = read_qc( filename_out.name() );

  extend_if_new( circuits );
  circuits.current() = circ;

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
