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
 * @file properties.hpp
 *
 * @brief Property Map Implementation for Algorithms
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <boost/any.hpp>

namespace cirkit
{

  /**
   * @brief Property Map for storing settings and statistical information
   *
   * In this data structure settings and statistical data can be stored.
   * The key to access data is always of type \p std::string and the value
   * can be of any type. To be type-safe, the getter corresponding get
   * functions have to be provided with a type.
   *
   * @since  1.0
   */
  struct properties
  {
    /**
     * @brief Internal storage type used with the internal property map
     *
     * @since  1.0
     */
    using storage_type = std::map<std::string, boost::any>;

    /**
     * @brief Value type of the property map, i.e. \p std::string
     *
     * @since  1.0
     */
    using value_type = storage_type::mapped_type;

    /**
     * @brief Key type of the property map, i.e. \p boost::any
     *
     * There are pre-defined getter methods, which can be called with a
     * type identifier for explicit casting.
     *
     * @since  1.0
     */
    using key_type = storage_type::key_type;

    /**
     * @brief Smart Pointer version of this class
     *
     * Inside the framework, always the Smart Pointer version is used.
     * To have an easy access, there are special functions provided
     * which take the smart pointer as parameter and check as well
     * if it can be dereferenced.
     *
     * @sa get
     * @sa set_error_message
     *
     * @since  1.0
     */
    using ptr = std::shared_ptr<properties>;

    /**
     * @brief Standard constructor
     *
     * Creates the property map on base of the storage map
     *
     * @since  1.0
     */
    properties();

    /**
     * @brief Direct access to the value type
     *
     * Since the \p value_type is of type \p boost::any, it is not recommended
     * to use this operator, but rather get and set.
     *
     * @param k Key to access the property map. Must exist.
     * @return The value associated with key \p k.
     *
     * @since  1.0
     */
    const value_type& operator[]( const key_type& k ) const;

    /**
     * @brief Casted access to an existing element
     *
     * With \p T you can specify the type of the element. Note, that
     * it has to be the original used type, e.g. there is a difference
     * even between \p int and \p unsigned.
     *
     * The type is determined automatically using the set method.
     *
     * @param k Key to access the property map. Must exist.
     * @return The value associated with key \p k casted to its original type \p T.
     *
     * @since  1.0
     */
    template<typename T>
    T get( const key_type& k ) const
    {
      return boost::any_cast<T>( map.find( k )->second );
    }

    /**
     * @brief Casted access to an existing element with fall-back option
     *
     * The same as get(const key_type& k), but if \p k does not exist,
     * a default value is returned, which has to be of type \p T.
     *
     * @param k Key to access the property map. May not exist.
     * @param default_value If \p k does not exist, this value is returned.
     * @return The value associated with key \p k casted to its original type \p T. If the key \p k does not exist,
     *         \p default_value is returned.
     *
     * @since  1.0
     */
    template<typename T>
    T get( const key_type& k, const T& default_value ) const
    {
      if ( map.find( k ) == map.end() )
      {
        return default_value;
      }
      else
      {
        return boost::any_cast<T>( map.find( k )->second );
      }
    }

    /**
     * @brief Adds or modifies a value in the property map
     *
     * This methods sets the value located at key \p k to \p value.
     * If the key does not exist, it will be created.
     * Be careful which type was used, especially with typed constants:
     * @code
     * properties p;
     * p.set( "a unsigned number", 5u );
     * p.get<unsigned>( "a unsigned number" ); // OK!
     * p.get<int>( "a unsigned number" );      // FAIL!
     *
     * p.set( "a signed number", 5 );
     * p.get<unsigned>( "a signed number" );   // FAIL!
     * p.get<int>( "a signed number" );        // OK!
     * @endcode
     *
     * @param k Key of the property
     * @param value The new value of \p k. If \p k already existed, the type of \p value must not change.
     *
     * @since  1.0
     */
    void set( const key_type& k, const value_type& value );

    /**
     * @brief Start iterator for the properties
     *
     * @return Iterator
     *
     * @since  1.1
     */
    storage_type::const_iterator begin() const;

    /**
     * @brief End iterator for the properties
     *
     * @return Iterator
     *
     * @since  1.1
     */
    storage_type::const_iterator end() const;

    /**
     * @brief Number of properties
     *
     * @return The number of properties
     *
     * @since  1.0
     */
    unsigned size() const;

    /**
     * @brief Clears all properties
     *
     * @since  1.1
     */
    void clear();

    /**
     * @brief Checks whether key exists
     *
     * @since 2.3
     */
    bool has_key( const key_type& k ) const
    {
      return map.find( k ) != map.end();
    }

  private:
    storage_type      map;
  };

  /**
   * @brief A helper method to access the get method on a properties smart pointer
   *
   * This method has basically two fall backs. If settings does not point to anything,
   * it returns \p default_value, and otherwise it calls the get method on the
   * pointee of the smart pointer with the \p default_value again, so in case the key \p k
   * does not exists, the \p default_value is returned as well.
   *
   * @param settings A smart pointer to a properties instance or an empty smart pointer
   * @param k Key of the property to be accessed
   * @param default_value A default_value as fall back option in case the smart pointer
   *                      is empty or the key does not exist.
   *
   * @return The value addressed by \p k or the \p default_value.
   *
   * @since  1.0
   */
  template<typename T>
  T get( const properties::ptr& settings, const properties::key_type& k, const T& default_value )
  {
    if ( settings )
    {
      return settings->get<T>( k, default_value );
    }
    else
    {
      return default_value;
    }
  }

  /**
   * @brief A helper method to access the set method on a properties smart pointer
   *
   * It doesn't fail if the properties smart pointer has no pointee.
   *
   * @since 2.3
   */
  template<typename T>
  void set( const properties::ptr& settings, const properties::key_type& k, const T& value )
  {
    if ( settings )
    {
      settings->set( k, value );
    }
  }

  /**
   * @brief Sets an error message to a statistics smart pointer
   *
   * This function checks first if the smart pointer references something,
   * and if that is the case, the value \p error, is written to the key
   * \b error.
   *
   * @param statistics A smart pointer to a properties instance or an empty smart pointer
   * @param error An error message, which should be written to the key \b error
   *              if the smart pointer \p statistics can be de-referenced.
   *
   * @since  1.0
   */
  void set_error_message( properties::ptr statistics, const std::string& error );

/**
 * Utility functions to create settings object
 */

namespace detail
{

template<typename T>
struct make_one_setting
{
  static void impl( const properties::ptr& settings, T v )
  {
    std::cout << "unknown" << std::endl;
  }
};

template<class S, class U>
struct make_one_setting<std::pair<S, U>>
{
  static void impl( const properties::ptr& settings, std::pair<S, U> p )
  {
    settings->set( p.first, p.second );
  }
};

template<>
struct make_one_setting<std::string>
{
  static void impl( const properties::ptr& settings, std::string s )
  {
    settings->set( s, true );
  }
};

template<>
struct make_one_setting<const char*>
{
  static void impl( const properties::ptr& settings, const char* s )
  {
    settings->set( s, true );
  }
};

}

void make_settings_rec( const properties::ptr& settings );

template<class T, class... Ts>
void make_settings_rec( const properties::ptr& settings, T value, Ts... rest )
{
  detail::make_one_setting<T>::impl( settings, value );
  make_settings_rec( settings, rest... );
}

template<class... Ts>
properties::ptr make_settings_from( Ts... values )
{
  const auto settings = std::make_shared<properties>();
  make_settings_rec( settings, values... );
  return settings;
}

}

#endif /* PROPERTIES_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
