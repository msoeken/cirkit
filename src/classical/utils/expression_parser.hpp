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
 * @file expression_parser.hpp
 *
 * @brief Expression parser
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef EXPRESSION_PARSER_HPP
#define EXPRESSION_PARSER_HPP

#include <cassert>
#include <deque>
#include <iostream>
#include <memory>
#include <string>

#include <core/utils/bdd_utils.hpp>

namespace cirkit
{

struct expression_t
{
  using ptr = std::shared_ptr<expression_t>;
  enum type_t { _const, _var, _inv, _and, _or, _maj, _xor };

  type_t          type;
  unsigned        value;
  std::deque<ptr> children;
};

expression_t::ptr parse_expression( const std::string& expression );
std::ostream& operator<<( std::ostream& os, const expression_t::ptr& expr );
std::string expression_to_string( const expression_t::ptr& expr );
std::string expression_to_shape( const expression_t::ptr& expr, bool with_inverters = false );

/******************************************************************************
 * Evaluate expressions                                                       *
 ******************************************************************************/

template<typename T>
class expression_evaluator
{
public:
  virtual T on_const( bool value ) const = 0;
  virtual T on_var( unsigned index ) const = 0;
  virtual T on_inv( const T& value ) const = 0;
  virtual T on_and( const T& value1, const T& value2 ) const = 0;
  virtual T on_or( const T& value1, const T& value2 ) const = 0;
  virtual T on_maj( const T& value1, const T& value2, const T& value3 ) const = 0;
  virtual T on_xor( const T& value1, const T& value2 ) const = 0;
};

template<typename T>
T evaluate_expression( const expression_t::ptr& expr, const expression_evaluator<T>& eval )
{
  switch ( expr->type )
  {
  case expression_t::_const:
    return eval.on_const( expr->value == 1 );

  case expression_t::_var:
    return eval.on_var( expr->value );

  case expression_t::_inv:
    return eval.on_inv( evaluate_expression( expr->children.front(), eval ) );

  case expression_t::_and:
    {
      auto it = expr->children.begin();
      T value1 = evaluate_expression( *it++, eval );
      T value2 = evaluate_expression( *it,   eval );
      return eval.on_and( value1, value2 );
    }

  case expression_t::_or:
    {
      auto it = expr->children.begin();
      T value1 = evaluate_expression( *it++, eval );
      T value2 = evaluate_expression( *it,   eval );
      return eval.on_or( value1, value2 );
    }

  case expression_t::_maj:
    {
      auto it = expr->children.begin();
      T value1 = evaluate_expression( *it++, eval );
      T value2 = evaluate_expression( *it++, eval );
      T value3 = evaluate_expression( *it,   eval );
      return eval.on_maj( value1, value2, value3 );
    }

  case expression_t::_xor:
    {
      auto it = expr->children.begin();
      T value1 = evaluate_expression( *it++, eval );
      T value2 = evaluate_expression( *it,   eval );
      return eval.on_xor( value1, value2 );
    }

  default:
    assert( false );
  }
}

bdd_function_t bdd_from_expression( Cudd& manager, const expression_t::ptr& expr );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
