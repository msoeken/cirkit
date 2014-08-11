/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include <reversible/truth_table.hpp>
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

}

#endif

// Local Variables:
// c-basic-offset: 2
// End:
