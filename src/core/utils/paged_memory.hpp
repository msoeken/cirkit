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
 * @file paged_memory.hpp
 *
 * @brief A container for paged memory
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef PAGED_MEMORY_HPP
#define PAGED_MEMORY_HPP

#include <vector>

#include <boost/range/iterator_range.hpp>

namespace cirkit
{

/* paged_memory
 *
 * This data structure represents a vector where each
 * element is a set of unsigned integer sets.
 *
 * For efficient traversal it manages the following containers
 *
 * data:      contains the set elements and set lenghts
 * offset[i]: start index in data for vector index i
 * count[i]:  number of sets for vector index i
 *
 * Example:
 *
 * Assume we want to store
 *
 *   [ {{0, 2, 3}, {1, 3}, {2}},
 *     {{0, 2}, {3, 0, 1}} ]
 *
 * data:
 *   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |
 *   | 3 | 0 | 2 | 3 | 2 | 1 | 3 | 1 | 2 | 2 |  0 |  2 |  3 |  3 |  0 |  1 |
 *
 * offset:
 *   | 0 | 1 |
 *   | 0 | 9 |
 *
 * count:
 *   | 0 | 1 |
 *   | 3 | 2 |
 *
 * We allow to store k additional values for each set.  This element is stored
 * directly after the size of the set.
 *
 * Example:
 *
 * Assume k = 1 and we want to store
 *
 *   [ {A => {0, 2, 3}, B => {1, 3}, C => {2}},
 *     {D => {0, 2}, E => {3, 0, 1}} ]
 *
 * data:
 *   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 |
 *   | 3 | A | 0 | 2 | 3 | 2 | B | 1 | 3 | 1 |  C |  2 |  2 |  D |  0 |  2 |  3 |  E |  3 |  0 |  1 |
 *
 * offset:
 *   | 0 |  1 |
 *   | 0 | 12 |
 *
 * count:
 *   | 0 | 1 |
 *   | 3 | 2 |
 */

class paged_memory
{
public:
  class set
  {
  public:
    friend class paged_memory;

  public:
    /* interface */
    using iterator   = std::vector<unsigned>::const_iterator;
    using value_type = unsigned;

    /* constructor */
    set( unsigned address, std::vector<unsigned>& data, unsigned additional );

    /* methods */
    std::size_t                     size() const;
    iterator                        begin() const;
    iterator                        end() const;
    boost::iterator_range<iterator> range() const;
    value_type                      extra( unsigned i ) const;
    void                            set_extra( unsigned i, value_type v );
    unsigned                        address() const;

  private:
    unsigned _address;
    std::vector<unsigned>& data;
    unsigned additional;
  };

  class iterator
  {
  public:
    /* interface */
    using reference         = set;
    using iterator_category = std::input_iterator_tag;
    using difference_type   = std::size_t;
    using value_type        = set;
    using pointer           = const set*;

    /* constructor */
    iterator( unsigned index, unsigned address, std::vector<unsigned>& data, unsigned additional );

    /* assignment operator */
    iterator& operator=( const iterator& other );

    /* operators */
    reference operator*() const;
    iterator& operator++();
    iterator  operator++(int);
    bool      operator==( iterator const& it ) const;
    bool      operator!=( iterator const& it ) const;

  private:
    unsigned index;
    unsigned address;
    std::vector<unsigned>& data;
    unsigned additional;
  };

  /* constructor */
  explicit paged_memory( unsigned n, unsigned k = 0u );

  /* methods */
  unsigned                        count( unsigned index ) const;
  boost::iterator_range<iterator> sets( unsigned index );
  unsigned                        sets_count() const;
  unsigned                        index( const set& s ) const;
  set                             from_address( unsigned address );
  set                             from_index( unsigned index );

  /* the following methods assume that nothing has been added to the index yet,
     i.e., it will update the offset */
  void                            assign_empty( unsigned index, const std::vector<unsigned>& extra = std::vector<unsigned>() );
  void                            assign_singleton( unsigned index, unsigned value, const std::vector<unsigned>& extra = std::vector<unsigned>() );

  /* the append_* methods do not update the offset, therefore append_begin() has to be
     called once before any append_* method is called */
  void                            append_begin( unsigned index );
  void                            append_singleton( unsigned index, unsigned value, const std::vector<unsigned>& extra = std::vector<unsigned>() );
  void                            append_set( unsigned index, const std::vector<unsigned>& values, const std::vector<unsigned>& extra = std::vector<unsigned>() );

  unsigned                        memory() const;

private:
  unsigned              _additional;
  std::vector<unsigned> _data;
  std::vector<unsigned> _offset;
  std::vector<unsigned> _count;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
