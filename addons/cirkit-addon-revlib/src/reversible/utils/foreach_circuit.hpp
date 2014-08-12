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
 * @file foreach_circuit.hpp
 *
 * @brief Iterates through RevLib circuits
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef FOREACH_CIRCUIT_HPP
#define FOREACH_CIRCUIT_HPP

#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <reversible/circuit.hpp>
#include <reversible/io/read_realization.hpp>

namespace cirkit
{

template<typename Cond, typename _Fn>
void foreach_circuit_if( _Fn&& __fn, Cond&& cond )
{
  using namespace boost::filesystem;

  path p( "../ext/circuits/" );

  for ( const auto& entry : boost::make_iterator_range( directory_iterator( p ), directory_iterator() ) )
  {
    if ( entry.path().extension() != ".real" || !cond( entry ) ) continue;

    __fn( entry.path() );
  }
}

template<typename Range, typename _Fn>
void foreach_circuit_with_blacklist( const Range& blacklist, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_circuit_if( __fn, [&blacklist]( const directory_entry& e ) { return boost::find( blacklist, e.path().stem() ) == boost::end( blacklist ); } );
}

template<typename Range, typename _Fn>
void foreach_circuit_with_whitelist( const Range& whitelist, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_circuit_if( __fn, [&whitelist]( const directory_entry& e ) { return boost::find( whitelist, e.path().stem() ) != boost::end( whitelist ); } );
}

template<typename _Fn>
void foreach_circuit_with_max_lines( unsigned max_lines, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_circuit_if( __fn, [&max_lines]( const directory_entry& e ) {
      circuit circ;
      read_realization_settings settings;
      settings.read_gates = false;
      read_realization( circ, e.path().relative_path().string(), settings );
      return circ.lines() <= max_lines;
    });
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
