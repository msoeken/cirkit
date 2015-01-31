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

#include "is_identity.hpp"

#include <boost/range/combine.hpp>
#include <boost/range/numeric.hpp>

#include <reversible/rcbdd.hpp>

namespace cirkit
{

bool is_identity( const circuit& circ )
{
  rcbdd mgr;
  mgr.initialize_manager();
  mgr.create_variables( circ.lines() );

  BDD f = mgr.create_from_circuit( circ );

  BDD identity = boost::accumulate( boost::combine( mgr.xs(), mgr.ys() ),
                                    mgr.manager().bddOne(),
                                    []( BDD current, const boost::tuple<BDD, BDD>& t ) {
                                      return current & ( boost::get<0>( t ).Xnor( boost::get<1>( t ) ) );
                                    } );

  return f == identity;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
