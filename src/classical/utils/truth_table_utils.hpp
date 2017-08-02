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
 * @file truth_table_utils.hpp
 *
 * @brief Truth table utilities
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef TRUTH_TABLE_UTILS_HPP
#define TRUTH_TABLE_UTILS_HPP

#include <boost/dynamic_bitset.hpp>

#include <core/utils/bitset_utils.hpp>
#include <classical/utils/expression_parser.hpp>

namespace cirkit
{

/**
 * @brief Truth table type
 *
 * Most of the functions implemented in this file are inspired by
 *   https://bitbucket.org/alanmi/abc/src/768916cc932a3608c7bc8b27d3d4cb0e15727d63/src/misc/util/utilTruth.h?at=default
 *   and
 *   https://bitbucket.org/sterin/pyaig/src/fd77fb0298da597e8b80c85cc7b5391d7ba7d1e4/src/pyaig/truthtables.py?at=default
 *
 * We assume a truth table argues on variables x_0, ..., x_{n-1}
 */
using tt = boost::dynamic_bitset<>;

class tt_store
{
public:
  tt_store( tt_store const& ) = delete;
  void operator=( tt_store const& ) = delete;

  static tt_store& i();
  const unsigned width = 6u;

  inline const tt& operator()( unsigned i ) const
  {
    return tts.at( i );
  }

  inline const std::vector<unsigned>& swaps( unsigned i ) const
  {
    assert( i >= 2u && i <= 6u );
    return _swaps.at( i - 2u );
  }

  inline const std::vector<unsigned>& flips( unsigned i ) const
  {
    assert( i >= 2u && i <= 6u );
    return _flips.at( i - 2u );
  }

private:
  tt_store();

  const std::vector<tt> tts  = { tt( 64u, 0xaaaaaaaaaaaaaaaa ),
                                 tt( 64u, 0xcccccccccccccccc ),
                                 tt( 64u, 0xf0f0f0f0f0f0f0f0 ),
                                 tt( 64u, 0xff00ff00ff00ff00 ),
                                 tt( 64u, 0xffff0000ffff0000 ),
                                 tt( 64u, 0xffffffff00000000 ) };

  const std::vector<std::vector<unsigned>> _swaps = { {0},
                                                      {1, 0, 1, 0, 1},
                                                      {2, 1, 0, 2, 0, 1, 2, 0, 2, 1, 0, 2, 0, 1, 2, 0, 2, 1, 0, 2, 0, 1, 2},
                                                      {3, 2, 1, 0, 3, 0, 1, 2, 3, 1, 3, 2, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 1, 0, 1, 0, 1, 2, 3, 1, 3, 2, 1, 0, 3, 0, 1, 2, 3, 0, 3, 2, 1, 0, 3, 0, 1, 2, 3, 1, 3, 2, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 1, 0, 1, 0, 1, 2, 3, 1, 3, 2, 1, 0, 3, 0, 1, 2, 3, 0, 3, 2, 1, 0, 3, 0, 1, 2, 3, 1, 3, 2, 1, 0, 1, 0, 1, 2, 3, 2, 3, 2, 1, 0, 1, 0, 1, 2, 3, 1, 3, 2, 1, 0, 3, 0, 1, 2, 3},
                                                      {4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 1, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 1, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 1, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 1, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 1, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 1, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4, 0, 4, 3, 2, 1, 0, 2, 0, 1, 2, 3, 4, 2, 4, 3, 2, 1, 0, 4, 0, 1, 2, 3, 4} };

