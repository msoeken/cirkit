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
#include "write_pla_from_cirkit_bdd.hpp"

#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/timer.hpp>

#include <fstream>
#include <iostream>

#include <classical/dd/visit_solutions.hpp>

namespace cirkit
{

void write_pla_from_cirkit_bdd (const std::vector<bdd> &fvec,
				const std::vector<std::string> &input_labels,
				const std::vector<std::string> &output_labels,
				std::ostream &os) {

  os << ".i "<< input_labels.size() << std::endl;
  os << ".o "<< output_labels.size() << std::endl;
  os << ".ilb";
  for (auto i=0u; i < input_labels.size(); i++) {
    os << " "<<input_labels.at(i);
  }
  os << std::endl << ".ob";
  for (auto i=0u; i < output_labels.size(); i++) {
    os << " "<<output_labels.at(i);
  }
  os << std::endl;
  print_paths (fvec, os);
  os << ".e" << std::endl;
}


void write_pla_from_cirkit_bdd (const bdd &f,
				const std::vector<std::string> &input_labels,
				const std::vector<std::string> &output_labels,
				std::ostream &os) {
  os << ".i "<< input_labels.size() << std::endl;
  os << ".o "<< output_labels.size() << std::endl;
  os << ".ilb";
  for (auto i=0u; i < input_labels.size(); i++) {
    os << " "<<input_labels.at(i);
  }
  os << std::endl << ".ob";
  for (auto i=0u; i < output_labels.size(); i++) {
    os << " "<<output_labels.at(i);
  }
  os << std::endl;
  print_paths (f, os);
  os << ".e" << std::endl;
}


} // namespace cirkit

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
