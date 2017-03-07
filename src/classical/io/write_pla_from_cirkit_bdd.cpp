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
