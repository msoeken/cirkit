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

#include "stores.hpp"

#include <cstdio>
#include <functional>
#include <iostream>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <core/io/read_pla.hpp>
#include <core/io/write_pla.hpp>
#include <core/utils/range_utils.hpp>

namespace alice
{

using namespace cirkit;

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

template<>
std::string store_entry_to_string<bdd_function_t>( const bdd_function_t& bdd )
{
  return ( boost::format( "%d variables, %d functions, %d nodes" ) % bdd.first.ReadSize() % bdd.second.size() % bdd.first.ReadKeys() ).str();
}

template<>
void print_store_entry<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd )
{
  for ( const auto& f : index( bdd.second ) )
  {
    os << "Function " << f.index << std::endl;
    f.value.PrintMinterm();
    os << std::endl;
  }
}

show_store_entry<bdd_function_t>::show_store_entry( command& cmd )
{
  boost::program_options::options_description bdd_options( "BDD options" );

  bdd_options.add_options()
    ( "add", "Convert BDD to ADD to have no complemented edges" )
    ;

  cmd.opts.add( bdd_options );
}

bool show_store_entry<bdd_function_t>::operator()( bdd_function_t& bdd,
                                                   const std::string& dotname,
                                                   const command& cmd )
{
  using namespace std::placeholders;

  auto * fd = fopen( dotname.c_str(), "w" );

  if ( cmd.is_set( "add" ) )
  {
    std::vector<ADD> adds( bdd.second.size() );
    boost::transform( bdd.second, adds.begin(), std::bind( &BDD::Add, _1 ) );
    bdd.first.DumpDot( adds, 0, 0, fd );
  }
  else
  {
    bdd.first.DumpDot( bdd.second, 0, 0, fd );
  }

  fclose( fd );

  return true;
}

command::log_opt_t show_store_entry<bdd_function_t>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd )
{
  std::vector<double> minterms;

  for ( const auto& f : bdd.second )
  {
    minterms.push_back( f.CountMinterm( bdd.first.ReadSize() ) );
  }

  os << "[i] no. of variables: " << bdd.first.ReadSize() << std::endl
     << "[i] no. of nodes:     " << bdd.first.ReadKeys() << std::endl
     << "[i] no. of minterms:  " << any_join( minterms, " " ) << std::endl
     << "[i] level sizes:      " << any_join( level_sizes( bdd.first, bdd.second ), " " ) << std::endl
     << "[i] maximum fanout:   " << maximum_fanout( bdd.first, bdd.second ) << std::endl
     << "[i] complement edges: " << count_complement_edges( bdd.first, bdd.second ) << std::endl;


  for ( const auto& p : index( bdd.second ) )
  {
    os << "[i] info for output " << p.index << ":" << std::endl
       << "[i] - path count:               " << p.value.CountPath() << std::endl
       << "[i] - path count (to non-zero): " << Cudd_CountPathsToNonZero( p.value.getNode() ) << std::endl;
  }

  bdd.first.info();
}

template<>
command::log_opt_t log_store_entry_statistics<bdd_function_t>( const bdd_function_t& bdd )
{
  return command::log_opt_t({
      {"inputs", bdd.first.ReadSize()},
      {"outputs", static_cast<unsigned>( bdd.second.size() )}
    });
}

template<>
bdd_function_t store_read_io_type<bdd_function_t, io_pla_tag_t>( const std::string& filename, const command& cmd )
{
  return read_pla( filename );
}

template<>
void store_write_io_type<bdd_function_t, io_pla_tag_t>( const bdd_function_t& bdd, const std::string& filename, const command& cmd )
{
  write_pla( bdd, filename );
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
