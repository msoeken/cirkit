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

#include "gia_to_cirkit.hpp"

#include <boost/format.hpp>

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
    const auto name = gia->vNamesIn && i < abc::Vec_PtrSize( gia->vNamesIn ) ? std::string( (char*)abc::Vec_PtrGetEntry( gia->vNamesIn, i ) ) : boost::str( boost::format( "input_%d" ) % i );
    aig_function pi = aig_create_pi( aig, name );
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
    const auto name = gia->vNamesOut && i < abc::Vec_PtrSize( gia->vNamesOut ) ? std::string( (char*)abc::Vec_PtrGetEntry( gia->vNamesOut, i ) ) : boost::str( boost::format( "output_%d" ) % i );
    const auto& n = nodes.at( abc::Gia_ObjId(const_cast< abc::Gia_Man_t* >( gia ), obj) - abc::Gia_ObjDiff0(obj) );
    aig_create_po( aig, { n, abc::Gia_ObjFaninC0(obj)!=0 }, name );
  }

  return aig;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
