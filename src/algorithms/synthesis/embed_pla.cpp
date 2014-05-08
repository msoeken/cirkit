/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "embed_pla.hpp"
#include "synthesis_utils_p.hpp"

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

#include <gmpxx.h>

#include <core/io/pla_parser.hpp>

namespace revkit
{

BDD create_bdd_from_incube(const rcbdd& cf, const std::string& incube, unsigned offset = 0u, std::vector<BDD>* dont_cares = 0)
{
  BDD cube = cf.manager().bddOne();

  for (unsigned i = 0u; i < incube.size(); ++i)
  {
    switch (incube.at(i))
    {
    case '0':
      cube &= !cf.x(offset + i);
      break;
    case '1':
      cube &= cf.x(offset + i);
      break;
    case '-':
      if (dont_cares)
      {
        *dont_cares += cf.x(offset + i);
      }
      break;
    }
  }

  return cube;
}

BDD create_bdd_from_outcube(const rcbdd& cf, const std::string& outcube)
{
  BDD cube = cf.manager().bddOne();

  for (unsigned i = 0u; i < outcube.size(); ++i)
  {
    switch (outcube.at(i))
    {
    case '1':
      cube &= cf.y(i);
      break;
    case '0':
    case '~':
    case '-':
      cube &= !cf.y(i);
      break;
    }
  }

  return cube;
}

unsigned calculate_required_lines(unsigned n, unsigned m, mpz_class maxmu)
{
  unsigned exp = 0u;

  while (pow2(exp) < maxmu) {
    ++exp;
  }

  return n > m + exp ? n : m + exp;
}

std::vector<BDD> _dec(const rcbdd& cf, const std::vector<BDD>& vars)
{
  std::vector<BDD> outputs;

  for (unsigned i = 0u; i < vars.size(); ++i)
  {
    BDD cube = cf.manager().bddOne();

    for (unsigned j = 0u; j < i; ++j)
    {
      cube &= !vars.at(j);
    }

    outputs += vars.at(i) ^ cube;
  }

  return outputs;
}

class embed_pla_processor : public pla_processor
{
public:
  embed_pla_processor( rcbdd& _cf )
    : cf( _cf ),
      n( 0u ), m( 0u ),
      variables_generated( false )
  {
    u = cf.manager().bddZero();
  }

  void on_num_inputs( unsigned num_inputs )
  {
    n = num_inputs;
  }

  void on_num_outputs( unsigned num_outputs )
  {
    m = num_outputs;
  }

  void on_cube( const std::string& in, const std::string& out )
  {
    if (!variables_generated)
    {
      cf.create_variables(n);
      variables_generated = false;
    }


    cubes += std::make_pair(in, out);

    /* Calculate number of patterns */
    BDD cube = create_bdd_from_incube(cf, in);

    mpz_class patterns(cube.CountMinterm(n));

    /* Update entry in mu */
    if (mu.find(out) == mu.end()) {
      mu[out] = patterns;
    } else {
      mu[out] += patterns;
    }

    /* Update used cubes BDD */
    u |= cube;
  }

public:
  rcbdd& cf;
  unsigned n, m;
  BDD u;
  std::map<std::string, mpz_class> mu;
  std::vector<std::pair<std::string, std::string> > cubes;

private:
  bool variables_generated;
};

bool embed_pla( rcbdd& cf, const std::string& filename,
                properties::ptr settings,
                properties::ptr statistics )
{
  /* Settings */
  bool verbose = get<bool>( settings, "verbose", false );

  /* BDD manager? */
  cf.initialize_manager();

  /* Parser */
  embed_pla_processor p( cf );
  pla_parser( filename, p );

  /* Cubes that map to zero */
  int *cube;
  CUDD_VALUE_TYPE value;
  DdGen *gen;

  BDD zeroCubes = !p.u;
  std::string zerocube = std::string(p.m, '0');
  if (p.mu.find(zerocube) == p.mu.end())
  {
    p.mu[zerocube] = 0;
  }

  Cudd_ForeachCube(zeroCubes.manager(), zeroCubes.getNode(), gen, cube, value)
  {
    std::string incube;
    char c;
    for (unsigned i = 0u; i < p.n; ++i)
    {
      c = cube[3u * i];
      switch (c)
      {
      case 0:
        incube += "0";
        break;
      case 1:
        incube += "1";
        break;
      case 2:
        incube += "-";
        break;
      }
    }

    p.mu[zerocube] += mpz_class(create_bdd_from_incube(cf, incube).CountMinterm(p.n));
    p.cubes += std::make_pair(incube, zerocube);
  }

  /* Maximum MU */
  using boost::adaptors::map_values;
  mpz_class maxmu = *boost::max_element(p.mu | map_values);
  unsigned req_vars = calculate_required_lines(p.n, p.m, maxmu);

  //std::cout << req_vars << std::endl;

  cf.create_variables(req_vars);

  /* Now create the BDD */
  BDD func = cf.manager().bddZero();
  p.mu.clear();
  p.u = cf.manager().bddZero();

  /* We store the garbage variables */
  std::vector<BDD> garbage;
  for (int i = req_vars - 1; i >= p.m; --i)
  {
    garbage += cf.y(i);
  }

  for (const auto& cube : p.cubes) {
    /* Assign cubes to local variables */
    const std::string& incube = cube.first;
    const std::string& outcube = cube.second;

    std::vector<BDD> dont_cares;
    BDD icube = create_bdd_from_incube(cf, incube, req_vars - p.n, &dont_cares);
    BDD ocube = create_bdd_from_outcube(cf, outcube);
    mpz_class patterns(icube.CountMinterm(p.n));

    /* TODO update func */
    BDD fcube = cf.manager().bddOne();
    for (unsigned i = 0u; i < req_vars - p.n; i++)
    {
      fcube &= !cf.x(i);
    }
    fcube &= icube & ocube;

    if (p.mu.find(outcube) == p.mu.end()) {
      p.mu[outcube] = 0u;
    }

    /*std::cout << "Incube: " << incube << std::endl;
    std::cout << "Outcube: " << outcube << std::endl;
    std::cout << "Patterns: " << patterns << std::endl;
    std::cout << "Number of don't cares: " << dont_cares.size() << std::endl;
    std::cout << "Number of garbage: " << (req_vars - m) << std::endl;
    std::cout << "mu[outcube]: " << mu[outcube] << std::endl;*/

    std::vector<BDD> dec_garbage = garbage;
    for (unsigned i = 0u; i < p.mu[outcube]; ++i)
    {
      dec_garbage = _dec(cf, dec_garbage);
    }

    /* Assign don't cares to dec_garbage (from back to front) */
    for (unsigned i = 0u; i < (req_vars - p.m); ++i)
    {
      if (i < dont_cares.size())
      {
        fcube &= dec_garbage.at(i).Xnor(dont_cares.at(dont_cares.size() - i - 1u));
      }
      else
      {
        fcube &= !dec_garbage.at(i);
      }
    }

    /* Update entry in mu */
    p.mu[outcube] += patterns;
    fcube &= !p.u;
    p.u |= icube;

    //fcube.PrintMinterm();

    /* Update func */
    func |= fcube;
  }

  if ( verbose )
  {
    func.PrintMinterm();
  }

  cf.set_chi( func );

  return false;

}


}

// Local Variables:
// c-basic-offset: 2
// End:
