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

#include "expression_parser.hpp"

#include <cassert>
#include <sstream>
#include <stack>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void push_expression( std::stack<expression_t::ptr>& stack, const expression_t::ptr& expr )
{
  if ( !stack.empty() && stack.top()->type == expression_t::_inv && stack.top()->children.empty() )
  {
    auto top = stack.top();
    stack.pop();
    top->children.push_back( expr );
    push_expression( stack, top );
  }
  else
  {
    stack.push( expr );
  }
}

void prepare_operation( std::stack<expression_t::ptr>& stack, expression_t::type_t type )
{
  auto expr = std::make_shared<expression_t>();
  expr->type = type;
  expr->value = 0u;
  stack.push( expr );
}

void push_operation( std::stack<expression_t::ptr>& stack, expression_t::type_t type, unsigned num_ops )
{
  std::deque<expression_t::ptr> children;

  for ( auto i = 0u; i < num_ops; ++i )
  {
    assert( !stack.empty() );
    children.push_front( stack.top() );
    stack.pop();
  }

  /* get placeholder */
  assert( !stack.empty() );
  auto top = stack.top();
  stack.pop();
  assert( top->children.empty() );
  assert( top->type == type );
  top->children = children;

  push_expression( stack, top );
}

std::ostream& print_operation( std::ostream& os, const expression_t::ptr& expr, char open, char closed )
{
  os << open;
  for ( const auto& c : expr->children )
  {
    os << c;
  }
  os << closed;
  return os;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

expression_t::ptr parse_expression( const std::string& expression )
{
  std::stack<expression_t::ptr> stack;

  for ( auto c : expression )
  {
    switch ( c )
    {
    case '!':
      {
        /* place an empty inverter on the stack */
        auto expr = std::make_shared<expression_t>();
        expr->type = expression_t::_inv;
        expr->value = 0u;
        stack.push( expr );
      } break;

    case '0':
    case '1':
      {
        auto expr = std::make_shared<expression_t>();
        expr->type = expression_t::_const;
        expr->value = ( c == '0' ) ? 0u : 1u;
        push_expression( stack, expr );
      } break;

    case '(':
      prepare_operation( stack, expression_t::_and );
      break;

    case '{':
      prepare_operation( stack, expression_t::_or );
      break;

    case '<':
      prepare_operation( stack, expression_t::_maj );
      break;

    case '[':
      prepare_operation( stack, expression_t::_xor );
      break;

    case ')':
      push_operation( stack, expression_t::_and, 2u );
      break;

    case '}':
      push_operation( stack, expression_t::_or, 2u );
      break;

    case '>':
      push_operation( stack, expression_t::_maj, 3u );
      break;

    case ']':
      push_operation( stack, expression_t::_xor, 2u );
      break;

    default:
      if ( c >= 'a' && c <= 'z' )
      {
        auto expr = std::make_shared<expression_t>();
        expr->type = expression_t::_var;
        expr->value = static_cast<unsigned>( c - 'a' );
        push_expression( stack, expr );
      }
      else
      {
        std::cout << "cannot parse " << c << std::endl;
        assert( false );
      }
      break;
    };
  }

  const auto result = stack.top();
  stack.pop();
  assert( stack.empty() );
  return result;
}

std::ostream& operator<<( std::ostream& os, const expression_t::ptr& expr )
{
  switch ( expr->type )
  {
  case expression_t::_const:
    os << expr->value;
    break;

  case expression_t::_var:
    os << static_cast<char>( 'a' + expr->value );
    break;

  case expression_t::_inv:
    os << '!' << expr->children.front();
    break;

  case expression_t::_and:
    print_operation( os, expr, '(', ')' );
    break;

  case expression_t::_or:
    print_operation( os, expr, '{', '}' );
    break;

  case expression_t::_maj:
    print_operation( os, expr, '<', '>' );
    break;

  case expression_t::_xor:
    print_operation( os, expr, '[', ']' );
    break;

  default:
    assert( false );
  }

  return os;
}

std::string expression_to_string( const expression_t::ptr& expr )
{
  std::stringstream s;
  s << expr;
  return s.str();
}

class expression_shape_evaluator : public expression_evaluator<std::string>
{
public:
  expression_shape_evaluator( bool with_inverters )
    : with_inverters( with_inverters )
  {
  }

  std::string on_const( bool value ) const
  {
    if ( with_inverters )
    {
      return value ? "1" : "0";
    }
    else
    {
      return "C";
    }
  }

  std::string on_var( unsigned index ) const
  {
    return "I";
  }

  std::string on_inv( const std::string& value ) const
  {
    return ( with_inverters ? "!" : "" ) + value;
  }

  std::string on_and( const std::string& value1, const std::string& value2 ) const
  {
    return "(" + value1 + value2 + ")";
  }

  std::string on_or( const std::string& value1, const std::string& value2 ) const
  {
    return "{" + value1 + value2 + "}";
  }

  std::string on_maj( const std::string& value1, const std::string& value2, const std::string& value3 ) const
  {
    return "<" + value1 + value2 + value3 + ">";
  }

  std::string on_xor( const std::string& value1, const std::string& value2 ) const
  {
    return "[" + value1 + value2 + "]";
  }

private:
  bool with_inverters;
};

std::string expression_to_shape( const expression_t::ptr& expr, bool with_inverters )
{
  return evaluate_expression( expr, expression_shape_evaluator( with_inverters ) );
}

class expression_bdd_evaluator : public expression_evaluator<BDD>
{
public:
  expression_bdd_evaluator( Cudd& manager )
    : manager( manager )
  {
  }

  BDD on_const( bool value ) const
  {
    return value ? manager.bddOne() : manager.bddZero();
  }

  BDD on_var( unsigned index ) const
  {
    if ( manager.ReadSize() < static_cast<int>( index ) )
    {
      for ( auto i = manager.ReadSize(); i <= static_cast<int>( index ); ++i )
      {
        manager.bddVar();
      }
    }

    return manager.bddVar( index );
  }

  BDD on_inv( const BDD& value ) const
  {
    return !value;
  }

  BDD on_and( const BDD& value1, const BDD& value2 ) const
  {
    return value1 & value2;
  }

  BDD on_or( const BDD& value1, const BDD& value2 ) const
  {
    return value1 | value2;
  }

  BDD on_maj( const BDD& value1, const BDD& value2, const BDD& value3 ) const
  {
    return ( value1 & value2) | ( value1 & value3 ) | ( value2 & value3 );
  }

  BDD on_xor( const BDD& value1, const BDD& value2 ) const
  {
    return value1 ^ value2;
  }

private:
  Cudd& manager;
};

bdd_function_t bdd_from_expression( Cudd& manager, const expression_t::ptr& expr )
{
  return {manager, {evaluate_expression( expr, expression_bdd_evaluator( manager ) )}};
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
