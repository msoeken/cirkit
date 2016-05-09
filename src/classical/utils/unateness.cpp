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

#include "unateness.hpp"

#include <classical/utils/aig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

unate_kind get_unateness_kind( const boost::dynamic_bitset<>& u, unsigned po, unsigned pi, unsigned num_pis )
{
  const auto pos = po * ( num_pis << 1u ) + ( pi << 1u );

  return u[pos] ? ( u[pos + 1u] ? unate_kind::independent : unate_kind::unate_neg ) : ( u[pos + 1u] ? unate_kind::unate_pos : unate_kind::binate );
}

unate_kind get_unateness_kind( const boost::dynamic_bitset<>& u, unsigned po, unsigned pi, const aig_graph_info& info )
{
  return get_unateness_kind( u, po, pi, info.inputs.size() );
}

unate_kind get_unateness_kind( const boost::dynamic_bitset<>& u, unsigned po, unsigned pi, const aig_graph& aig )
{
  return get_unateness_kind( u, po, pi, aig_info( aig ) );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