  const std::vector<std::vector<unsigned>> _flips = { {0, 1, 0},
                                                      {0, 1, 0, 2, 0, 1, 0},
                                                      {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0},
                                                      {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0},
                                                      {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0} };
};

/**
 * @brief Constant 0 truth table
 *
 * Has size 2^0
 */
tt tt_const0();

/**
 * @brief Constant 1 truth table
 *
 * Has size 2^0
 */
tt tt_const1();

/*
 * @brief Truth table for function x_i
 *
 * Has size 2^{i+1}
 */
tt tt_nth_var( unsigned i );

/**
 * @brief Returns number of variables in truth table
 *
 * This is not the support size but only n (see above)
 */
unsigned tt_num_vars( const tt& t );

/**
 * @brief Extends a truth table to fit n variables
 */
void tt_extend( tt& t, unsigned to );

/**
 * @brief Shrinks a truth table to fit n variables
 *
 * This may cut off variables
 */
void tt_shrink( tt& t, unsigned to );

/**
 * @brief Aligns two truth tables two same size
 */
void tt_align( tt& t1, tt& t2 );

/**
 * @brief Checks whether x_i is in the support set of t
 */
bool tt_has_var( const tt& t, unsigned i );

/**
 * @brief Returns a bitset which encodes the supporting set
 *
 * The bitset is enabled at position i if x_i is in the
 * support of t.  The bitset is of size n, where n is the
 * size of t.
 */
boost::dynamic_bitset<> tt_support( const tt& t );

/**
 * @brief Returns the number of variables in the support of t
 */
unsigned tt_support_size( const tt& t );

/**
 * @brief Computes the 0 cofactor
 */
tt tt_cof0( const tt& t, unsigned i );

/**
 * @brief Computes the 1 cofactor
 */
tt tt_cof1( const tt& t, unsigned i );

bool tt_cof0_is_const0( const tt& t, unsigned i );
bool tt_cof0_is_const1( const tt& t, unsigned i );
bool tt_cof1_is_const0( const tt& t, unsigned i );
bool tt_cof1_is_const1( const tt& t, unsigned i );
bool tt_cofs_opposite( const tt& t, unsigned i );

void tt_resize( tt& t, unsigned size );
bool tt_is_const0( const tt& t );
bool tt_is_const1( const tt& t );

/**
 * @brief Existential quantification
 */
tt tt_exists( const tt& t, unsigned i );

/**
 * @brief Universal quantification
 */
tt tt_forall( const tt& t, unsigned i );

/**
 * @brief Permutes variables i and j
 */
tt tt_permute( const tt& t, unsigned i, unsigned j );

/**
 * @brief Removes variable i
 */
tt tt_remove_var( const tt& t, unsigned i );

/**
 * @brief Flips variable i
 */
tt tt_flip( const tt& t, unsigned i );

/**
 * @brief Compact truth table to minbase
 *
 * If `psupport' variable is given, then the support is stored
 * there which anyway needs to be computed.
 */
void tt_to_minbase( tt& t, boost::dynamic_bitset<>* psupport = nullptr );

/**
 * @brief Compact truth table to minbase and discard some variables
 *
 * If `psupport' variable is given, then the support is stored
 * there which anyway needs to be computed.
 */
void tt_to_minbase_and_discard( tt& t, unsigned max_size = 6u, boost::dynamic_bitset<>* psupport = nullptr );

/**
 * @brief Uncompact truth table
 */
void tt_from_minbase( tt& t, const boost::dynamic_bitset<> pattern );

/**
 * @brief Converts truth table to hex string
 */
std::string tt_to_hex( const tt& t );

/**
 * @brief Converts hex string to truth table
 *
 * The truth table has at least two variables.
 */
tt tt_from_hex( const std::string& s );

/**
 * @brief Converts hex string to a truth table and adjusts size
 *
 * This may cut off variables
 */
tt tt_from_hex( const std::string& s, unsigned to );

/**
 * @brief Iterate through each minterm
 */
template<typename Fn>
void foreach_minterm( const tt& t, Fn&& f )
{
  const auto n = tt_num_vars( t );
  foreach_bit( t, [&]( unsigned pos ) {
      f( boost::dynamic_bitset<>( n, pos ) );
    } );
}

/**
 * @brief Creates a truth table from an expression
 */
class tt_expression_evaluator : public expression_evaluator<std::pair<tt, unsigned>>
{
public:
  std::pair<tt, unsigned> on_const( bool value ) const;
  std::pair<tt, unsigned> on_var( unsigned index ) const;
  std::pair<tt, unsigned> on_inv( const std::pair<tt, unsigned>& value ) const;
  std::pair<tt, unsigned> on_and( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2 ) const;
  std::pair<tt, unsigned> on_or( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2 ) const;
  std::pair<tt, unsigned> on_maj( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2, const std::pair<tt, unsigned>& value3 ) const;
  std::pair<tt, unsigned> on_xor( const std::pair<tt, unsigned>& value1, const std::pair<tt, unsigned>& value2 ) const;
};

tt tt_from_expression( const expression_t::ptr& expr );

tt tt_from_sop_spec( const std::string& spec );

std::vector<int> walsh_spectrum( const tt& func );

tt tt_maj( tt a, tt b, tt c );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
