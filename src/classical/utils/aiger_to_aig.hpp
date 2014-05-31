/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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
 * @file aiger_to_aig.hpp
 *
 * @brief Creates aig_graph from aiger struct
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIGER_TO_AIG_HPP
#define AIGER_TO_AIG_HPP

#include <classical/aig.hpp>

#include <boost/graph/adjacency_list.hpp>

extern "C" {
#include <aiger.h>
}

namespace revkit
{

  struct aiger_to_aig_settings
  {
    std::string dotname;
  };

  void aiger_to_aig( const aiger * aig, aig_graph& graph, const aiger_to_aig_settings& settings = aiger_to_aig_settings() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
