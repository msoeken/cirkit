/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

/**
 * @author Mathias Soeken
 */

#include <fstream>

#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/circuit_to_truth_table.hpp>
#include <reversible/io/create_image.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/print_statistics.hpp>
#include <reversible/io/read_realization.hpp>
#include <reversible/io/write_blif.hpp>
#include <reversible/simulation/partial_simulation.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/utils/costs.hpp>
#include <reversible/utils/reversible_program_options.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string blifname;

  reversible_program_options opts;
  opts.add_read_realization_option();
  opts.add_options()
    ( "circuit,c",                         "Prints the circuit" )
    ( "truth_table,t",                     "Prints truth table" )
    ( "partial_truth_table,p",             "Prints partial truth table" )
    ( "statistics,s",                      "Prints circuit statistics" )
    ( "image,i",                           "Creates circuit image in LaTeX" )
    ( "rcbdd,r",                           "Prints rcbdd statistics" )
    ( "blifname",      value( &blifname ), "If given, then the circuit is written to a blif file" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  circuit circ;
  read_realization( circ, opts.read_realization_filename() );

  if ( opts.is_set( "circuit" ) )
  {
    std::cout << circ << std::endl;
  }

  if ( opts.is_set( "truth_table" ) )
  {
    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, simple_simulation_func() );
    std::cout << spec << std::endl;
  }

  if ( opts.is_set( "partial_truth_table" ) )
  {
    auto settings = std::make_shared<properties>();
    settings->set( "partial", true );
    binary_truth_table spec;
    circuit_to_truth_table( circ, spec, partial_simulation_func( settings ) );
    std::cout << spec << std::endl;
  }

  if ( opts.is_set( "statistics" ) )
  {
    print_statistics_settings settings;
    print_statistics( circ, -1.0, settings );
  }

  if ( opts.is_set( "image" ) )
  {
    create_tikz_settings settings;
    create_image( std::cout, circ, settings );
  }

  if ( opts.is_set( "rcbdd" ) )
  {
    rcbdd mgr;
    mgr.initialize_manager();
    mgr.create_variables( circ.lines() );
    BDD f = mgr.create_from_circuit( circ );

    std::cout << "RCBDD nodes:      " << f.nodeCount() << std::endl;
  }

  if ( opts.is_set( "blifname" ) )
  {
    std::ofstream os( blifname.c_str(), std::ofstream::out );
    write_blif( circ, os );
    os.close();
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
