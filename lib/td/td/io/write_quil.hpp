#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include <fmt/format.h>

#include "../networks/small_mct_circuit.hpp"

// TODO move to library
void write_quil( small_mct_circuit const& circ, std::ostream& out )
{
  circ.foreach_node( [&]( auto const& n ) {
    auto const& g = circ.get_gate( n );

    if ( circ.gate_type( g ) != gate_type_t::mct )
    {
      std::cerr << "[w] unsupported gate type\n";
      return true;
    }

    std::vector<uint32_t> controls, targets;

    circ.foreach_control( g, [&]( auto q ) { controls.push_back( q ); } );
    circ.foreach_target( g, [&]( auto q ) { targets.push_back( q ); } );

    switch ( controls.size() )
    {
      default:
        std::cerr << "[w] unsupported control size\n";
        return true;
      case 0u:
        for ( auto q : targets )
        {
          out << fmt::format( "X {}\n", q );
        }
        break;
      case 1u:
        for ( auto q : targets )
        {
          out << fmt::format( "CNOT {} {}\n", controls[0], q );
        }
        break;
      case 2u:
        for ( auto i = 1u; i < targets.size(); ++i )
        {
          out << fmt::format( "CNOT {} {}\n", targets[0], targets[i] );
        }
        out << fmt::format( "CCNOT {} {} {}\n", controls[0], controls[1], targets[0] );
        for ( auto i = 1u; i < targets.size(); ++i )
        {
          out << fmt::format( "CNOT {} {}\n", targets[0], targets[i] );
        }
        break;
    }

    return true;
  });
}

void write_quil( small_mct_circuit const& circ, const std::string& filename )
{
  std::ofstream out( filename.c_str(), std::ofstream::out );
  write_quil( circ, out );
}
