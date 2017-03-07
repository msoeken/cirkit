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
 * @file benchmark_table.hpp
 *
 * @brief Data-structures for producing benchmark tables
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef BENCHMARK_TABLE_HPP
#define BENCHMARK_TABLE_HPP

#include <iostream>
#include <iomanip>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/counting_range.hpp>

namespace cirkit
{

/* this is to format a cell */
using length_t = boost::optional<unsigned>;

template<typename T>
std::string print_cell( const T& value, const length_t& length = length_t() )
{
  assert( 0 );
}

template<>
std::string print_cell<>( const std::string& value, const length_t& length )
{
  if ( length )
  {
    return boost::str( boost::format( "%-" + boost::lexical_cast<std::string>( *length ) + "s" ) % value );
  }
  else
  {
    return value;
  }
}

template<>
std::string print_cell<>( const int& value, const length_t& length )
{
  if ( length )
  {
    return boost::str( boost::format( "%" + boost::lexical_cast<std::string>( *length ) + "d" ) % value );
  }
  else
  {
    return boost::lexical_cast<std::string>( value );
  }
}

template<>
std::string print_cell<>( const unsigned& value, const length_t& length )
{
  if ( length )
  {
    return boost::str( boost::format( "%" + boost::lexical_cast<std::string>( *length ) + "d" ) % value );
  }
  else
  {
    return boost::lexical_cast<std::string>( value );
  }
}

template<>
std::string print_cell<>( const double& value, const length_t& length )
{
  if ( length )
  {
    return boost::str( boost::format( "%" + boost::lexical_cast<std::string>( *length ) + ".2f" ) % value );
  }
  else
  {
    return boost::str( boost::format( "%.2f" ) % value );
  }
}

template<>
std::string print_cell<>( const boost::optional<unsigned>& value, const length_t& length )
{
  std::string timeout = "T/O";
  return value ? print_cell( *value, length ) : print_cell( timeout );
}

/* this is to display one row of the result set */
template<class TypleT, int N, typename... Arguments> struct print_row_t;
template<class TupleT, int N, class T, typename... Arguments> struct print_row_t<TupleT, N, T, Arguments...>
{
  static void print( const TupleT& t, const std::vector<unsigned>& column_lengths )
  {
    std::cout << "| " << print_cell( std::get<N>( t ), column_lengths[N] ) << " ";
    print_row_t<TupleT, N + 1, Arguments...>::print( t, column_lengths );
  }
};
template<class TupleT, int N, class T> struct print_row_t<TupleT, N, T>
{
  static void print( const TupleT& t, const std::vector<unsigned>& column_lengths )
  {
    std::cout << "| " << print_cell( std::get<N>( t ), column_lengths[N] ) << " |" << std::endl;
  }
};

template<class TupleT, typename... Arguments>
void print_row( const TupleT& t, const std::vector<unsigned>& column_lengths )
{
  print_row_t<TupleT, 0, Arguments...>::print( t, column_lengths );
};

/* the actual benchmark table */
template<class... Arguments>
class benchmark_table
{
public:
  using benchmark = std::tuple<Arguments...>;

  benchmark_table( std::initializer_list<std::string> column_names, bool verbose = false ) : verbose( verbose )
  {
    using boost::adaptors::transformed;
    boost::push_back( _column_names, column_names );
    boost::push_back( lengths, _column_names | transformed( []( const std::string& s ) { return s.size(); } ) );
  }

  template<typename T>
  void add_length( const T& value )
  {
    unsigned l = print_cell( value ).size();
    if ( l > lengths[length_i] )
    {
      lengths[length_i] = l;
    }
    ++length_i;
  }

  template<typename T>
  void compute_lengths(const T& value)
  {
    add_length( value );
  }

  template<typename U, typename... T>
  void compute_lengths(const U& head, const T&... tail)
  {
    add_length( head );
    compute_lengths(tail...);
  }

  template<class... Args>
  void add(Args&&... args)
  {
    length_i = 0u;
    compute_lengths( args... );
    results.push_back(std::make_tuple(std::forward<Args>(args)...));

    if ( verbose )
    {
      print_row<benchmark, Arguments...>( results.back(), lengths );
    }
  }

  void print() const
  {
    for ( unsigned n : boost::counting_range( 0u, (unsigned)_column_names.size() ) )
    {
      std::cout.setf( std::ios::left );
      std::cout << "| " << std::setw( lengths[n] ) << _column_names[n] << " ";
    }
    std::cout << "|" << std::endl;

    for ( const auto& result : results )
    {
      print_row<benchmark, Arguments...>( result, lengths );
    }
  }

private:
  bool verbose = false;
  std::vector<std::string> _column_names;
  std::vector<benchmark> results;
  std::vector<unsigned> lengths;
  unsigned length_i;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
