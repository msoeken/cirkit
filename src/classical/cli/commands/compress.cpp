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

#include "compress.hpp"

#include <cmath>
#include <iostream>

#include <boost/program_options.hpp>

#include <core/cli/stores.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/conversion_utils.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>

#include <cuddObj.hh>

using boost::program_options::value;

namespace cirkit
{

/******************************************************************************
 * compressor                                                                 *
 ******************************************************************************/

std::string convert_dec2bin( unsigned number, unsigned numvars )
{
  std::stringstream s;
  s << std::hex << 42;

  const auto ndigits = (unsigned)ceil( numvars / 4.0 );
  return convert_hex2bin( std::string( std::max<unsigned>( 0u, ndigits - s.str().size() ), '0' ) + s.str() );
}

class bdd_compressor
{
public:
  bdd_compressor()
    : func( mgr.bddZero() )
  {
  }

  void add( const std::string& binnumber )
  {
    func |= make_cube( mgr, binnumber );
  }

  void info()
  {
    std::cout << func.CountMinterm( 32 ) << std::endl;
  }

  bdd_function_t get() const
  {
    return {mgr, {func}};
  }

private:
  Cudd mgr;
  BDD func;
};

compress_command::compress_command( const environment::ptr& env )
  : cirkit_command( env, "Compress files with DDs" )
{
  opts.add_options()
    ( "filename",   value( &filename ),          "file with one number per line" )
    ( "base,b",     value_with_default( &base ), "number base (2, 10, or 16)" )
    ( "numvars,n",  value( &numvars ),           "number of variables (can be implied for base 2 and 16)" )
    ;
  add_positional_option( "filename" );
  add_new_option();
}

command::rules_t compress_command::validity_rules() const
{
  return {
    {[this]() { return base == 2 || base == 10 || base == 16; }, "base must be 2, 10, or 16"},
    {[this]() { return base != 10 || is_set( "numvars" ); }, "number of variables cannot be implied from base 10"}
  };
}

bool compress_command::execute()
{
  bdd_compressor cmp;

  foreach_line_in_file( filename, [this, &cmp]( const std::string& line ) {
      switch ( base )
      {
      case 2:
        cmp.add( line );
        break;
      case 10:
        cmp.add( convert_dec2bin( boost::lexical_cast<unsigned>( line ), numvars ) );
        break;
      case 16:
        cmp.add( convert_hex2bin( line ) );
        break;
      }

      return true;
    });

  cmp.info();

  auto& bdds = env->store<bdd_function_t>();
  extend_if_new( bdds );
  bdds.current() = cmp.get();

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
