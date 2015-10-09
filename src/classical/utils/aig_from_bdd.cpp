/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

  if Cudd_IsConstant( r )
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

  for ( auto i = num_pis; i < n; ++i )
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
