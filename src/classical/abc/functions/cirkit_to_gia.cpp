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

  /* map aig_nodes to literals (in gia graph) */
  std::vector< int > node_to_lit( boost::num_vertices( aig ) );
  node_to_lit[0] = 0;

  /* inputs */
  assert( !gia->vNamesIn );
  gia->vNamesIn = abc::Vec_PtrStart( info.inputs.size() );
  for ( const auto& input : index( info.inputs ) )
  {
    const int obj = abc::Gia_ManAppendCi( gia );
    node_to_lit[input.value] = obj;
    auto name = ( info.node_names.size() >= input.value ) ? info.node_names.at( input.value ) : ( boost::format("input_%d") % input.value ).str();
    if ( name.empty() ) { name = ( boost::format( "input_%d" ) % input.value ).str(); }
    abc::Vec_PtrSetEntry( gia->vNamesIn, input.index, strcpy( (char*)malloc( sizeof( char ) * ( name.size() + 1u ) ), name.c_str() ) );
  }

  /* latches */
  assert( info.cis.size() == info.cos.size() );
  assert( _num_latches == 0u );

  /* and gates */
  std::vector< unsigned > topsort( boost::num_vertices( aig ) );
  boost::topological_sort( aig, topsort.begin() );

  for ( const auto& node : topsort )
  {
    if ( !boost::out_degree( node, aig ) ) { continue; }

    const auto children = get_children( aig, node );
    assert( children.size() == 2u );

    const int child0 = children[0].complemented ? abc::Abc_LitNot(node_to_lit[children[0].node]) : node_to_lit[children[0].node];
    const int child1 = children[1].complemented ? abc::Abc_LitNot(node_to_lit[children[1].node]) : node_to_lit[children[1].node];
    const int obj = Gia_ManAppendAnd2_Simplified( gia, child0, child1 );
    node_to_lit[node] = obj;
  }

  /* outputs */
  assert( !gia->vNamesOut );
  gia->vNamesOut = abc::Vec_PtrStart( info.outputs.size() );
  for ( const auto& output : index( info.outputs ) )
  {
    const int arg = output.value.first.complemented ? abc::Abc_LitNot( node_to_lit[output.value.first.node] ) : node_to_lit[output.value.first.node];
    abc::Gia_ManAppendCo( gia, arg );
    auto name = output.value.second;
    if ( name.empty() ) { name = ( boost::format( "output_%d" ) % output.index ).str(); }
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
