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

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>

#include <gmpxx.h>

#include <core/io/pla_parser.hpp>

namespace cirkit
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
      //cube &= !cf.y(i);
      break;
    }
  }

  return cube;
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

void print_truth_table( const rcbdd& cf, const BDD& f )
{
  using boost::adaptors::transformed;

  DdGen *gen;
  int  *cube;
  CUDD_VALUE_TYPE value;

  unsigned vars = cf.num_vars();
  unsigned n = cf.num_inputs();
  unsigned m = cf.num_outputs();

  std::cout << std::string( vars - n, 'c' )
            << ' '
            << std::string( n, 'x' )
            << " | "
            << std::string( m, 'y' )
            << ' '
            << std::string( vars - m, 'g' )
            << std::endl;
  std::cout << std::string( vars + 2u, '-' ) << '+' << std::string( vars + 2u, '-' ) << std::endl;

  Cudd_ForeachCube( cf.manager().getManager(), f.getNode(), gen, cube, value )
  {
    std::cout << boost::join( boost::irange( 0u, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i], 1u ); } ), "" ).insert( vars - n, " " );
    std::cout << " | ";
    std::cout << boost::join( boost::irange( 0u, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i + 1u], 1u ); } ), "" ).insert( m, " " );
    std::cout << std::endl;
  }
}

void write_pla_to_file( const rcbdd& cf, const BDD& f, const std::string& filename )
{
  using boost::adaptors::transformed;

  DdGen *gen;
  int  *cube;
  CUDD_VALUE_TYPE value;

  unsigned vars = cf.num_vars();
  unsigned n = cf.num_inputs();
  unsigned m = cf.num_outputs();

  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );

  os << ".i " << n << std::endl
     << ".o " << m << std::endl
     << ".ilb " << boost::join( cf.input_labels(), " " ) << std::endl
     << ".ob " << boost::join( cf.output_labels(), " " ) << std::endl;

  Cudd_ForeachCube( cf.manager().getManager(), f.getNode(), gen, cube, value )
  {
    os << boost::join( boost::irange( vars - n, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i], 1u ); } ), "" )
       << " "
       << boost::join( boost::irange( 0u, m ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i + 1u], 1u ); } ), "" )
       << std::endl;
  }

  fb.close();
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
    cf.set_num_inputs( n = num_inputs );
  }

  void on_num_outputs( unsigned num_outputs )
  {
    cf.set_num_outputs( m = num_outputs );
  }

  void on_input_labels( const std::vector<std::string>& input_labels )
  {
    cf.set_input_labels( input_labels );
  }

  void on_output_labels( const std::vector<std::string>& output_labels )
  {
    cf.set_output_labels( output_labels );
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
  bool        verbose     = get( settings, "verbose",     false         );
  bool        truth_table = get( settings, "truth_table", false         ); /* prints the truth table (for debugging) */
  std::string write_pla   = get( settings, "write_pla",   std::string() );

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

    /* compute h */
    BDD constants = cf.manager().bddOne();
    for ( unsigned i = 0; i < req_vars - p.n; ++i ) { constants += cf.x( i ); }
    BDD h = cf.remove_ys( func ).ExistAbstract(constants);
    assert( !h & icube == icube );

    /* add new cubes */
    BDD fcube = cf.manager().bddOne();
    for (unsigned i = 0u; i < req_vars - p.n; i++)
    {
      fcube &= !cf.x(i);
    }
    fcube &= !h & icube & ocube;

    if (p.mu.find(outcube) == p.mu.end()) {
      p.mu[outcube] = 0u;
    }

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

    func |= fcube;
    p.mu[outcube] += patterns;

    /* Update existing cubes */
    assert( (h & icube) == cf.manager().bddZero() );
    func &= !(h & icube) | ocube;
  }

  /* Assign zeros */
  for ( unsigned i = 0; i < p.m; ++i )
  {
    BDD f0 = func.Cofactor( !cf.y( i ) );
    BDD f1 = func.Cofactor(  cf.y( i ) );

    func = ((f1 & !f0) & cf.y( i )) | ((f1 & f0) & !cf.y( i ));
  }

  if ( verbose )
  {
    func.PrintMinterm();
    std::cout << "|f|:   " << func.CountMinterm( 2 * cf.num_vars() ) << std::endl;
    std::cout << "|f_x|: " << cf.remove_ys( func ).CountMinterm( cf.num_vars() ) << std::endl;
    std::cout << "|f_y|: " << cf.remove_xs( func ).CountMinterm( cf.num_vars() ) << std::endl;
  }

  if ( truth_table )
  {
    print_truth_table( cf, func );
  }

  if ( write_pla.size() )
  {
    write_pla_to_file( cf, func, write_pla );
  }

  cf.set_chi( func );

  return false;

}


}

// Local Variables:
// c-basic-offset: 2
// End:
