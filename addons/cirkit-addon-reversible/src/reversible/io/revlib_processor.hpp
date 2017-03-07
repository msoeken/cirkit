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
 * @file revlib_processor.hpp
 *
 * @brief Processor which works with the revlib_parser
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef REVLIB_PROCESSOR_HPP
#define REVLIB_PROCESSOR_HPP

#include <iosfwd>
#include <vector>

#include <boost/any.hpp>
#include <boost/optional.hpp>

#include <core/properties.hpp>
#include <reversible/circuit.hpp>
#include <reversible/io/revlib_parser.hpp>

namespace cirkit
{

  /**
   * @brief Base class for actions on the revlib_parser
   *
   * The RevLib file parser revlib_parser(std::istream&, revlib_processor&, std::string*)
   * gets an instance of a revlib_processor object as parameter.
   *
   * For creating a circuit a circuit_processor
   * already exists. Other processors can be implemented by
   * inhereting from this class.
   *
   * The base class provides virtual functions for each command
   * in a RevLib file with corresponding parameters.
   *
   * Only the methods which data is needed must be overriden.
   *
   * @since  1.0
   */
  class revlib_processor
  {
  public:
    /**
     * @brief Default constructor
     *
     * Initializes private data
     *
     * @since  1.0
     */
    revlib_processor();

    /**
     * @brief Default deconstructor
     *
     * Clears private data
     *
     * @since  1.0
     */
    virtual ~revlib_processor();

  private:
    friend bool revlib_parser( std::istream& in, revlib_processor& reader, const revlib_parser_settings&, std::string* error );

  protected:
    /**
     * @brief Called when parsing comments
     *
     * This method is called when a comment (starting with a #)
     * is parsed. The comment (text after the #) is stored
     * in the parameter \p comment.
     *
     * @param comment Comment (single line)
     *
     * @since  1.0
     */
    virtual void on_comment( const std::string& comment ) const;

    /**
     * @brief Called when parsing .version command
     *
     * This method is called when a \b .version command is parsed.
     * The version (as a \b std::string) is stored in the
     * parameter \p version.
     *
     * @param version Version
     *
     * @since  1.0
     */
    virtual void on_version( const std::string& version ) const;

    /**
     * @brief Called when parsing .numvars command
     *
     * This method is called when a \b .numvars command is parsed.
     * The number of variables is stored in the parameter \p numvars.
     *
     * @param numvars Number of variables
     *
     * @since  1.0
     */
    virtual void on_numvars( unsigned numvars ) const;

    /**
     * @brief Called when parsing .variables command
     *
     * This method is called when a \b .variables command is parsed.
     * The variables in a \b std::vector can be accessed via
     * their \p first and \p last iterator.
     *
     * @note When overriding this function, make sure to call the
     *       base method first, because private data is filled
     *       with the variable names already in this class to
     *       access variable names with the method
     *       variable(std::vector<std::string>::size_type).
     *
     * @param first Begin iterator of variables vector
     * @param last  End iterator of variables vector
     *
     * @since  1.0
     */
    virtual void on_variables( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;

    /**
     * @brief Called when parsing .inputs command
     *
     * This method is called when a \b .inputs command is parsed.
     * The inputs in a \b std::vector can be accessed via
     * their \p first and \p last iterator.
     *
     * @param first Begin iterator of inputs vector
     * @param last  End iterator of inputs vector
     *
     * @since  1.0
     */
    virtual void on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;

    /**
     * @brief Called when parsing .outputs command
     *
     * This method is called when a \b .outputs command is parsed.
     * The outputs in a \b std::vector can be accessed via
     * their \p first and \p last iterator.
     *
     * @param first Begin iterator of outputs vector
     * @param last  End iterator of outputs vector
     *
     * @since  1.0
     */
    virtual void on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;

    /**
     * @brief Called when parsing the .constants command
     *
     * This method is called when a \b .constants command is parsed.
     * The constant in a \b std::vector can be accessed via
     * their \p first and \p last iterator. A optional boolean value
     * indicates whether the line is constant or not and in case it
     * is constant, which constant line is represented.
     *
     * @param first Begin iterator of constants vector
     * @param last  End iterator of constants vector
     *
     * @since  1.0
     */
    virtual void on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const;

    /**
     * @brief Called when parsing .garbage command
     *
     * This method is called when a \b .garbage command is parsed.
     * The garbage in a \b std::vector can be accessed via
     * their \p first and \p last iterator. A boolean value
     * indicates whether the line has a garbage output or not.
     *
     * @param first Begin iterator of garbage vector
     * @param last  End iterator of garbage vector
     *
     * @since  1.0
     */
    virtual void on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const;

    /**
     * @brief Called when parsing .inputbus command
     *
     * This method is called when a \b .inputbus command is parsed.
     * The first parameter \p name is the name of the bus,
     * whereas the corresponding variables can be accessed via the
     * second parameter \p line_indices, which already contains
     * the indices rather than the variable names.
     *
     * @param name Name of the bus
     * @param line_indices Line indices of the variables of the bus
     *
     * @since  1.1
     */
    virtual void on_inputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const;

    /**
     * @brief Called when parsing .outputbus command
     *
     * This method is called when a \b .outputbus command is parsed.
     * The first parameter \p name is the name of the bus,
     * whereas the corresponding variables can be accessed via the
     * second parameter \p line_indices, which already contains
     * the indices rather than the variable names.
     *
     * @param name Name of the bus
     * @param line_indices Line indices of the variables of the bus
     *
     * @since  1.1
     */
    virtual void on_outputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const;

    /**
     * @brief Called when parsing .state command
     *
     * This method is called when a \b .state command is parsed.
     * The first parameter \p name is the name of the state (bus),
     * whereas the corresponding variables can be accessed via the
     * second parameter \p line_indices, which already contains
     * the indices rather than the variable names.
     *
     * A .state command is modeled like a bus since it can contain more
     * than one line index. Even if the state contains only one signal,
     * this pattern is used.
     *
     * @param name Name of the state (bus)
     * @param line_indices Line indices of the variables of the state (bus)
     * @param initial_value Initial value for the state signal
     *
     * @since  1.1
     */
    virtual void on_state( const std::string& name, const std::vector<unsigned>& line_indices, unsigned initial_value ) const;

    /**
     * @brief Called when parsing .module command
     *
     * This method is called when a \b .module command is parsed.
     *
     * @param name Name of the module (used later to add gates)
     * @param filename File-name of the module circuit
     *
     * @since  1.1
     */
    virtual void on_module( const std::string& name, const boost::optional<std::string>& filename ) const;

    /**
     * @brief Called when parsing .begin command
     *
     * This method is called when a \b .begin command is parsed.
     *
     * @since  1.0
     */
    virtual void on_begin() const;

    /**
     * @brief Called when parsing .end command
     *
     * This method is called when a \b .end command is parsed.
     *
     * @since  1.0
     */
    virtual void on_end() const;

    /**
     * @brief Called when a gate is parsed
     *
     * This method is called when a gate is parsed
     * providing the gate type in parameter \p target_type
     * and all connected line via their zero-index indices.
     *
     * The last indices in the list \p line_indices are the
     * target lines. The number of target lines depends on
     * the corresponding gate type.
     *
     * @param target_type Target type of the target line(s).
     * @param line_indices All connected lines, both control
     *                     and target lines.
     *
     * @since  2.0
     */
    virtual void on_gate( const boost::any& target_type, const std::vector<variable>& line_indices ) const;

    /**
     * @brief Called when a truth table line is parsed
     *
     * This method is called when a truth table line is parsed.
     * A truth table line is parsed in a specification file.
     *
     * The truth table line is given as first and last
     * iterator over boost::optional<bool> values and as additional
     * parameter the corresponding truth table line is given.
     *
     * @param line_index The corresponding truth table line
     * @param first      Begin iterator over truth table output value
     * @param last       End iterator over truth table output value
     *
     * @since  1.0
     */
    virtual void on_truth_table_line( unsigned line_index, const std::vector<boost::optional<bool> >::const_iterator first, const std::vector<boost::optional<bool> >::const_iterator last ) const;

    /**
     * @brief Adds an annotation to the processor
     *
     * The parser adds all annotations in each step, i.e. every
     * time it parses a line. Old annotations are removed before.
     * The annotations can be read with current_annotation() when
     * processing an action.
     *
     * @param key Key of the annotation
     * @param value Value of the annotation
     *
     * @since  1.1
     */
    void add_annotation( const std::string& key, const std::string& value );

    /**
     * @brief Clears the current annotations
     *
     * This method clears the current annotations, i.e. when a new
     * line is processed by the parser.
     *
     * @since  1.1
     */
    void clear_annotations();

    /**
     * @brief Returns the current annotations
     *
     * @return Properties structure with annotations
     *
     * @since  1.1
     */
    properties::ptr current_annotations() const;

  private:
    class priv;
    priv* const d;
  };

}

#endif /* REVLIB_PROCESSOR_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
