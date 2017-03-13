/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
 * @file gia.hpp
 *
 * @brief GIA wrapper
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ABC_GIA_HPP
#define ABC_GIA_HPP

#include <core/properties.hpp>
#include <classical/aig.hpp>

#define LIN64
#include <base/main/main.h>
#include <aig/gia/gia.h>
#include <misc/vec/vecWec.h>

namespace cirkit
{

class gia_graph
{
public:
  /** construct GIA from GIA */
  explicit gia_graph( abc::Gia_Man_t* gia );

  /** construct GIA from filename */
  explicit gia_graph( const std::string& filename );

  /** construct GIA from aig_graph */
  explicit gia_graph( const aig_graph& aig );

  ~gia_graph();

  /// CONSISTENCY AND CONVERSION

  /** checks if graph is okay */
  inline bool okay() const { return p_gia != nullptr; }

  /** implicit conversion to ABC type */
  inline operator abc::Gia_Man_t*() const { return p_gia; }

  /** access GIA object */
  inline abc::Gia_Obj_t* object( int index ) const { return abc::Gia_ManObj( p_gia, index ); }

  /// MAPPING

  gia_graph if_mapping( const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() );

  gia_graph extract_lut( int index ) const;

  /// PRINTING

  void print_stats() const;

  /// I/O

  void write_aiger( const std::string& filename ) const;

  /// OTHER LOGIC REPRESENTATIONS

  using esop_ptr = std::unique_ptr<abc::Vec_Wec_t, decltype(&abc::Vec_WecFree)>;
  esop_ptr compute_esop_cover() const;

  /// ITERATORS

  template<typename Fn>
  void foreach_lut( Fn&& f ) const
  {
    int i{};
    Gia_ManForEachLut( p_gia, i )
    {
      f( i );
    }
  }

private:
  abc::Gia_Man_t* p_gia = nullptr;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
