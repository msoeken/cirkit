/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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
 * @file store.hpp
 *
 * @brief A store for the CLI environment
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_STORE_HPP
#define CLI_STORE_HPP

#include <string>
#include <vector>

#include <boost/format.hpp>

#include <core/properties.hpp>
#include <core/utils/program_options.hpp>

namespace cirkit
{

template<class T>
class cli_store
{
public:
  explicit cli_store( const std::string& name ) : _name( name ) {}

  inline T& current()
  {
    if ( _current < 0 )
    {
      throw boost::str( boost::format( "[e] no current %s available" ) % _name );
    }
    return _data[_current];
  }

  inline const T& current() const
  {
    if ( _current < 0 )
    {
      throw boost::str( boost::format( "[e] no current %s available" ) % _name );
    }
    return _data.at( _current );
  }

  inline T& operator*()
  {
    return current();
  }

  inline const T& operator*() const
  {
    return current();
  }

  inline T& operator[]( unsigned i )
  {
    return _data[i];
  }

  inline const T& operator[]( unsigned i ) const
  {
    return _data.at( i );
  }

  inline bool empty() const
  {
    return _data.empty();
  }

  inline const std::vector<T>& data() const
  {
    return _data;
  }

  inline typename std::vector<T>::size_type size() const
  {
    return _data.size();
  }

  inline int current_index() const
  {
    return _current;
  }

  inline void set_current_index( unsigned i )
  {
    _current = i;
  }

  void extend()
  {
    const auto s = _data.size();
    _data.resize( s + 1u );
    _current = s;
    current() = T();
  }

  void clear()
  {
    _data.clear();
    _current = -1;
  }

private:
  std::string    _name;
  std::vector<T> _data;
  int            _current = -1;
};

/* for customizing stores */
template<typename T>
struct store_info {};

template<typename T>
std::string store_entry_to_string( const T& element )
{
  return "UNKNOWN";
}

template<typename T>
void print_store_entry( std::ostream& os, const T& element )
{
  os << "UNKNOWN" << std::endl;
}

template<typename T>
struct show_store_entry
{
  show_store_entry( program_options& opts ) {}

  void operator()( T& element, const std::string& dotname, const program_options& opts, const properties::ptr& settings )
  {
    std::cout << "[w] show is not supported for this store element" << std::endl;
  }
};

// template<typename T>
// void show_store_entry( T& element, const std::string& dotname, unsigned levels, const properties::ptr& settings )
// {
//   std::cout << "[w] show is not supported for this store element" << std::endl;
// }

template<typename T>
void print_store_entry_statistics( std::ostream& os, const T& element )
{
  os << "UNKNOWN" << std::endl;
}

/* for the use in commands */
template<typename S>
int add_option_helper( program_options& opts )
{
  constexpr auto option   = store_info<S>::option;
  constexpr auto mnemonic = store_info<S>::mnemonic;
  opts.add_options()
    ( ( boost::format( "%s,%s" ) % option % mnemonic ).str().c_str(), store_info<S>::name_plural )
    ;
  return 0;
}

template<typename T>
bool any_true_helper( std::initializer_list<T> list )
{
  for ( auto i : list )
  {
    if ( i ) { return true; }
  }

  return false;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
