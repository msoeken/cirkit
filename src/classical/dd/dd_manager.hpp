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
 * @file dd_manager.hpp
 *
 * @brief Base class for DD managers
 *
 * @author Heinz Riener
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef DD_MANAGER_HPP
#define DD_MANAGER_HPP

#include <memory>
#include <ostream>
#include <vector>

namespace cirkit
{

class hash_cache
{
public:
  using value_type     = std::tuple<unsigned, unsigned, unsigned, int>;
  using container_type = std::vector<value_type>;
  using size_type      = container_type::size_type;

public:
  hash_cache( size_type log_size );
  int lookup( unsigned arg0, unsigned arg1, unsigned arg2 );
  int insert( unsigned arg0, unsigned arg1, unsigned arg2, int res );

  std::size_t cache_size() const;

  std::size_t hit () const;
  std::size_t miss () const;
private:
  inline container_type::reference entry( unsigned arg0, unsigned arg1, unsigned arg2 )
  {
    return data[( 12582917 * (int)arg0 + 4256249 * (int)arg1 + 741457 * (int)arg2 ) & mask];
  }

private:
  container_type data;
  unsigned mask;

  unsigned nhit;
  unsigned nmiss;
};

struct dd_node
{
  unsigned var;
  unsigned high;
  unsigned low;

  bool operator==( const dd_node& other ) const
  {
    return var == other.var && high == other.high && low == other.low;
  }
};

std::ostream& operator<<( std::ostream& os, const dd_node& z );

class dd_manager
{
public:
  dd_manager( unsigned nvars, unsigned log_max_objs, bool verbose = false );
  virtual ~dd_manager();

  inline unsigned num_vars() const { return nvars; }

  unsigned size() const;

  unsigned get_var( unsigned z ) const;
  unsigned get_high( unsigned z ) const;
  unsigned get_low( unsigned z ) const;

  void dump_stats ( std::ostream& stream ) const;

protected:
  unsigned unique_lookup( unsigned var, unsigned high, unsigned low );

protected:
  unsigned             nvars;
  unsigned             nnodes = 0u;
  unsigned             mask = 0u;
  hash_cache           cache;
  std::vector<dd_node> nodes;
  bool                 verbose;
  unsigned *           unique;
  unsigned *           nexts;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
