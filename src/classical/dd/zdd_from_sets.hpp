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
 * @file zdd_from_sets.hpp
 *
 * @brief Get ZDD from set of sets
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef ZDD_FROM_SETS_HPP
#define ZDD_FROM_SETS_HPP

#include <set>

#include <classical/dd/zdd.hpp>

namespace cirkit
{

template<typename T>
using set_family = std::set<std::set<T>>;

template<typename T>
zdd zdd_from_sets( zdd_manager& mgr, const set_family<T>& set )
{
  zdd p = mgr.zdd_bot();
  for ( const auto& u : set )
  {
    zdd t = mgr.zdd_top();
    for ( auto i : u )
    {
      t = t + mgr.zdd_var(i);
    }
    p = p || t;
  }

  return p;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
