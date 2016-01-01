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
 * @file write_pla_from_cirkit_bdd.hpp
 *
 * @brief Functions to write a pla file from a  cirkit BDD.
 *
 * @author Arun Chandrasekharan
 */

#pragma once

#ifndef WRITE_PLA_FROM_CIRKIT_BDD
#define WRITE_PLA_FROM_CIRKIT_BDD

#include <core/properties.hpp>
#include <classical/dd/bdd.hpp>
#include <classical/dd/visit_solutions.hpp>

#include <fstream>
#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace cirkit
{
void write_pla_from_cirkit_bdd (const std::vector<bdd> &fvec, 
				const std::vector<std::string> &input_labels, 
				const std::vector<std::string> &output_labels, 
				std::ostream &os);

void write_pla_from_cirkit_bdd (const bdd &f,
				const std::vector<std::string> &input_labels, 
				const std::vector<std::string> &output_labels, 
				std::ostream &os);

} // namespace cirkit

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:

