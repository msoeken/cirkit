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

#include "aig.hpp"

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

// command::rules_t aig_command::validity_rules() const
// {
//   return {
//     {[&]() { return static_cast<unsigned>( opts.is_set( "mig" ) ) + static_cast<unsigned>( opts.is_set( "tt" ) ) == 1u; }, "either mig or tt needs to be chosen"},
//     {[&]() { return !opts.is_set( "mig" ) || !env->store<mig_graph>().empty(); }, "no MIG available"},
//     {[&]() { return !opts.is_set( "tt" ) || !env->store<tt>().empty(); }, "no truth table available"}
//   };
// }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
