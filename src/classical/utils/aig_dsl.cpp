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
