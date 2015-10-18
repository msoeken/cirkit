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
 * @file strash.hpp
 *
 * @brief Strashes (re-builds) an AIG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef STRASH_HPP
#define STRASH_HPP

#include <map>
#include <string>

#include <boost/dynamic_bitset.hpp>

#include <core/properties.hpp>
#include <classical/aig.hpp>
#include <classical/functions/simulate_aig.hpp>

namespace cirkit
{

class strash_simulator : public aig_simulator<aig_function>
{
public:
  strash_simulator( aig_graph& aig_new, unsigned offset,
                    const std::map<unsigned, unsigned>& reorder,
                    const boost::dynamic_bitset<>& invert );

  aig_function get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  aig_function get_constant() const;
  aig_function invert( const aig_function& v ) const;
  aig_function and_op( const aig_node& node, const aig_function& v1, const aig_function& v2 ) const;

private:
  aig_graph& aig_new;
  const aig_graph_info& info;
  unsigned offset;
  const std::map<unsigned, unsigned>& reorder;
  const boost::dynamic_bitset<>& _invert;
};

aig_graph strash( const aig_graph& aig,
                  const properties::ptr& settings = properties::ptr(),
                  const properties::ptr& statistics = properties::ptr() );

void strash( const aig_graph& aig,
             aig_graph& dest,
             const properties::ptr& settings = properties::ptr(),
             const properties::ptr& statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
