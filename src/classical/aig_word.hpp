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
