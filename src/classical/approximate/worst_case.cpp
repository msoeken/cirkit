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

#include "worst_case.hpp"

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>
#include <classical/utils/aig_utils.hpp>

#include <aig/gia/gia.h>
#include <sat/bsat/satSolver.h>

#define LN( x ) if ( verbose ) { std::cout << x << std::endl; }

namespace abc
{
void Wlc_BlastSubtract( Gia_Man_t * pNew, int * pAdd0, int * pAdd1, int nBits ); // result is in pAdd0
int Wlc_BlastLessSigned( Gia_Man_t * pNew, int * pArg0, int * pArg1, int nBits );
}

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

// po1 and po2 are vectors of literals of the original outputs
abc::Gia_Man_t * Gia_ManDupAppendNewWithoutPOs( abc::Gia_Man_t * pOne, abc::Gia_Man_t * pTwo, std::vector<int>& po1, std::vector<int>& po2 )
{
  abc::Gia_Obj_t * pObj;
  int i;
  auto pNew = abc::Gia_ManStart( abc::Gia_ManObjNum( pOne ) + abc::Gia_ManObjNum( pTwo ) );
  pNew->pName = abc::Abc_UtilStrsav( pOne->pName );
  pNew->pSpec = abc::Abc_UtilStrsav( pOne->pSpec );
  abc::Gia_ManHashAlloc( pNew );
  abc::Gia_ManConst0( pOne )->Value = 0;
  Gia_ManForEachObj1( pOne, pObj, i )
  {
    if ( abc::Gia_ObjIsAnd( pObj ) )
    {
      pObj->Value = abc::Gia_ManHashAnd( pNew, abc::Gia_ObjFanin0Copy( pObj ), abc::Gia_ObjFanin1Copy( pObj ) );
    }
    else if ( abc::Gia_ObjIsPi( pOne, pObj ) )
    {
      pObj->Value = abc::Gia_ManAppendCi( pNew );
    }
  }
  abc::Gia_ManConst0( pTwo )->Value = 0;
  Gia_ManForEachObj1( pTwo, pObj, i )
  {
    if ( abc::Gia_ObjIsAnd( pObj ) )
    {
      pObj->Value = abc::Gia_ManHashAnd( pNew, abc::Gia_ObjFanin0Copy( pObj ), abc::Gia_ObjFanin1Copy( pObj ) );
    }
    else if ( abc::Gia_ObjIsPi( pTwo, pObj ) )
    {
      pObj->Value = abc::Gia_ManPi( pOne, abc::Gia_ObjCioId( pObj ) )->Value;
    }
  }

  po1.clear();
  po2.clear();

  Gia_ManForEachPo( pOne, pObj, i )
  {
    po1.push_back( Gia_ObjFanin0Copy( pObj ) );
  }
  Gia_ManForEachPo( pTwo, pObj, i )
  {
    po2.push_back( Gia_ObjFanin0Copy( pObj ) );
  }
  abc::Gia_ManSetRegNum( pNew, 0 );

  return pNew;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

boost::multiprecision::uint256_t worst_case( const aig_graph& f, const aig_graph& fhat,
                                             const properties::ptr& settings,
                                             const properties::ptr& statistics )
{
  properties_timer t( statistics );

  const auto& finfo = aig_info( f );
  const auto& fhatinfo = aig_info( fhat );

  if ( ( finfo.inputs.size() != fhatinfo.inputs.size() ) || ( finfo.outputs.size() != fhatinfo.outputs.size() ) )
  {
    set_error_message( statistics, "circuits have incompatible sizes" );
    return 0;
  }

  const auto num_bits = finfo.outputs.size();

  const auto giaf = cirkit_to_gia( f );
  const auto giafhat = cirkit_to_gia( fhat );

  /* append giafhat to giaf and share PIs */
  std::vector<int> po1, po2;
  auto miter = Gia_ManDupAppendNewWithoutPOs( giaf, giafhat, po1, po2 );
  assert( abc::Gia_ManCiNum( miter ) == static_cast<int>( finfo.inputs.size() ) );
  assert( abc::Gia_ManCoNum( miter ) == 0 );
  assert( po1.size() == num_bits );
  assert( po2.size() == num_bits );

  /* create comparator */
  int * a = &po1[0];
  int * b = &po2[0];

  const auto less = abc::Wlc_BlastLessSigned( miter, a, b, num_bits );

  std::vector<int> m1( num_bits ), m2( num_bits );
  for ( auto i = 0u; i < num_bits; ++i )
  {
    m1[i] = abc::Gia_ManHashMux( miter, less, po2[i], po1[i] ); /* returns the larger number */
    m2[i] = abc::Gia_ManHashMux( miter, less, po1[i], po2[i] ); /* returns the smaller number */
  }

  /* create subtractor */
  abc::Wlc_BlastSubtract( miter, &m1[0], &m2[0], num_bits ); /* stores result in m1 */

  for ( auto l : m1 )
  {
    abc::Gia_ManAppendCo( miter, l );
  }

  abc::Gia_ManHashStop( miter );

  /* create CNF */
  auto * cnf = static_cast<abc::Cnf_Dat_t*>( abc::Mf_ManGenerateCnf( miter, 8, 0, 0, 0, 0 ) );
  auto * solver = (abc::sat_solver *)abc::Cnf_DataWriteIntoSolver( cnf, 1, 0 );
  abc::Cnf_DataFree( cnf );

  /* output literals are the first ones */
  std::vector<int> lits;
  for ( auto i = 1u; i <= num_bits; ++i )
  {
    lits.push_back( abc::Abc_Var2Lit( i, 0 ) );
  }

  /* call LEXSAT on output values */
  boost::multiprecision::uint256_t result;
  if ( abc::sat_solver_solve_lexsat( solver, &lits[0], lits.size() ) == abc::l_True )
  {
    boost::dynamic_bitset<> sol( num_bits );
    for ( auto i = 0u; i < num_bits; ++i )
    {
      sol[i] = abc::sat_solver_var_value( solver, i + 1 );
    }

    result = to_multiprecision<boost::multiprecision::uint256_t>( sol );
  }
  else
  {
    assert( false );
  }

  abc::sat_solver_delete( solver );

  /* clean up */
  abc::Gia_ManStop( miter );
  abc::Gia_ManStop( giaf );
  abc::Gia_ManStop( giafhat );

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
