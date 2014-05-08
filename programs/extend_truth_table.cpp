#include <iostream>

#include <core/truth_table.hpp>
#include <core/functions/extend_pla.hpp>
#include <core/io/read_pla.hpp>
#include <core/io/read_pla_to_bdd.hpp>

using namespace revkit;

int main( int argc, char ** argv )
{
  // usage
  if ( argc != 2 )
  {
    std::cout << "usage: " << argv[0] << " filename" << std::endl;
    return 1;
  }

  binary_truth_table pla, extended;
  read_pla_settings settings;
  settings.extend = false;
  read_pla( pla, argv[1], settings );

  std::cout << pla << std::endl;

  extend_pla( pla, extended );

  std::cout << "Extended PLA:" << std::endl;
  std::cout << extended << std::endl;

  BDDTable base;
  read_pla_to_bdd( base, argv[1] );

  return 0;
}
