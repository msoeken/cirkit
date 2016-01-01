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
 * @file aig_to_cirkit_bdd.hpp
 *
 * @brief Convert AIG to CirKit BDD
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_TO_CIRKIT_BDD_HPP
#define AIG_TO_CIRKIT_BDD_HPP

#include <vector>

#include <classical/dd/bdd.hpp>
#include <classical/functions/simulate_aig.hpp>

namespace cirkit
{

class cirkit_bdd_simulator : public aig_simulator<bdd>
{
public:
  cirkit_bdd_simulator( const aig_graph& aig, unsigned log_max_objs = 20u );
  cirkit_bdd_simulator( const bdd_manager_ptr& mgr );

  bdd get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bdd get_constant() const;
  bdd invert( const bdd& v ) const;
  bdd and_op( const aig_node& node, const bdd& v1, const bdd& v2 ) const;

public:
  bdd_manager_ptr mgr;
};

std::vector<bdd> aig_to_bdd( const aig_graph& aig, const bdd_manager_ptr& mgr );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
