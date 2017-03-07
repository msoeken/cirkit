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
 * @file aig_word.hpp
 *
 * @brief AIG word level operations
 *
 * The implementation of the word-level operations is based on
 * metaSMT's implementation of Finn Haedicke and Stefan Frehse.
 *
 * @author Heinz Riener
 * @since  2.0
 */

#ifndef AIG_WORD_HPP
#define AIG_WORD_HPP

#include <classical/aig.hpp>

namespace cirkit
{

typedef std::vector< aig_function > aig_word;

aig_word to_aig_word( const aig_function& f );
const aig_function &to_aig_function( const aig_word& w );

aig_word aig_create_wi( aig_graph& aig, const unsigned width, const std::string& name );
void aig_create_wo( aig_graph& aig, const aig_word& w, const std::string& name );
aig_word aig_create_bvbin( aig_graph& aig, const std::string& value );
aig_function aig_create_equal( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvult( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvule( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvuge( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvugt( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvslt( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvsle( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvsgt( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_function aig_create_bvsge( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_bvand( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_bvor( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_bvxor( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_ite( aig_graph& aig, const aig_function& cond, const aig_word& t, const aig_word& e );

aig_word aig_create_bvadd( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_bvmul( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_bvsub( aig_graph& aig, const aig_word& left, const aig_word& right );
aig_word aig_create_sext( aig_graph& aig, const unsigned width, const aig_word& w );
aig_word aig_create_zext( aig_graph& aig, const unsigned width, const aig_word& w );
aig_word aig_create_bvnot( aig_graph& aig, const aig_word& w );
aig_word aig_create_bvneg( aig_graph& aig, const aig_word& w );

std::ostream& operator<<( std::ostream& os, const aig_word &w );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
