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

/**
 * @file add_aig.hpp
 *
 * @brief Adds clauses based from AIG with ABC
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef ADD_AIG_WITH_GIA_HPP
#define ADD_AIG_WITH_GIA_HPP

#include <boost/graph/depth_first_search.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/range_utils.hpp>

#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>

#include <classical/aig.hpp>
#include <classical/utils/aig_dfs.hpp>
#include <classical/utils/aig_utils.hpp>

#include <classical/sat/sat_solver.hpp>
#include <classical/sat/operations/logic.hpp>

namespace cirkit
{

inline int cnf_lit_to_var( int Lit ) { return (Lit & 1)? -(Lit >> 1)-1 : (Lit >> 1)+1;  }
inline int cnf_lit_to_var_offset( int Lit, unsigned offset ) { return (Lit & 1) ? -(Lit >> 1) - 1 - offset : (Lit >> 1) + 1 + offset; }

template<class S>
int add_aig_with_gia( S& solver, const aig_graph& aig, int sid, std::vector<int>& piids, std::vector<int>& poids,
                      properties::ptr settings = properties::ptr(),
                      properties::ptr statistics = properties::ptr() )
{
  /* settings */
  // const auto use_node_ids = get( settings, "use_node_ids", true );

  /* write to CNF */
  auto gia = cirkit_to_gia( aig );
  const auto cnf = static_cast<abc::Cnf_Dat_t*>( Mf_ManGenerateCnf( gia, 8, 0, 0, 0, 0 ) );

  const int offset = sid - 1;
  // std::cout << "[i] offset: " << offset << std::endl;

  /* quick hack to ensure variable size */
  add_clause( solver )( {cnf->nVars + offset, -( cnf->nVars + offset )} );


  for ( auto i = 0; i < cnf->nClauses; ++i )
  {
    using namespace std::placeholders;

    std::vector<int> clause( std::distance( cnf->pClauses[i], cnf->pClauses[i + 1] ) );
    std::transform( cnf->pClauses[i], cnf->pClauses[i + 1], clause.begin(), std::bind( &cnf_lit_to_var_offset, std::placeholders::_1, offset ) );
    add_clause( solver )( clause );
  }

  const auto new_sid = sid + cnf->nVars;

  /* fill pis and pos */
  const auto& info = aig_info( aig );
  piids.resize( info.inputs.size() );
  poids.resize( info.outputs.size() );

  // if ( use_node_ids )
  // {
  //   int id, i;
  //   Gia_ManForEachCoId( gia, id, i )
  //   {
  //     poids[i] = id + offset + 1u;
  //   }
  //   Gia_ManForEachCiId( gia, id, i )
  //   {
  //     piids[i] = id + offset + 1u;
  //   }
  // }
  // else
  // {
  boost::iota( poids, sid + 1u );
  boost::iota( piids, new_sid - info.inputs.size() );
  // }

  Gia_ManStop( gia );

  return new_sid;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
