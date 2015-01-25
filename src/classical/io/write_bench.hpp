/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
