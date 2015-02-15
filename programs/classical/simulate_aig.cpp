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
 * @author Heinz Riener
 */

#include <core/utils/program_options.hpp>
#include <core/utils/timer.hpp>
#include <classical/aig.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/utils/simulate_aig.hpp>

#include <boost/format.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  std::string assignment;
  unsigned method = 0u;

  program_options opts;
  opts.add_options()
    ( "filename",   value<std::string>( &filename ),                     "AIG filename (in ASCII AIGER format)" )
    ( "assignment", value<std::string>( &assignment ),                   "Assignment, e.g. \"x1=0 x2=1 x3=1 y=1010 z=01\"" )
    ( "method,m",   value<unsigned>( &method )->default_value( method ), "Simulation method (considered only when assignment is not set)\n0: truth table\n1: BDD" )
    ( "little-endian,l",                                                 "Bit endianness (default: big-endian)" )
    ;
  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  aig_graph aig;
  try
  {
    read_aiger( aig, filename );
  }
  catch (const char *msg)
  {
    std::cerr << msg << std::endl;
    return 2;
  }

  const bool little_endian = opts.is_set( "little-endian" );

  new_print_timer t;

  if ( assignment.empty() )
  {
    if ( method == 0u )
    {
      auto values = simulate_aig( aig, tt_simulator() );

      for ( const auto& f : values )
      {
        std::cout << boost::format( "[I] Simulation result for %d:" ) % aig_to_literal( aig, f.first ) << std::endl;
        std::cout << f.second << std::endl;
      }
    }
    else if ( method == 1u )
    {
      /* INFO: The bdd_simulator variable needs to be declared out of simulate_aig and
               also before the values map. It must be ensured that it is deleted after
               the values map or the BDDs in the values map are deleted, because they
               have a reference to the Cudd manager that is stored inside the bdd_simulator
               class. */
      bdd_simulator simulator;
      auto values = simulate_aig( aig, simulator );

      for ( const auto& f : values )
      {
        std::cout << boost::format( "[I] Simulation result for %d:" ) % aig_to_literal( aig, f.first ) << std::endl;
        f.second.PrintMinterm();
      }
    }
    else
    {
      std::cout << "[E] unknown simulation method" << std::endl;
    }
  }
  else
  {
    std::map<std::string, std::string> wassignment;
    std::vector<std::string> a;
    boost::split( a, assignment, boost::is_any_of( " " ), boost::token_compress_on );
    for ( const auto& p : a )
    {
      std::vector<std::string> vv;
      boost::split( vv, p, boost::is_any_of( "=" ) );
      assert( vv.size() == 2u );
      wassignment.insert( {vv[0u], vv[1u]} );
    }

    std::map<std::string, bool> massignment;
    for ( const auto& e : wassignment )
    {
      const unsigned size = e.second.size();
      if (size == 1u)
      {
        massignment.insert( {e.first, e.second[0u] == '1'} );
      }
      else
      {
        for ( unsigned u = 0u; u < size; ++u )
        {
          const std::string name = (boost::format("%s[%d]") % e.first % u).str();
          const bool value = (!little_endian ? e.second[size-u-1u] : e.second[u]) == '1';
          massignment.insert( {name, value} );
        }
      }
    }

    // for ( const auto& e : wassignment )
    // {
    //   std::cout << e.first << " " << e.second << '\n';
    // }

    // for ( const auto& e : massignment )
    // {
    //   std::cout << e.first << " " << e.second << '\n';
    // }

    auto values = simulate_aig( aig, simple_assignment_simulator( massignment ) );

    for ( const auto& o : boost::get_property( aig, boost::graph_name ).outputs )
    {
      std::cout << "[I] value for '" << o.second << "': " << values[o.first] << std::endl;
    }
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
