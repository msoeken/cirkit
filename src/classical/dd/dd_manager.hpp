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
