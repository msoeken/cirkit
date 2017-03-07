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
