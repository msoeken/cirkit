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

#include "aig_dsl.hpp"

namespace cirkit
{

aig_dsl::aig_dsl()
{
  aig_initialize( g );
}

aig_dsl::node aig_dsl::operator[]( const std::string& name )
{
  if ( pis.find( name ) == pis.end() )
  {
    pis.insert( {name, {&g, aig_create_pi( g, name )}} );
  }
  return pis[name];
}

const aig_function& operator*( const aig_dsl::node& node )
{
  return node.second;
}

aig_dsl::node operator!( aig_dsl::node& node )
{
  return {node.first, !node.second};
}

aig_dsl::node operator&&( aig_dsl::node nodel, aig_dsl::node noder )
{
  return {nodel.first, aig_create_and( *nodel.first, nodel.second, noder.second )};
}

aig_dsl::node operator||( aig_dsl::node nodel, aig_dsl::node noder )
{
  return {nodel.first, aig_create_or( *nodel.first, nodel.second, noder.second )};
}

aig_dsl::node operator^( aig_dsl::node nodel, aig_dsl::node noder )
{
  return {nodel.first, aig_create_xor( *nodel.first, nodel.second, noder.second )};
}

void operator<( const std::string& name, aig_dsl::node node )
{
  aig_create_po( *node.first, node.second, name );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
