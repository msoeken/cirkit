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
 * @file aig_from_bdd.hpp
 *
 * @brief Create AIGs from BDDs
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#include "aig_from_bdd.hpp"

#include <boost/format.hpp>

#include <cuddInt.h>

#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

aig_function aig_from_bdd_rec( aig_graph& aig, DdManager* dd, DdNode* node )
{
  const auto& info = aig_info( aig );

  auto   is_complement = Cudd_IsComplement( node );
  auto * r = Cudd_Regular( node );

  aig_function f;

  if ( Cudd_IsConstant( r ) )
  {
    f = aig_get_constant( aig, r == DD_ONE( dd ) );
  }
  else
  {
    auto index = Cudd_NodeReadIndex( r );

    auto f_true  = aig_from_bdd_rec( aig, dd, cuddT( r ) );
    auto f_false = aig_from_bdd_rec( aig, dd, cuddE( r ) );

    f = aig_create_ite( aig, {info.inputs[index], false}, f_true, f_false );
  }

  return is_complement ? !f : f;
}

aig_function aig_from_bdd( aig_graph& aig, DdManager* dd, DdNode* node )
{
  const auto& info = aig_info( aig );

  auto n = Cudd_ReadSize( dd );
  auto num_pis = info.inputs.size();

  for ( auto i = num_pis; static_cast<int>( i ) < n; ++i )
  {
    aig_create_pi( aig, boost::str( boost::format( "x%d" ) % i ) );
  }

  return aig_from_bdd_rec( aig, dd, node );
}

aig_function aig_from_bdd( aig_graph& aig, const BDD& node )
{
  return aig_from_bdd( aig, node.manager(), node.getNode() );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
