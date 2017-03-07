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
 * @file foreach_function.hpp
 *
 * @brief Iterates through RevLib functions
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef FOREACH_FUNCTION_HPP
#define FOREACH_FUNCTION_HPP

#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>
#include <reversible/io/read_pla.hpp>

namespace cirkit
{

template<typename Cond, typename _Fn>
void foreach_function_if( _Fn&& __fn, Cond&& cond )
{
  using namespace boost::filesystem;

  path p( "../ext/functions/" );

  for ( const auto& entry : boost::make_iterator_range( directory_iterator( p ), directory_iterator() ) )
  {
    if ( entry.path().extension() != ".pla" || !cond( entry ) ) continue;

    __fn( entry.path() );
  }
}

template<typename Range, typename _Fn>
void foreach_function_with_blacklist( const Range& blacklist, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_function_if( __fn, [&blacklist]( const directory_entry& e ) { return boost::find( blacklist, e.path().stem() ) == boost::end( blacklist ); } );
}

template<typename Range, typename _Fn>
void foreach_function_with_whitelist( const Range& whitelist, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_function_if( __fn, [&whitelist]( const directory_entry& e ) { return boost::find( whitelist, e.path().stem() ) != boost::end( whitelist ); } );
}

template<typename _Fn>
void foreach_function_with_max_variables( unsigned num_variables, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_function_if( __fn, [&num_variables]( const directory_entry& e ) {
      binary_truth_table spec;
      read_pla_settings settings;
      settings.extend = false;
      settings.skip_after_first_cube = true;
      read_pla( spec, e.path().relative_path().string(), settings );
      return ( spec.num_inputs() + spec.num_outputs() ) <= num_variables;
    });

}

/**
 * @brief Creates truth tables from irreversible functions
 *
 * The function iterates over all functions of $n$ variables and
 * creates a truth table, where the function is embedded in the
 * first column.  If the function is not balanced an additional
 * line is added, and the function is embedded at constant 0.
 */
template<typename _Fn>
void foreach_function_as_truth_table( unsigned num_variables, _Fn&& __fn )
{
  boost::dynamic_bitset<> bs( 1u << num_variables );

  do {
    __fn( bs, truth_table_from_bitset( bs ) );
    inc( bs );
  } while ( bs.any() );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
