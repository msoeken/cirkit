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

#include "write_specification.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/spirit/include/karma.hpp>

#include <core/version.hpp>

#include "io_utils_p.hpp"

using namespace boost::assign;

namespace cirkit
{

  struct tristate_to_char
  {
    char operator()( const binary_truth_table::value_type& vt ) const
    {
      return vt ? ( *vt ? '1' : '0' ) : '-';
    }
  };

  struct garbage_to_char
  {
    char operator()( bool g )
    {
      return g ? '1' : '-';
    }
  };

  write_specification_settings::write_specification_settings()
    : version( "2.0" ),
      header( boost::str( boost::format( "This file has been generated using RevKit %s (www.revkit.org)" ) % cirkit_version() ) )
  {
  };

  bool write_specification( const binary_truth_table& spec, const std::string& filename, const write_specification_settings& settings )
  {
    std::filebuf fb;
    fb.open( filename.c_str(), std::ios::out );

    std::ostream os( &fb );

    if ( spec.num_inputs() < spec.num_outputs() )
    {
      return false;
    }

    unsigned oldsize = 0;

    if ( settings.header.size() )
    {
      std::string header = settings.header;
      boost::algorithm::replace_all( header, "\n", "\n# " );
      os << "# " << header << std::endl;
    }

    os << ".version " << settings.version << std::endl
       << ".numvars " << spec.num_inputs() << std::endl;

    std::vector<std::string> variables( spec.num_inputs() );

    for ( unsigned i = 0u; i < spec.num_inputs(); ++i )
    {
      variables[i] = boost::str( boost::format( "x%d" ) % i );
    }

    std::vector<std::string> _inputs = spec.inputs();
    oldsize = _inputs.size();
    _inputs.resize( spec.num_inputs() );

    for ( unsigned i = oldsize; i < spec.num_inputs(); ++i )
    {
      _inputs[i] = boost::str( boost::format( "i%d" ) % i );
    }

    std::vector<std::string> _outputs = spec.outputs();
    oldsize = _outputs.size();
    _outputs.resize( spec.num_inputs() );

    for ( unsigned i = oldsize; i < spec.num_inputs(); ++i )
    {
      _outputs[i] = boost::str( boost::format( "o%d" ) % i );
    }

    os << ".variables " << boost::algorithm::join( variables, " " ) << std::endl;

    namespace karma = boost::spirit::karma;
    namespace ascii = boost::spirit::ascii;

    os << ".inputs ";
    std::ostream_iterator<char> outit( os );
    karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _inputs );
    os << std::endl;

    os << ".outputs ";
    karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _outputs );
    os << std::endl;

    std::string _constants( spec.num_inputs(), '-' );
    std::transform( spec.constants().begin(), spec.constants().end(), _constants.begin(), tristate_to_char() );

    std::string _garbage( spec.num_inputs(), '-' );
    std::vector<bool> garbage = spec.garbage();
    std::transform( garbage.begin(), garbage.end(), _garbage.begin(), garbage_to_char() );

    os << ".constants " << _constants << std::endl
       << ".garbage " << _garbage << std::endl
       << ".begin" << std::endl;

    using table_type = std::map<unsigned, std::pair<binary_truth_table::out_const_iterator, binary_truth_table::out_const_iterator> >;
    table_type table;

    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      std::vector<unsigned> numbers;

      in_cube_to_values( it->first.first, it->first.second, std::back_inserter( numbers ) );

      for ( const auto& number : numbers )
      {
        table.insert( std::make_pair( number, it->second ) );
      }
    }

    table_type::const_iterator itTable = table.begin();
    unsigned position = 0;

    /* output permutation */
    std::vector<unsigned> output_order = settings.output_order;
    if ( output_order.size() != spec.num_outputs() )
    {
      output_order.clear();
      std::copy( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( spec.num_outputs() ), std::back_inserter( output_order ) );
    }

    do
    {
      // fill free spaces
      unsigned to = ( itTable == table.end() ) ? ( 1u << spec.num_inputs() ) : itTable->first;

      for ( unsigned i = position; i < to; ++i )
      {
        os << std::string( spec.num_inputs(), '-' ) << std::endl;
      }

      // break if done
      if ( itTable == table.end() )
      {
        break;
      }

      // now the actual line
      std::string outLine( spec.num_inputs(), '-' );
      for ( binary_truth_table::out_const_iterator itOut = itTable->second.first; itOut != itTable->second.second; ++itOut )
      {
        outLine.at( output_order.at( itOut - itTable->second.first ) ) = tristate_to_char()( *itOut );
      }

      os << outLine << std::endl;

      position = to + 1;
      ++itTable;

    } while ( true );

    os << ".end" << std::endl;

    fb.close();

    return true;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
