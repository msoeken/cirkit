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

#include "approximate_additional_lines.hpp"

#include <fstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/lexical_cast.hpp>

#include <core/utils/timer.hpp>
#include <reversible/synthesis/synthesis_utils_p.hpp>

#include <cuddObj.hh>
#include <gmpxx.h>

namespace cirkit
{

unsigned parse_number_from_pla_command(const std::string& line)
{
  using boost::algorithm::split;
  using boost::algorithm::is_any_of;
  using boost::algorithm::token_compress_on;
  using boost::lexical_cast;

  std::vector<std::string> result;
  split(result, line, is_any_of("\t "), token_compress_on);

  return lexical_cast<unsigned>(result.at(1u));
}

BDD create_bdd_from_incube(Cudd& mgr, const std::string& incube, const std::vector<BDD>& variables)
{
  BDD cube = mgr.bddOne();

  for (unsigned i = 0u; i < incube.size(); ++i)
  {
    switch (incube.at(i)) {
    case '0':
      cube &= !variables.at(i);
      break;
    case '1':
      cube &= variables.at(i);
      break;
    }
  }

  return cube;
}

unsigned approximate_additional_lines( const std::string& filename, properties::ptr settings, properties::ptr statistics )
{
  /* Timer */
  properties_timer t( statistics );

  /* Number of inputs and outputs */
  unsigned n = 0u;
  unsigned m = 0u;

  /* Parse PLA */
  std::ifstream is;
  is.open(filename.c_str());

  std::string line;

  /* For the BDD */
  Cudd mgr(0, 0);
  BDD u = mgr.bddZero();
  std::vector<BDD> variables;

  /* MU hash table */
  std::map<std::string, mpz_class> mu;

  while(std::getline(is, line)) {
    using boost::algorithm::starts_with;
    using boost::algorithm::trim;

    trim(line);
    if (line.size() == 0u) continue;

    if (starts_with(line, ".i ")) {
      n = parse_number_from_pla_command(line);
      variables.resize(n);
      boost::generate(variables, [&mgr](){ return mgr.bddVar(); });
    } else if (starts_with(line, ".o ")) {
      m = parse_number_from_pla_command(line);
    } else if (starts_with(line, "#")) {
      /* ignore comments */
    } else if (!starts_with(line, ".")) {
      using boost::algorithm::split;
      using boost::algorithm::is_any_of;
      using boost::algorithm::token_compress_on;

      /* Split cubes */
      std::vector<std::string> split_result;
      split(split_result, line, is_any_of("\t "), token_compress_on);

      /* Assign cubes to local variables */
      const std::string& incube = split_result.at(0u);
      std::string outcube = split_result.at(1u);
      boost::replace_all( outcube, "-", "0" );

      /* Calculate number of patterns */
      BDD cube = create_bdd_from_incube(mgr, incube, variables);

      mpz_class patterns(cube.CountMinterm(n));

      /* Update entry in mu */
      if (mu.find(outcube) == mu.end()) {
        mu[outcube] = patterns;
      } else {
        mu[outcube] += patterns;
      }

      /* Update used cubes BDD */
      u |= cube;
    }
  }

  /* Cubes that map to zero */
  std::string zerocube = std::string(m, '0');
  mpz_class zeromu = pow2(n) - mpz_class(u.CountMinterm(n));
  /* It could be that there has been a mapping to 0 in the PLA, just in case */
  if (mu.find(zerocube) == mu.end()) {
    mu[zerocube] = zeromu;
  } else {
    mu[zerocube] += zeromu;
  }

  is.close();

  /* Maximum MU */
  using boost::adaptors::map_values;
  for ( auto v : mu )
  {
#ifdef DEBUG
    std::cout << v.first << " has " << v.second << std::endl;
#endif
  }
  mpz_class maxmu = *boost::max_element(mu | map_values);

  /* Statistics */
  if ( statistics )
  {
    statistics->set( "num_inputs", n );
    statistics->set( "num_outputs", m );
  }

  return calculate_required_lines(n, m, maxmu) - n;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
