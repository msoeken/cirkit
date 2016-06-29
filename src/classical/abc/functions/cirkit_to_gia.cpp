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

#include "cirkit_to_gia.hpp"

#include <cstring>

#include <boost/graph/topological_sort.hpp>

#include <core/utils/range_utils.hpp>
#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

inline int Gia_ManAppendAnd2_Simplified( abc::Gia_Man_t * p, int iLit0, int iLit1 )
{
  if ( iLit1 < iLit0 ) return Gia_ManAppendAnd2_Simplified( p, iLit1, iLit0 );
  if ( iLit0 == 0 ) return 0;
  if ( iLit1 == 1 ) return iLit0;
  if ( iLit0 == iLit1 ) return iLit1;
  if ( (iLit0 >> 1) == (iLit1 >> 1) )
    return 0;
  return Gia_ManAppendAnd( p, iLit0, iLit1 );
}

abc::Gia_Man_t* cirkit_to_gia( const aig_graph& aig )
{
  const auto& info = aig_info( aig );

  const unsigned _num_inputs = info.inputs.size();
  const unsigned _num_outputs = info.outputs.size();
  const unsigned _num_latches = info.cis.size();
  const unsigned _num_vertices = num_vertices( aig ) - 1u;
  const unsigned _num_gates = _num_vertices - _num_latches - _num_inputs;

  assert( _num_vertices == _num_inputs + _num_latches + _num_gates );

  /* allocate an empty aig in abc */
  abc::Gia_Man_t * gia = abc::Gia_ManStart( _num_vertices + _num_latches + _num_outputs + 1u );
  gia->nConstrs = 0;
  gia->pName = strcpy( (char*)malloc( sizeof( char ) * ( info.model_name.size() + 1u ) ), info.model_name.c_str() );

  std::vector< int > node_to_obj( boost::num_vertices( aig ) );
  node_to_obj[0] = 0;

  /* inputs */
  assert( !gia->vNamesIn );
  gia->vNamesIn = abc::Vec_PtrStart( info.inputs.size() );
  for ( const auto& input : index( info.inputs ) )
  {
    const int obj = abc::Gia_ManAppendCi( gia );
    node_to_obj[input.value] = abc::Abc_Lit2Var( obj );
    const auto name = ( info.node_names.size() >= input.value ) ? info.node_names.at( input.value ) : ( boost::format("input_%d") % input.value ).str();
    abc::Vec_PtrSetEntry( gia->vNamesIn, input.index, strcpy( (char*)malloc( sizeof( char ) * ( name.size() + 1u ) ), name.c_str() ) );
  }

  /* latches */
  assert( info.cis.size() == info.cos.size() );
  assert( _num_latches == 0u );

  /* and gates */
  std::vector<unsigned> topsort( boost::num_vertices( aig ) );
  boost::topological_sort( aig, topsort.begin() );

  for ( const auto& node : topsort )
  {
    if ( !boost::out_degree( node, aig ) ) { continue; }

    const auto children = get_children( aig, node );
    assert( children.size() == 2u );

    const int obj = Gia_ManAppendAnd2_Simplified( gia,
                                                  abc::Abc_Var2Lit( node_to_obj[children[0].node], children[0].complemented ),
                                                  abc::Abc_Var2Lit( node_to_obj[children[1].node], children[1].complemented ) );

    node_to_obj[node] = abc::Abc_Lit2Var( obj );
  }

  /* outputs */
  assert( !gia->vNamesOut );
  gia->vNamesOut = abc::Vec_PtrStart( info.outputs.size() );
  for ( const auto& output : index( info.outputs ) )
  {
    abc::Gia_ManAppendCo( gia, abc::Abc_Var2Lit( node_to_obj[output.value.first.node], output.value.first.complemented ) );
    const auto name = output.value.second;
    abc::Vec_PtrSetEntry( gia->vNamesOut, output.index, strcpy( (char*)malloc( sizeof( char ) * ( name.size() + 1u ) ), name.c_str() ) );
  }

  return gia;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
