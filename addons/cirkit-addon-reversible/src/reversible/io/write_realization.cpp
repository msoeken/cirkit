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

#include "write_realization.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/spirit/include/karma.hpp>

#include <core/version.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/target_tags.hpp>

using namespace boost::assign;

namespace cirkit
{

  struct constant_to_char
  {
    char operator()( const constant& c )
    {
      return ( c ? ( *c ? '1' : '0' ) : '-' );
    }
  };

  struct garbage_to_char
  {
    char operator()( bool g )
    {
      return g ? '1' : '-';
    }
  };

  struct line_to_variable
  {
    std::string operator()( const variable& v ) const
    {
      return boost::str( boost::format( "%sx%d" ) % ( v.polarity() ? "" : "-" ) % v.line() );
    }

    std::string operator()( const unsigned& l ) const
    {
      return boost::str( boost::format( "x%d" ) % l );
    }
  };

  write_realization_settings::write_realization_settings()
    : header( boost::str( boost::format( "This file has been generated using RevKit %s (www.revkit.org)" ) % cirkit_version() ) )
  {
  }

  std::string write_realization_settings::type_label( const gate& g ) const
  {
    if ( is_toffoli( g ) )
    {
      return boost::str( boost::format( "t%d" ) % g.size() );
    }
    else if ( is_fredkin( g ) )
    {
      return boost::str( boost::format( "f%d" ) % g.size() );
    }
    else if ( is_peres( g ) )
    {
      return "p";
    }
    else if ( is_module( g ) )
    {
      return boost::any_cast<module_tag>( g.type() ).name;
    }
    else if ( is_stg( g ) )
    {
      const auto stg = boost::any_cast<stg_tag>( g.type() );
      return boost::str( boost::format( "stg%d[%s]" ) % g.size() % tt_to_hex( stg.function ) );
    }
    else
    {
      return "UNKNOWN";
    }
  }

  void write_realization( const circuit& circ, std::ostream& os, const write_realization_settings& settings )
  {
    unsigned oldsize = 0;

    if ( !settings.header.empty() )
    {
      std::string header = settings.header;
      boost::algorithm::replace_all( header, "\n", "\n# " );
      os << "# " << header << std::endl;
    }

    if ( !settings.version.empty() )
    {
      os << ".version " << settings.version << std::endl;
    }

    os << ".numvars " << circ.lines() << std::endl;

    std::vector<std::string> variables( circ.lines() );

    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      variables[i] = boost::str( boost::format( "x%d" ) % i );
    }

    std::vector<std::string> _inputs( circ.inputs().begin(), circ.inputs().end() );
    oldsize = _inputs.size();
    _inputs.resize( circ.lines() );

    for ( unsigned i = oldsize; i < circ.lines(); ++i )
    {
      _inputs[i] = boost::str( boost::format( "i%d" ) % i );
    }

    std::vector<std::string> _outputs( circ.outputs().begin(), circ.outputs().end() );
    oldsize = _outputs.size();
    _outputs.resize( circ.lines() );

    for ( unsigned i = oldsize; i < circ.lines(); ++i )
    {
      _outputs[i] = boost::str( boost::format( "o%d" ) % i );
    }

    os << ".variables " << boost::algorithm::join( variables, " " ) << std::endl;

    namespace karma = boost::spirit::karma;
    namespace ascii = boost::spirit::ascii;

    os << ".inputs";
    //std::ostream_iterator<char> outit( os );
    //karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _inputs );

    for ( const auto& _input : _inputs )
    {
      std::string quote = ( _input.find( " " ) != std::string::npos ) ? "\"" : "";
      os << boost::format( " %s%s%s" ) % quote % _input % quote;
    }

    os << std::endl;

    os << ".outputs";
    //karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _outputs );

    for ( const auto& _output : _outputs )
    {
      std::string quote = ( _output.find( " " ) != std::string::npos ) ? "\"" : "";
      os << boost::format( " %s%s%s" ) % quote % _output % quote;
    }

    os << std::endl;

    std::string _constants( circ.lines(), '-' );
    std::transform( circ.constants().begin(), circ.constants().end(), _constants.begin(), constant_to_char() );

    std::string _garbage( circ.lines(), '-' );
    std::transform( circ.garbage().begin(), circ.garbage().end(), _garbage.begin(), garbage_to_char() );

    os << ".constants " << _constants << std::endl
       << ".garbage " << _garbage << std::endl;

    for ( const auto& bus : circ.inputbuses().buses() )
    {
      std::vector<std::string> lines;
      std::transform( bus.second.begin(), bus.second.end(), std::back_inserter( lines ), line_to_variable() );
      os << ".inputbus " << bus.first << " " << boost::algorithm::join( lines, " " ) << std::endl;
    }

    for ( const auto& bus : circ.outputbuses().buses() )
    {
      std::vector<std::string> lines;
      std::transform( bus.second.begin(), bus.second.end(), std::back_inserter( lines ), line_to_variable() );
      os << ".outputbus " << bus.first << " " << boost::algorithm::join( lines, " " ) << std::endl;
    }

    for ( const auto& bus : circ.statesignals().buses() )
    {
      std::vector<std::string> lines;
      std::transform( bus.second.begin(), bus.second.end(), std::back_inserter( lines ), line_to_variable() );
      os << ".state " << bus.first << " " << boost::algorithm::join( lines, " " ) << std::endl;
    }

    for ( const auto& module : circ.modules() )
    {
      os << ".module " << module.first << std::endl;

      write_realization_settings module_settings;
      module_settings.version.clear();
      module_settings.header.clear();
      write_realization( *module.second, os, module_settings );
    }

    os << ".begin" << std::endl;

    std::string cmd;

    for ( const auto& g : circ )
    {
      cmd = settings.type_label( g );

      std::vector<std::string> lines;

      // Peres is special
      boost::transform( g.controls(), std::back_inserter( lines ), line_to_variable() );
      boost::transform( g.targets(), std::back_inserter( lines ), line_to_variable() );

      os << cmd << " " << boost::algorithm::join( lines, " " );

      boost::optional<const std::map<std::string, std::string>&> annotations = circ.annotations( g );
      if ( annotations )
      {
        std::string sannotations;
        for ( const auto& p : *annotations )
        {
          sannotations += boost::str( boost::format( " %s=\"%s\"" ) % p.first % p.second );
        }
        os << " #@" << sannotations;
      }

      os << std::endl;
    }

    os << ".end" << std::endl;
  }

  bool write_realization( const circuit& circ, const std::string& filename, const write_realization_settings& settings, std::string* error )
  {
    std::filebuf fb;
    if ( !fb.open( filename.c_str(), std::ios::out ) )
    {
      if ( error )
      {
        *error = "Cannot open " + filename;
      }
      return false;
    }

    std::ostream os( &fb );

    write_realization( circ, os, settings );

    fb.close();

    return true;
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
