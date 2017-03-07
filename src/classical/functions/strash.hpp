/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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
