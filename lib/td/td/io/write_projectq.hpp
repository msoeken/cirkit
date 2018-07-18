#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include <fmt/format.h>
#include <kitty/detail/mscfix.hpp>

#include "../networks/small_mct_circuit.hpp"

auto make_qubit_list( std::string& s )
{
  return [&]( auto c ) {
      if ( !s.empty() )
      {
        s += ", ";
      }
      s += fmt::format( "qs[{}]", c );
  };
}

// TODO move to library
void write_projectq( small_mct_circuit const& circ, std::ostream& out )
{
  circ.foreach_node( [&]( auto const& n ) {
    auto const& g = circ.get_gate( n );

    std::string controls, targets;

    circ.foreach_control( g, make_qubit_list( controls ) );
    circ.foreach_target( g, make_qubit_list( targets ) );

    char u;
    switch ( circ.gate_type( g ) )
    {
      default: assert( false ); break;
      case gate_type_t::mct: u = 'X'; break;
      case gate_type_t::mcy: u = 'Y'; break;
      case gate_type_t::mcz: u = 'Z'; break;
    }

    out << fmt::format( "C(All({}), {}) | ([{}], [{}])\n", u, circ.num_controls( g ), controls, targets );
  });
}

void write_projectq( small_mct_circuit const& circ, const std::string& filename )
{
  std::ofstream out( filename.c_str(), std::ofstream::out );
  write_projectq( circ, out );
}
