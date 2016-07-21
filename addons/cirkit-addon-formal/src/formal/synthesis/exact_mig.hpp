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
 * @file exact_mig.hpp
 *
 * @brief Finds minimal MIG representations
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef EXACT_MIG_HPP
#define EXACT_MIG_HPP

#include <boost/optional.hpp>

#include <core/properties.hpp>
#include <classical/mig/mig.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

/**
 * @settings
 *
   |---------------------+-----------------------------------------------+------------------------|
   | Settings            | Description                                   | Default                |
   |---------------------+-----------------------------------------------+------------------------|
   | start               | Initial number of gates                       | 1u                     |
   | model_name          | Name of the MIG model                         | std::string( "exact" ) |
   | output_name         | Name of the output                            | std::string( "f" )     |
   | output_inverter     | Allow output inversion in encoding            | false                  |
   | incremental         | Explicit or incremental SAT solving           | false                  |
   | min_depth           | Smallest MIG with smallest depth              | false                  |
   | all_solutions       | Enumerate all solutions                       | false                  |
   | enc_with_bitvectors | Encode numbers as bit-vectors and not as ints | false                  |
   | verbose             | Be verbose                                    | false                  |
   |---------------------+-----------------------------------------------+------------------------|
 */
boost::optional<mig_graph> exact_mig_with_sat( const tt& spec,
                                               const properties::ptr& settings = properties::ptr(),
                                               const properties::ptr& statistics = properties::ptr() );

boost::optional<mig_graph> exact_mig_with_sat( const mig_graph& spec,
                                               const properties::ptr& settings = properties::ptr(),
                                               const properties::ptr& statistics = properties::ptr() );

boost::optional<xmg_graph> exact_xmg_with_sat( const tt& spec,
                                               const properties::ptr& settings = properties::ptr(),
                                               const properties::ptr& statistics = properties::ptr() );

boost::optional<xmg_graph> exact_xmg_with_sat( const mig_graph& spec,
                                               const properties::ptr& settings = properties::ptr(),
                                               const properties::ptr& statistics = properties::ptr() );

boost::optional<mig_graph> exact_mig_with_bdds( const tt& spec,
                                                const properties::ptr& settings = properties::ptr(),
                                                const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
