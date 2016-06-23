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

#include "transparent_arithmetic.hpp"

#include <chrono>
#include <cmath>
#include <random>
#include <sstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void generate_transparent_arithmetic_circuit( std::ostream& os,
                                              const properties::ptr& settings,
                                              const properties::ptr& statistics )
{
  /* settings */
  const auto seed            = get( settings, "seed",            static_cast<unsigned>( std::chrono::system_clock::now().time_since_epoch().count() ) );
  const auto bitwidth        = get( settings, "bitwidth",        8u );
  const auto min_words       = get( settings, "min_words",       3u );
  const auto max_words       = get( settings, "max_words",       10u );
  const auto max_fanout      = get( settings, "max_fanout",      4u );
  const auto max_rounds      = get( settings, "max_rounds",      4u );
  const auto operators       = get( settings, "operators",       std::vector<std::string>( {"+", "-", "*", "/"} ) );
  const auto mux_prob        = get( settings, "mux_prob",        40u );
  const auto new_ctrl_prob   = get( settings, "new_ctrl_prob",   30u );
  const auto word_pattern    = get( settings, "word_pattern",    std::string( "w%d" ) );
  const auto control_pattern = get( settings, "control_pattern", std::string( "c%d" ) );
  const auto module_name     = get( settings, "module_name",     std::string( "trans_arith" ) );

  /* timing */
  properties_timer t( statistics );

  /* random number generation */
  std::default_random_engine generator( seed );
  std::exponential_distribution<> num_fanouts( 1 );

  /* some helper functions */
  const auto get_fanout = [&generator, &num_fanouts, max_fanout]() {
    return std::min( static_cast<unsigned>( ceil( num_fanouts( generator ) ) ), max_fanout );
  };

  const auto dice = [&generator]( unsigned from, unsigned to ) {
    return std::uniform_int_distribution<unsigned>( from, to )( generator );
  };

  const auto pick_two_and_remove = [&generator, &dice]( std::vector<std::string>& words ) {
    const auto first_index = dice( 0u, words.size() - 1u );
    const auto first_word  = words[first_index];
    words.erase( words.begin() + first_index );

    const auto second_index = dice( 0u, words.size() - 1u );
    const auto second_word  = words[second_index];
    words.erase( words.begin() + second_index );

    return std::make_pair( first_word, second_word );
  };

  /* initialize */
  const auto num_words = dice( min_words, max_words );

  auto words = create_name_list( word_pattern, num_words, 1u );
  auto inputs = words;
  auto next_word_id = num_words + 1u;
  std::vector<std::string> new_words, wires, controls;

  /* helper function to create a new word */
  const auto new_word = [&new_words, word_pattern, &next_word_id]() {
    new_words.push_back( boost::str( boost::format( word_pattern ) % next_word_id++ ) );
    return new_words.back();
  };

  const auto get_control = [&controls, new_ctrl_prob, control_pattern, &dice]() {
    if ( controls.empty() || dice( 0u, 100u ) <= new_ctrl_prob )
    {
      std::string c = boost::str( boost::format( control_pattern ) % ( controls.size() + 1u ) );
      controls.push_back( c );
      return c;
    }
    else
    {
      return controls[dice( 0u, controls.size() - 1u )];
    }
  };

  /* create */
  std::stringstream assigns;
  auto round = 0u;
  while ( round++ < max_rounds && words.size() >= 2u )
  {
    boost::push_back( wires, new_words );
    new_words.clear();

    while ( words.size() >= 2u )
    {
      std::string w1, w2;
      std::tie( w1, w2 ) = pick_two_and_remove( words );

      const auto nw = new_word();

      /* mux or operator */
      if ( dice( 0u, 100u ) <= mux_prob )
      {
        const auto c = get_control();
        assigns << boost::format( "  assign %s = %s ? %s : %s;" ) % nw % c % w1 % w2 << std::endl;
      }
      else
      {
        const auto op = operators[dice( 0u, operators.size() - 1 )];
        assigns << boost::format( "  assign %s = %s %s %s;" ) % nw % w1 % op % w2 << std::endl;
      }

      /* fanout? */
      for ( auto i = 1u; i < get_fanout(); ++i )
      {
        const auto nw_copy = new_word();

        assigns << boost::format( "  assign %s = %s;" ) % nw_copy % nw << std::endl;
      }
    }

    boost::push_back( words, new_words );
  }

  for ( const auto& w : words )
  {
    const auto it = boost::find( wires, w );
    if ( it == wires.end() ) { continue; }
    wires.erase( it );
  }
  auto outputs = words;

  /* dump file */
  os << "// this file has been generated with CirKit using the command:" << std::endl
     << boost::format( "//   gen_trans_arith --seed %d --bitwidth %d --min_words %d --max_words %d --max_fanout %d --max_rounds %d --operators \"%s\" --mux_prob %d --new_ctrl_prob %d --word_pattern \"%s\" --control_pattern \"%s\" --module_name \"%s\"" )
        % seed
        % bitwidth
        % min_words
        % max_words
        % max_fanout
        % max_rounds
        % boost::join( operators, " " )
        % mux_prob
        % new_ctrl_prob
        % word_pattern
        % control_pattern
        % module_name
     << std::endl << std::endl
     << boost::format( "module %s( %s" ) % module_name % boost::join( inputs, ", " );

  if ( !controls.empty() )
  {
    os << boost::format( ", %s" ) % boost::join( controls, ", " );
  }

  os << boost::format( ", %s );" )  % boost::join( outputs, ", " ) << std::endl
     << boost::format( "  input [%d:0] %s;" ) % ( bitwidth - 1 ) % boost::join( inputs, ", " ) << std::endl;

  if ( !controls.empty() )
  {
    os << boost::format( "  input %s;" ) % boost::join( controls, ", " ) << std::endl;
  }
  os << boost::format( "  output [%d:0] %s;" ) % ( bitwidth - 1 ) % boost::join( outputs, ", " ) << std::endl
     << boost::format( "  wire [%d:0] %s;" ) % ( bitwidth - 1 ) % boost::join( wires, ", " ) << std::endl << std::endl
     << assigns.str()
     << "endmodule" << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
