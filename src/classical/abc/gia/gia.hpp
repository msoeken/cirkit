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
 * @file gia.hpp
 *
 * @brief GIA wrapper
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ABC_GIA_HPP
#define ABC_GIA_HPP

#include <iostream>
#include <string>
#include <vector>

#include <core/properties.hpp>
#include <classical/aig.hpp>
#include <classical/utils/truth_table_utils.hpp>

#define LIN64
#include <base/main/main.h>
#include <aig/gia/gia.h>
#include <misc/vec/vecWec.h>

#include <cuddObj.hh>

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
  inline bool okay() const { return p_gia.get() != nullptr; }

  /** implicit conversion to ABC type */
  inline operator abc::Gia_Man_t*() const { return p_gia.get(); }

  /** GIA properties */
  inline int size() const { return abc::Gia_ManObjNum( p_gia.get() ); }
  std::string input_name( int input_index ) const;
  std::string output_name( int output_index ) const;

  /** access GIA object */
  inline abc::Gia_Obj_t* object( int index ) const { return abc::Gia_ManObj( p_gia.get(), index ); }
  inline unsigned& value( int index ) const { return abc::Gia_ManObj( p_gia.get(), index )->Value; }

  /** number of inputs */
  inline int num_inputs() const { return abc::Gia_ManPiNum( p_gia.get() ); }
  inline int num_outputs() const { return abc::Gia_ManPoNum( p_gia.get() ); }

  /* number of XORs */
  int num_xors() const;

  /// OPERATIONS
  gia_graph cofactor( int var, bool value ) const;
  gia_graph select_outputs( const std::vector<int>& indexes ) const;
  gia_graph syn3() const;
  gia_graph syn4() const;

  /// MAPPING

  gia_graph if_mapping( const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() ) const;
  void satlut_mapping( const properties::ptr& settings = properties::ptr(), const properties::ptr& statistics = properties::ptr() ) const;

  inline bool has_mapping() const { return abc::Gia_ManHasMapping( p_gia.get() ) == 1; }
  inline int lut_count() const { return abc::Gia_ManLutNum( p_gia.get() ); }
  inline int max_lut_size() const { return abc::Gia_ManLutSizeMax( p_gia.get() ); }
  inline bool is_lut( int index ) const { return abc::Gia_ObjIsLut( p_gia.get(), index ); }
  inline int lut_size( int index ) const { return abc::Gia_ObjLutSize( p_gia.get(), index ); }
  inline int lut_ref_num( int index ) const { return Gia_ObjLutRefNumId( p_gia.get(), index ); }
  inline int lut_ref_dec( int index ) const { return Gia_ObjLutRefDecId( p_gia.get(), index ); }
  inline int lut_ref_inc( int index ) const { return Gia_ObjLutRefIncId( p_gia.get(), index ); }

  void init_lut_refs() const;
  gia_graph extract_lut( int index ) const;

  void init_truth_tables() const;
  uint64_t lut_truth_table( int index ) const;

  tt truth_table( int output_index ) const;

  /// PRINTING

  void print_stats() const;

  /// I/O

  void write_aiger( const std::string& filename ) const;
  void write_dot( const std::string& filename, const std::vector<int>& highlight ) const;
  void write_dot_with_luts( const std::string& filename ) const;

  /// OTHER LOGIC REPRESENTATIONS

  using esop_ptr = std::unique_ptr<abc::Vec_Wec_t, decltype(&abc::Vec_WecFree)>;

  enum class esop_cover_method { aig, aig_new, aig_threshold, bdd };
  esop_ptr compute_esop_cover( esop_cover_method method = esop_cover_method::aig, const properties::ptr& settings = properties::ptr() ) const;

  /// ITERATORS

  template<typename Fn>
  void foreach_input( Fn&& f ) const
  {
    abc::Gia_Obj_t* obj{};
    int i{};
    Gia_ManForEachPi( p_gia.get(), obj, i )
    {
      f( abc::Gia_ManCiIdToId( p_gia.get(), i ), i );
    }
  }

  template<typename Fn>
  void foreach_output( Fn&& f ) const
  {
    abc::Gia_Obj_t* obj{};
    int i{};
    Gia_ManForEachPo( p_gia.get(), obj, i )
    {
      f( abc::Gia_ManCoIdToId( p_gia.get(), i ), i );
    }
  }

  template<typename Fn>
  void foreach_and( Fn&& f ) const
  {
    abc::Gia_Obj_t* obj{};
    int i{};
    Gia_ManForEachAnd( p_gia.get(), obj, i )
    {
      f( i, obj );
    }
  }

  template<typename Fn>
  void foreach_lut( Fn&& f ) const
  {
    int i{};
    Gia_ManForEachLut( p_gia.get(), i )
    {
      f( i );
    }
  }

  template<typename Fn>
  void foreach_lut_fanin( int index, Fn&& f ) const
  {
    int i{}, k{};
    Gia_LutForEachFanin( p_gia.get(), index, i, k )
    {
      f( i );
    }
  }

private:
  using gia_ptr = std::shared_ptr<abc::Gia_Man_t>;
  gia_ptr p_gia;

  mutable abc::Vec_Wrd_t* p_truths = nullptr;
};

std::istream& operator>>( std::istream& in, gia_graph::esop_cover_method& cover_method );
std::ostream& operator<<( std::ostream& out, const gia_graph::esop_cover_method& cover_method );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
