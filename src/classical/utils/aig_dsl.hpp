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
 * @file aig_dsl.hpp
 *
 * @brief An embedded DSL to create AIGs
 *
 * @author Mathias Soeken
 * @since  2.2
 */

#ifndef AIG_DSL_HPP
#define AIG_DSL_HPP

#include <map>

#include <classical/aig.hpp>

namespace cirkit
{

class aig_dsl
{
public:
  aig_dsl();

  using node = std::pair<aig_graph*, aig_function>;

  inline aig_graph& aig() { return g; }
  inline const aig_graph& aig() const { return g; }

  node operator[]( const std::string& name );

private:
  aig_graph g;
  const aig_graph_info& aig_info = boost::get_property( g, boost::graph_name );
  std::map<std::string, node> pis;
};

const aig_function& operator*( const aig_dsl::node node );

aig_dsl::node operator!( aig_dsl::node node );
aig_dsl::node operator&&( aig_dsl::node nodel, aig_dsl::node noder );
aig_dsl::node operator||( aig_dsl::node nodel, aig_dsl::node noder );
aig_dsl::node operator^( aig_dsl::node nodel, aig_dsl::node noder );

inline aig_dsl::node operator-( aig_dsl::node node ) { return node; }
void operator<( const std::string& name, aig_dsl::node node );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
