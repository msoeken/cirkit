/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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
