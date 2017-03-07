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
 * @file cnf_manager.hpp
 *
 * @brief CNF manager
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CNF_MANAGER_HPP
#define CNF_MANAGER_HPP

#include <iostream>
#include <unordered_map>
#include <vector>

#include <boost/range/iterator_range.hpp>

#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

class cnf_manager
{
public:
  using vertex_range_t = boost::iterator_range<std::vector<int>::const_iterator>;

public:
  vertex_range_t compute( const tt& func, unsigned* literal_count = nullptr );

  void print_statistics( std::ostream& os = std::cout ) const;

private:
  /* maps str repr of tt to tuple of start and (exclusive) end index in `covers', and the literal count */
  using hash_t = std::unordered_map<std::string, std::tuple<unsigned, unsigned, unsigned>>;

  std::vector<int> covers;
  hash_t           hash;

  /* statistics */
  double        runtime    = 0.0;
  unsigned long cache_hit  = 0;
  unsigned long cache_miss = 0;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
