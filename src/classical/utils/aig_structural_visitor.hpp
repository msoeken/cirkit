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
 * @file aig_structural_visitor.hpp
 *
 * @brief AIG structural visitor.
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef AIG_STRUCTURAL_VISITOR_HPP
#define AIG_STRUCTURAL_VISITOR_HPP

#include <classical/aig.hpp>
#include <iostream>

namespace cirkit
{

class aig_structural_visitor
{
public:
  aig_structural_visitor( const aig_graph& aig )
    : aig( aig )
  {}

public:
  virtual void visit_constant( const aig_node& node ) const
  { }

  virtual void visit_input( const aig_node& node ) const
  { }

  virtual void visit_output( const aig_node& node ) const
  { }

  virtual void visit_latch( const aig_node& node ) const
  { }

  virtual void visit_and( const aig_node& node, const aig_function& left, const aig_function& right ) const
  { }

  const aig_graph& aig;
};

class aig_printing_visitor : public aig_structural_visitor
{
public:
  aig_printing_visitor( const aig_graph& aig )
    : aig_structural_visitor( aig )
  {}

public:
  void visit_constant( const aig_node& node ) const
  {
    std::cout << "[const]" << node << std::endl;
  }

  void visit_input( const aig_node& node ) const
  {
    std::cout << "[in] " << node << std::endl;
  }

  void visit_output( const aig_node& node ) const
  {
    std::cout << "[out] " << node << std::endl;
  }

  void visit_latch( const aig_node& node ) const
  {
    std::cout << "[lat] " << node << std::endl;
  }

  void visit_and( const aig_node& node, const aig_function& left, const aig_function& right ) const
  {
    std::cout << "[and] " << node << ' ' << left << ' ' << right << std::endl;
  }
};

void aig_visit_nodes( const aig_graph& aig, const aig_structural_visitor& vis );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
