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
 * @file read_specification.hpp
 *
 * @brief Parser for RevLib specification (*.spec) file format
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef READ_SPECIFICATION_HPP
#define READ_SPECIFICATION_HPP

#include <iosfwd>
#include <vector>

#include <reversible/truth_table.hpp>

#include <reversible/io/revlib_processor.hpp>

/**
 * @brief Main namespace
 */
namespace cirkit
{

  /**
   * @brief Implementation of revlib_processor to construct a reversible_truth_table
   *
   * This class inherits from revlib_processor and constructs
   * a reversible_truth_table when parsing a specification file.
   *
   * For convinience the function read_specification(reversible_truth_table&, const std::string&, std::string*)
   * wraps the use of this class to read truth tables from a file.
   *
   * @since  1.0
   */
  class specification_processor : public revlib_processor
  {
  public:
    /**
     * @brief Default constructor
     *
     * Initializes private data
     *
     * @param spec An empty reversible_truth_table which will be constructed
     *             and filled with entries while parsing the
     *             file
     *
     * @since  1.0
     */
    explicit specification_processor( binary_truth_table& spec );


    /**
     * @brief Default deconstructor
     *
     * Clears private data
     *
     * @since  1.0
     */
    virtual ~specification_processor();

  protected:
    virtual void on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;
    virtual void on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;
    virtual void on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const;
    virtual void on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const;
    virtual void on_truth_table_line( unsigned line_index, const std::vector<boost::optional<bool> >::const_iterator first, const std::vector<boost::optional<bool> >::const_iterator last ) const;

  private:
    class priv;
    priv* const d;
  };

  /**
   * @brief Read a specification into a truth table from stream
   *
   * This method uses revlib_parser(std::istream&, revlib_processor&, std::string*)
   * but with a specification_processor as reader. The
   * required empty reversible_truth_table for the reader is given as first
   * parameter.
   *
   * @param spec  truth table to be constructed
   * @param in    input stream containing the specification
   * @param error A pointer to a string. In case the parsing fails,
   *              and \p error is not null, a error message is stored
   * @return true on success, false otherwise
   *
   * @since  1.0
   */
  bool read_specification( binary_truth_table& spec, std::istream& in, std::string* error = 0 );

  /**
   * @brief Read a specification into a truth table from filename
   *
   * This method construts a \b std::ifstream of the given \p filename
   * and calls read_specification(reversible_truth_table&, std::istream&, std::string*)
   * with it.
   *
   * @section Example
   *
   * The following code demonstrates how to read a specification from
   * a file given by its filename into a truth table.
   *
   * @code
   * reversible_truth_table spec;
   * read_specification( spec, "function.spec" );
   * @endcode
   *
   * Sometimes it is useful to provide the caller with error information
   * in case the call failed. To obtain an error message a pointer to
   * a \b std::string is given as third parameter to the function. The function
   * returns \b false if it fails.
   *
   * @code
   * reversible_truth_table spec;
   * std::string error;
   *
   * if ( !read_specification( spec, "fucntion.spec", &error ) ) {
   *   std::cerr << "An error occured: " << error << std::endl;
   * }
   * @endcode
   *
   * @param spec     truth table to be constructed
   * @param filename filename of the specification
   * @param error    A pointer to a string. In case the parsing fails,
   *                 and \p error is not null, a error message is stored
   * @return true on success, false otherwise
   *
   * @since  1.0
   */
  bool read_specification( binary_truth_table& spec, const std::string& filename, std::string* error = 0 );
}

#endif /* READ_SPECIFICATION_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
