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
 * @file exorcism_minimization.hpp
 *
 * @brief ESOP minimization using EXORCISM-4
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef EXORCISM_MINIMIZATION_HPP
#define EXORCISM_MINIMIZATION_HPP

// TODO move common types in separate header
#include <classical/optimization/optimization.hpp>

#include <string>

#include <cudd.h>

#include <core/cube.hpp>
#include <core/properties.hpp>
#include <classical/abc/gia/gia.hpp>
#include <classical/aig.hpp>

namespace cirkit
{

/**
 * @brief ESOP minimization with EXORCISM-4
 *
 * The BDD will first be written to a PLA file.
 *
 * @author Mathias Soeken
 */
void exorcism_minimization( DdManager * cudd, DdNode * f,
                            const properties::ptr& settings = properties::ptr(),
                            const properties::ptr& statistics = properties::ptr() );

void exorcism_minimization( const cube_vec_t& cubes,
                            const properties::ptr& settings = properties::ptr(),
                            const properties::ptr& statistics = properties::ptr() );

/******************************************************************************
 * New exorcism optimization                                                  *
 ******************************************************************************/

enum class exorcism_script
{
  none,     /* no optimization */
  def,      /* default (original) exorcism script */
  def_wo4,  /* default without EXORLINK-4 */
  j2r       /* just two rounds */
};

std::istream& operator>>( std::istream& in, exorcism_script& script );
std::ostream& operator<<( std::ostream& out, const exorcism_script& script );

gia_graph::esop_ptr exorcism_minimization( const gia_graph::esop_ptr& esop, unsigned ninputs, unsigned noutputs,
                                           const properties::ptr& settings = properties::ptr(),
                                           const properties::ptr& statistics = properties::ptr() );

gia_graph::esop_ptr exorcism_minimization( const gia_graph& gia,
                                           const properties::ptr& settings = properties::ptr(),
                                           const properties::ptr& statistics = properties::ptr() );

void write_esop( const gia_graph::esop_ptr& esop, unsigned ninputs, unsigned noutputs, const std::string& filename );
void write_esop( const gia_graph::esop_ptr& esop, unsigned ninputs, unsigned noutputs, std::ostream& os );


dd_based_esop_optimization_func dd_based_exorcism_minimization_func( properties::ptr settings = std::make_shared<properties>(),
                                                                     properties::ptr statistics = std::make_shared<properties>() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
