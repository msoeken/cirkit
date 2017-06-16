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
 * @file functor.hpp
 *
 * @brief Functor with properties
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include <functional>

#include <core/properties.hpp>

namespace cirkit
{

  /**
   * @brief Functor class for interfacing algorithms
   *
   * When interfacing an algorithm, we wanna encapsulate
   * the settings and the statistics. That is, a user can still
   * provide settings from outside, but another algorithm
   * can change settings of a functor as well. Therewith,
   * this class extends the std::function object by adding
   * methods to access the corresponding settings and statistics
   * data from the respective algorithm.
   *
   * @since  1.0
   */
  template<typename T>
  class functor : public std::function<T>
  {
  public:
    /**
     * @brief Default constructor
     *
     * Calls the constructor of the base class.
     *
     * @since  1.0
     */
    functor()
      : std::function<T>() {}

    /**
     * @brief Copy constructor
     *
     * This copy constructor allows for example, the assignment
     * of other std::function objects. Note, that the settings
     * and statistics are not set with this constructor, but
     * have to be assigned explicitly using init().
     *
     * @param f Object to be assigned
     *
     * @since  1.0
     */
    template<typename F>
    functor( F f ) : std::function<T>( f ) {}

    /**
     * @brief Initializes the settings and statistics fields.
     *
     * @param settings Settings properties
     * @param statistics Statistics properties
     *
     * @since  1.0
     */
    void init( const properties::ptr& settings, const properties::ptr& statistics )
    {
      _settings = settings;
      _statistics = statistics;
    }

    /**
     * @brief Returns a smart pointer to the settings
     *
     * This smart pointer can be empty, if init() was never called.
     *
     * @return A smart pointer to the settings
     */
    const properties::ptr& settings() const
    {
      return _settings;
    }

    /**
     * @brief Returns a smart pointer to the statistics
     *
     * This smart pointer can be empty, if init() was never called.
     *
     * @return A smart pointer to the statistics
     */
    const properties::ptr& statistics() const
    {
      return _statistics;
    }

  private:
    /** @cond */
    properties::ptr _settings;
    properties::ptr _statistics;
    /** @endcond */
  };

}

#endif /* FUNCTOR_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
