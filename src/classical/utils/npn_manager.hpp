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
 * @file npn_manager.hpp
 *
 * @brief NPN manager
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef NPN_MANAGER_HPP
#define NPN_MANAGER_HPP

#include <functional>
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include <classical/functions/npn_canonization.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

inline std::function<tt(const tt&, boost::dynamic_bitset<>&, std::vector<unsigned>&)> make_exact_npn_canonization_wrapper()
{
  return std::function<tt(const tt&, boost::dynamic_bitset<>&, std::vector<unsigned>&)>( []( const tt& t, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm ) {
      return exact_npn_canonization( t, phase, perm );
    } );
}

class npn_manager
{
public:
  using npn_classifier_t = std::function<tt(const tt&, boost::dynamic_bitset<>&, std::vector<unsigned>&)>;

  npn_manager( unsigned hash_table_size = 4096, const npn_classifier_t& npn_func = make_exact_npn_canonization_wrapper() );

  tt compute( const tt& tt, boost::dynamic_bitset<>& phase, std::vector<unsigned>& perm );
  void print_statistics( std::ostream& os = std::cout ) const;

private:
  struct table_entry_t
  {
    table_entry_t() {}
    table_entry_t( const std::string& tt, const std::string& npn, const std::vector<unsigned>& perm, const boost::dynamic_bitset<>& phase )
      : tt( tt ), npn( npn ), perm( perm ), phase( phase ) {}

    std::string             tt;
    std::string             npn;
    std::vector<unsigned>   perm;
    boost::dynamic_bitset<> phase;
  };

  using table_t = std::vector<table_entry_t>;

  table_t          table;

  npn_classifier_t npn_func;

  double           runtime    = 0.0;
  unsigned long    cache_hit  = 0;
  unsigned long    cache_miss = 0;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
