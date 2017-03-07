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
 * @file program_options.hpp
 *
 * @brief Easier access to program options
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef PROGRAM_OPTIONS_HPP
#define PROGRAM_OPTIONS_HPP

#include <boost/program_options.hpp>

namespace cirkit
{

  /**
   * @brief Class for program options on top of the Boost.Program_Options library
   *
   * This class can be used when writing programs for accessing algorithms.
   * It parses from C argc and argv variables and has some functions for
   * adding program options for common used parameters like input realization or
   * specification filename and output realization filename.
   *
   * @note It can be useful to check the <a href="http://www.boost.org/doc/libs/1_41_0/doc/html/program_options.html">Boost.Program_Options</a>
   *       documentation for further information.
   *
   * @section sec_example_program_options
   *
   * This could be used for a synthesis algorithm to read a specification
   * and write the result to a realization.
   */
  class program_options : public boost::program_options::options_description
  {
  public:
    /**
     * @brief Default constructor
     *
     * Calls the constructor of the boost::program_options::options_description
     * base class and adds a --help option.
     *
     * @param line_length Length of the terminal where to output
     *
     * @since  1.0
     */
    explicit program_options( unsigned line_length = m_default_line_length );

    /**
     * @brief Constructor with setting a caption for usage output
     *
     * Calls the constructor of the boost::program_options::options_description
     * base class and adds a --help option.
     *
     * @param caption     A caption  is primarily useful for output
     * @param line_length Length of the terminal where to output
     *
     * @since  1.0
     */
    explicit program_options( const std::string& caption, unsigned line_length = m_default_line_length );

    /**
     * @brief Default deconstructor
     */
    virtual ~program_options();

    /**
     * @brief Is help needed? Are all properties set properly?
     *
     * This method returns true when the --help option is not set
     * and when the --filename option is set, as far as either
     * add_read_realization_option() or add_read_specification_option()
     * was called before.
     *
     * @return true, when all properties are set properly. Otherwise false.
     *
     * @since  1.0
     */
    virtual bool good() const;

    /**
     * @brief Parses the command line
     *
     * @param argc C argc argument of the main function
     * @param argv C argv argument of the main function
     *
     * @since  1.0
     */
    void parse( int argc, char ** argv );

    /**
     * @brief Checks whether a parameter was set or not
     *
     * This method calls Boost's variable_map::count method
     * and checks if the parameter was set at least once.
     *
     * This class should be simple in general so there is no
     * distinction between one or more options of the same name.
     *
     * @param option Name of the option
     *
     * @return true when \p option was set, false otherwise.
     *
     * @since  1.0
     */
    bool is_set( const std::string& option ) const;

    void clear();

    void set_positional_option( const std::string& name );

    const boost::program_options::variables_map& variables() const;

  protected:
    bool parsed() const;

  private:
    void init();

    class priv;
    priv* const d;
  };

  template<class T>
  boost::program_options::typed_value<T>* value_with_default( T* v )
  {
    return boost::program_options::value( v )->default_value( *v );
  }

}

#endif /* PROGRAM_OPTIONS_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
