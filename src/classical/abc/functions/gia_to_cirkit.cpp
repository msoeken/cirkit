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

#include "gia_to_cirkit.hpp"

#include <classical/utils/aig_utils.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/write_aiger.hpp>

namespace cirkit
{

aig_graph gia_to_cirkit( const abc::Gia_Man_t* gia )
{
  aig_graph aig;
  aig_initialize( aig );

  auto& info = aig_info( aig );

  if ( gia->pName )
  {
    info.model_name = std::string( gia->pName );
  }

  std::map<int, aig_node> nodes = { {0, info.constant } };

  abc::Gia_Obj_t* obj; int i;
  Gia_ManForEachCi( const_cast< abc::Gia_Man_t* >( gia ), obj, i )
  {
    // std::cout << "[ci] " << Gia_ObjId( const_cast< Gia_Man_t* >( gia ), obj )  << '\n';
    aig_function pi = aig_create_pi( aig, "" );
    nodes.insert( { abc::Gia_ObjId( const_cast< abc::Gia_Man_t* >( gia ), obj ), pi.node } );
  }

  Gia_ManForEachAnd( const_cast< abc::Gia_Man_t* >( gia ), obj, i )
  {
    // std::cout << "[and] " << ( Gia_ObjId(const_cast< Gia_Man_t* >( gia ), obj) ) << '\n';
    const auto l_id = abc::Gia_ObjId(const_cast<abc::Gia_Man_t*>( gia ), abc::Gia_ObjFanin0( obj ) );
    const auto r_id = abc::Gia_ObjId(const_cast<abc::Gia_Man_t*>( gia ), abc::Gia_ObjFanin1( obj ) );
    const auto l_it = nodes.find( l_id );
    const auto r_it = nodes.find( r_id );

    if ( l_it == nodes.end() )
    {
      std::cout << "[e] no node with id " << l_id << std::endl;
      assert( false );
    }
    if ( r_it == nodes.end() )
    {
      std::cout << "[e] no node with id " << r_id << std::endl;
      assert( false );
    }

    const auto& l = l_it->second;
    const auto& r = r_it->second;
    aig_function g = aig_create_and( aig, {l, abc::Gia_ObjFaninC0(obj) != 0}, {r, abc::Gia_ObjFaninC1(obj) != 0} );
    nodes.insert( { abc::Gia_ObjId(const_cast< abc::Gia_Man_t* >( gia ), obj), g.node } );
  }

  Gia_ManForEachCo( const_cast< abc::Gia_Man_t* >( gia ), obj, i )
  {
    // std::cout << "[co] " << ( Gia_ObjId(const_cast< Gia_Man_t* >( gia ), obj) - Gia_ObjDiff0(obj) )  << '\n';
    const auto& n = nodes.at( abc::Gia_ObjId(const_cast< abc::Gia_Man_t* >( gia ), obj) - abc::Gia_ObjDiff0(obj) );
    aig_create_po( aig, { n, abc::Gia_ObjFaninC0(obj)!=0 }, "" );
  }

  return aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
