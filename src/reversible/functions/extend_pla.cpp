/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include "extend_pla.hpp"

#include "../io/read_pla.hpp"

#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <cuddObj.hh>

using namespace boost::assign;

namespace cirkit
{

void extend_pla( binary_truth_table& base, binary_truth_table& extended, const extend_pla_settings& settings )
{
  // copy metadata
  extended.set_inputs( base.inputs() );
  extended.set_outputs( base.outputs() );

  // CUDD stuff
  Cudd mgr( 0, 0 );
  std::vector<BDD> vars( base.num_inputs() );
  boost::generate( vars, [&mgr]() { return mgr.bddVar(); } );

  // A function to create a BDD from a cube
  auto bddFromCube = [&mgr, &vars]( const std::pair<binary_truth_table::cube_type::const_iterator, binary_truth_table::cube_type::const_iterator>& cube ) {
    BDD c = mgr.bddOne();
    unsigned pos = 0u;
    for ( const auto& literal : boost::make_iterator_range( cube.first, cube.second ) ) {
      if ( literal ) c &= ( *literal ? vars.at( pos ) : !vars.at( pos ) );
      ++pos;
    }
    return c;
  };

  // A function to get truth table cubes from a BDD
  auto cubesFromBdd = [&mgr, &vars]( BDD& from, std::vector<binary_truth_table::cube_type>& cubes ) {
    char * cube = new char[vars.size()];
    unsigned pos;
    while ( from.CountMinterm( vars.size() ) > 0.0 ) {
      from.PickOneCube( cube );
      binary_truth_table::cube_type tcube;
      BDD bcube = mgr.bddOne();
      for ( pos = 0u; pos < vars.size(); ++pos ) {
        switch ( cube[pos] ) {
        case 0:
          bcube &= !vars.at( pos );
          tcube += false;
          break;
        case 1:
          bcube &= vars.at( pos );
          tcube += true;
          break;
        case 2:
          tcube += boost::optional<bool>();
          break;
        }
      }
      cubes += tcube;
      from &= !bcube;
    }

    delete[] cube;
  };

  while ( std::distance( base.begin(), base.end() ) > 0 )
  {
    // Pick one cube from base
    binary_truth_table::iterator base_cube = base.begin();
    binary_truth_table::cube_type base_in( base_cube->first.first, base_cube->first.second );
    binary_truth_table::cube_type base_out( base_cube->second.first, base_cube->second.second );

    if ( settings.verbose )
    {
      std::cout << "[I] Processing:" << std::endl;
      std::cout << "[I] ";
      std::for_each( base_cube->first.first, base_cube->first.second, []( const boost::optional<bool>& b ) { std::cout << (b ? (*b ? "1" : "0") : "-"); } );
      std::cout << " ";
      std::for_each( base_cube->second.first, base_cube->second.second, []( const boost::optional<bool>& b ) { std::cout << (b ? (*b ? "1" : "0") : "-"); } );
      std::cout << std::endl;
    }

    BDD bicube = bddFromCube( base_cube->first );
    base.remove_entry( base_cube );

    // Go through all cubes of extended
    bool found_match = false;
    for ( binary_truth_table::iterator extended_cube = extended.begin(); extended_cube != extended.end(); ++extended_cube )
    {
      BDD bocube = bddFromCube( extended_cube->first );

      if ( ( bicube & bocube ).CountMinterm( base.num_inputs() ) > 0.0 )
      {
        if ( settings.verbose )
        {
          std::cout << "[I] Intersection detected with" << std::endl;
          std::cout << "[I] ";
          std::for_each( extended_cube->first.first, extended_cube->first.second, []( const boost::optional<bool>& b ) { std::cout << (b ? (*b ? "1" : "0") : "-"); } );
          std::cout << " ";
          std::for_each( extended_cube->second.first, extended_cube->second.second, []( const boost::optional<bool>& b ) { std::cout << (b ? (*b ? "1" : "0") : "-"); } );
          std::cout << std::endl;
        }
        binary_truth_table::cube_type extended_in( extended_cube->first.first, extended_cube->first.second );
        binary_truth_table::cube_type extended_out( extended_cube->second.first, extended_cube->second.second );

        extended.remove_entry( extended_cube );

        BDD keep_in_base = bicube & !bocube;
        BDD intersection = bicube & bocube;
        BDD keep_in_extended = !bicube & bocube;

        std::vector<binary_truth_table::cube_type> cubes;

        cubesFromBdd( keep_in_base, cubes );
        for ( const auto& cube : cubes )
        {
          bool match = false;
          for ( binary_truth_table::iterator base_inner_cube = base.begin(); base_inner_cube != base.end(); ++base_inner_cube )
          {
            binary_truth_table::cube_type base_inner_in( base_inner_cube->first.first, base_inner_cube->first.second );
            binary_truth_table::cube_type base_inner_out( base_inner_cube->second.first, base_inner_cube->second.second );

            if ( cube == base_inner_in )
            {
              base.remove_entry( base_inner_cube );
              base.add_entry( cube, combine_pla_cube( base_out, base_inner_out ) );
              match = true;
              break;
            }
          }

          if ( !match )
          {
            base.add_entry( cube, base_out );
          }
        }
        cubes.clear();

        cubesFromBdd( intersection, cubes );
        for ( const auto& cube : cubes ) extended.add_entry( cube, combine_pla_cube( base_out, extended_out ) );
        cubes.clear();

        cubesFromBdd( keep_in_extended, cubes );
        for ( const auto& cube : cubes ) extended.add_entry( cube, extended_out );

        found_match = true;
        break;
      }
    }

    // Copy the base_cube if no match has been found
    if ( !found_match )
    {
      if ( settings.verbose )
      {
        std::cout << "[I] Add directly!" << std::endl;
      }
      extended.add_entry( base_in, base_out );
    }

    if ( settings.verbose )
    {
      std::cout << "[I] base:" << std::endl;
      std::cout << base << std::endl;
      std::cout << "[I] extended:" << std::endl;
      std::cout << extended << std::endl << std::endl;
    }
  }

  /* Compact */
  if ( settings.post_compact ) {
    // Compute compacted nouns
    std::map<binary_truth_table::cube_type, BDD> compacted_monoms;
    for ( const auto& row : extended ) {
      BDD in_cube = bddFromCube( row.first );
      binary_truth_table::cube_type out_pattern( row.second.first, row.second.second );

      auto it = compacted_monoms.find( out_pattern );
      if ( it == compacted_monoms.end() ) {
        compacted_monoms[out_pattern] = in_cube;
      } else {
        it->second |= in_cube;
      }
    }

    // Clear extended PLA representation
    extended.clear();

    // Add compacted monoms back to PLA representation
    for ( const auto& p : compacted_monoms ) {
      std::vector<binary_truth_table::cube_type> cubes;

      BDD bdd = p.second;
      cubesFromBdd( bdd, cubes );
      for ( const auto& cube : cubes ) extended.add_entry( cube, p.first );
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
