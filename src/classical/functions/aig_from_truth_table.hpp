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

/**
 * @file aig_from_truth_table.hpp
 *
 * @brief Create AIG from truth table
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_FROM_TRUTH_TABLE_HPP
#define AIG_FROM_TRUTH_TABLE_HPP

#include <classical/aig.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

aig_graph aig_from_truth_table( const tt& t );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
