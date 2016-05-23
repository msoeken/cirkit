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
