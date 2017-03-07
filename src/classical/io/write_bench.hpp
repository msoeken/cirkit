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
 * @file write_bench.hpp
 *
 * @brief Write AIG to bench format
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef WRITE_BENCH_HPP
#define WRITE_BENCH_HPP

#include <iostream>
#include <string>

#include <classical/aig.hpp>
#include <classical/lut/lut_graph.hpp>

namespace cirkit
{

struct write_bench_settings
{
  std::string prefix;
  bool write_input_declarations = true;
  bool write_output_declarations = true;
};

void write_bench( const aig_graph& aig, std::ostream& os, const write_bench_settings& settings = write_bench_settings() );
void write_bench( const aig_graph& aig, const std::string& filename, const write_bench_settings& settings = write_bench_settings() );

void write_bench( const lut_graph_t& lut, std::ostream& os, const write_bench_settings& settings = write_bench_settings() );
void write_bench( const lut_graph_t& lut, const std::string& filename, const write_bench_settings& settings = write_bench_settings() );

void write_bench( const lut_graph& graph, std::ostream& os, const write_bench_settings& settings = write_bench_settings() );
void write_bench( const lut_graph& graph, const std::string& filename, const write_bench_settings& settings = write_bench_settings() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
