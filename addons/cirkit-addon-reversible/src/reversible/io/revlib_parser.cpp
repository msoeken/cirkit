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

#include "revlib_parser.hpp"

#include <iostream>
#include <locale>
#include <map>
#include <stack>

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/spirit/include/qi.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/io/revlib_processor.hpp>

using namespace boost::assign;

namespace cirkit
{

struct transform_to_garbage
{
  bool operator()( const char& c ) const
  {
    return c == '1';
  }
};

struct transform_to_constants
{
  constant operator()( const char& c ) const
  {
    switch ( c ) {
    case '-':
      return constant();
      break;

    case '0':
    case '1':
      return constant( c == '1' );
      break;

    default:
      assert( false );
      return constant();
      break;
    }
  }
};

// CODE move to utils
template<typename Map>
struct map_access_functor
{
  explicit map_access_functor( Map& map )
    : _map( map )
  {
  }

  const typename Map::mapped_type& operator()( const typename Map::key_type& key ) const
  {
    return _map.find( key )->second;
  }

private:
  Map& _map;
};

template<typename Map>
map_access_functor<Map> make_map_access_functor( Map& map )
{
  return map_access_functor<Map>( map );
}

bool parse_string_list( const std::string& line, std::vector<std::string>& params )
{
  enum class parse_state { none, quoted, unquoted } state = parse_state::none;
  std::string item;

  for ( auto c : line )
  {
    switch ( state )
    {
    case parse_state::none:
      switch ( c )
      {
      case ' ':
        break;
      case '"':
        state = parse_state::quoted;
        break;
      default:
        state = parse_state::unquoted;
        item += c;
        break;
      }
      break;

    case parse_state::quoted:
      switch ( c )
      {
      case '"':
        state = parse_state::none;
        params.push_back( item );
        item.clear();
        break;
      default:
        item += c;
        break;
      }
      break;

    case parse_state::unquoted:
      switch ( c )
      {
      case ' ':
        state = parse_state::none;
        params.push_back( item );
        item.clear();
        break;
      default:
        item += c;
        break;
      }
      break;
    }
  }

  if ( state == parse_state::unquoted )
  {
    params.push_back( item );
  }

  return true;
}

bool parse_annotations( const std::string& line, std::vector<std::pair<std::string, std::string> >& annotations )
{
  enum class parse_state { none, key, assign, quoted, unquoted } state = parse_state::none;
  std::string key, value;

  for ( auto c : line )
  {
    switch ( state )
    {
    case parse_state::none:
      switch ( c )
      {
      case ' ':
        break;
      case '=':
        assert( false );
        break;
      case '"':
        assert( false );
        break;
      default:
        state = parse_state::key;
        key += c;
        break;
      }
      break;

    case parse_state::key:
      switch ( c )
      {
      case '=':
        state = parse_state::assign;
        break;
      case ' ':
        assert( false );
        break;
      case '"':
        assert( false );
        break;
      default:
        key += c;
        break;
      }
      break;

    case parse_state::assign:
      switch ( c )
      {
      case ' ':
        assert( false );
        break;
      case '=':
        assert( false );
        break;
      case '"':
        state = parse_state::quoted;
        break;
      default:
        state = parse_state::unquoted;
        value += c;
        break;
      }
      break;

    case parse_state::quoted:
      switch ( c )
      {
      case '"':
        state = parse_state::none;
        annotations.push_back( {key, value} );
        key.clear();
        value.clear();
        break;
      default:
        value += c;
        break;
      }
      break;

    case parse_state::unquoted:
      switch ( c )
      {
      case ' ':
        state = parse_state::none;
        annotations.push_back( {key, value} );
        key.clear();
        value.clear();
        break;
      default:
        value += c;
        break;
      }
      break;
    }
  }

  if ( state == parse_state::unquoted )
  {
    annotations.push_back( {key, value} );
  }

  return true;
}

bool is_number( const std::string& str )
{
  for ( const auto& c : str )
  {
    if ( !std::isdigit( c ) )
    {
      return false;
    }
  }

  return true;
}

boost::optional<boost::any> revlib_parser_string_to_target_tag( const std::string& str )
{
  switch ( str[0] )
  {
  case 't':
    return boost::optional<boost::any>( toffoli_tag() );

  case 'p':
    return boost::optional<boost::any>( peres_tag() );

  case 'f':
    return boost::optional<boost::any>( fredkin_tag() );

  default:
    return boost::optional<boost::any>();
  }
}

bool revlib_parser( std::istream& in, revlib_processor& reader, const revlib_parser_settings& settings, std::string* error )
{
  std::string line;

  unsigned numvars = 0;
  unsigned truth_table_index = 0;
  std::stack<std::map<std::string, unsigned> > variable_indices;
  std::vector<std::string> module_names;

  while ( in.good() && getline( in, line ) )
  {
    /* clear previous annotations */
    reader.clear_annotations();

    /* extract comments */
    if ( boost::iterator_range<std::string::iterator> result = boost::algorithm::find_first( line, "#" ) )
    {
      std::string comment( result.begin() + 1u, line.end() );

      if ( !comment.empty() && comment.at( 0u ) == '@' )
      {
        std::string sannotations( comment.begin() + 1u, comment.end() );
        std::vector<std::pair<std::string, std::string> > annotations;
        boost::algorithm::trim( sannotations );
        parse_annotations( sannotations, annotations );

        for ( const auto& pair : annotations )
        {
          reader.add_annotation( pair.first, pair.second );
        }
      }
      else
      {
        reader.on_comment( comment );
      }

      line.erase( result.begin(), line.end() );
    }

    boost::algorithm::trim( line );

    /* skip empty lines */
    if ( !line.size() )
    {
      continue;
    }

    std::vector<std::string> params;
    boost::algorithm::split( params, line, boost::algorithm::is_any_of( " " ) );

    /* It is possible that there are empty elements in params,
       e.g. when line contains two spaces between identifiers instead of one.
       These should be removed. */
    std::vector<std::string>::iterator newEnd = std::remove( params.begin(), params.end(), "" );
    params.erase( newEnd, params.end() );

    /* By means of the first element we can determine the command */
    std::string command = params.front();
    params.erase( params.begin() );

    if ( command == "#" )
    {
      /* All parameters combined are considered as comment */
      reader.on_comment( boost::algorithm::join( params, " " ) );
    }
    else if ( command == ".version" )
    {
      /* All parameters combined are considered as version */
      reader.on_version( boost::algorithm::join( params, " " ) );
    }
    else if ( command == ".numvars" )
    {
      if ( params.size() != 1 )
      {
        if ( error )
        {
          *error = "Invalid number of parameters for .numvars command";
        }
        return false;
      }

      try
      {
        numvars = boost::lexical_cast<unsigned>( params.front() );
        reader.on_numvars( numvars );
      }
      catch ( boost::bad_lexical_cast& )
      {
        if ( error )
        {
          *error = "Invalid parameter for .numvars command";
        }
        return false;
      }
    }
    else if ( command == ".variables" )
    {
      if ( params.size() != numvars )
      {
        if ( error )
        {
          *error = "Variable count does not fit numvars";
        }
        return false;
      }

      /* fill the index map later used when processing the gates */
      std::map<std::string, unsigned> map;
      for ( std::vector<std::string>::const_iterator it = params.begin(); it != params.end(); ++it )
      {
        map.insert( std::make_pair( *it, it - params.begin() ) );
      }
      variable_indices.push( map );

      reader.on_variables( params.begin(), params.end() );
    }
    else if ( command == ".inputs" )
    {
      // since the inputs can contain spaces in quotes we have to deal with
      // them specifically
      std::string inputs_str( line.begin() + command.size() + 1u, line.end() );
      boost::algorithm::trim( inputs_str );
      std::vector<std::string> inputs;

      if ( !parse_string_list( inputs_str, inputs ) )
      {
        if ( error )
        {
          *error = "Cannot parse .input command";
        }
        return false;
      }

      if ( inputs.size() != numvars )
      {
        if ( error )
        {
          *error = "Input count does not fit numvars";
        }
        return false;
      }

      reader.on_inputs( inputs.begin(), inputs.end() );
    }
    else if ( command == ".outputs" )
    {
      // see .inputs
      std::string outputs_str( line.begin() + command.size() + 1u, line.end() );
      boost::algorithm::trim( outputs_str );
      std::vector<std::string> outputs;

      if ( !parse_string_list( outputs_str, outputs ) )
      {
        if ( error )
        {
          *error = "Cannot parse .output command";
        }
        return false;
      }

      if ( outputs.size() != numvars )
      {
        if ( error )
        {
          *error = "Output count does not fit numvars";
        }
        return false;
      }

      reader.on_outputs( outputs.begin(), outputs.end() );
    }
    else if ( command == ".constants" )
    {
      if ( params.size() != 1 || params.front().size() != numvars )
      {
        if ( error )
        {
          *error = "Constant count does not fit numvars";
        }
        return false;
      }

      std::vector<constant> constants( numvars );
      std::transform( params.front().begin(), params.front().end(), constants.begin(), transform_to_constants() );

      reader.on_constants( constants.begin(), constants.end() );
    }
    else if ( command == ".garbage" )
    {
      if ( params.size() != 1 || params.front().size() != numvars )
      {
        if ( error )
        {
          *error = "Garbage count does not fit numvars";
        }
        return false;
      }

      std::vector<bool> garbage( numvars );
      std::transform( params.front().begin(), params.front().end(), garbage.begin(), transform_to_garbage() );

      reader.on_garbage( garbage.begin(), garbage.end() );
    }
    else if ( command == ".inputbus" )
    {
      if ( params.size() < 2u )
      {
        if ( error )
        {
          *error = "Too few arguments in .inputbus command";
        }
        return false;
      }

      std::vector<unsigned> line_indices( params.size() - 1u );
      std::transform( params.begin() + 1u, params.end(), line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

      reader.on_inputbus( params.front(), line_indices );
    }
    else if ( command == ".outputbus" )
    {
      if ( params.size() < 2u )
      {
        if ( error )
        {
          *error = "Too few arguments in .outputbus command";
        }
        return false;
      }

      std::vector<unsigned> line_indices( params.size() - 1u );
      std::transform( params.begin() + 1u, params.end(), line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

      reader.on_outputbus( params.front(), line_indices );
    }
    else if ( command == ".state" )
    {
      if ( params.size() < 2u )
      {
        if ( error )
        {
          *error = "Too few arguments in .state command";
        }
        return false;
      }

      boost::optional<unsigned> initial_value;

      if ( is_number( params.back() ) )
      {
        if ( params.size() == 2u )
        {
          if ( error )
          {
            *error = "Too few arguments in .state command";
          }
          return false;
        }

        initial_value = boost::lexical_cast<unsigned>( params.back() );
      }

      unsigned offset = initial_value ? 1u : 0u;

      std::vector<unsigned> line_indices( params.size() - 1u - offset );
      std::transform( params.begin() + 1u, params.end() - offset, line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

      reader.on_state( params.front(), line_indices, initial_value.get_value_or( 0u ) );
    }
    else if ( command == ".module" )
    {
      assert( params.size() <= 2u );
      std::string name = params.front();

      boost::optional<std::string> filename;
      if ( params.size() == 2u )
      {
        filename = settings.base_directory + "/" + params.back();
      }

      if ( filename && !boost::filesystem::exists( *filename ) )
      {
        if ( error )
        {
          *error = boost::str( boost::format( "File for module %s not found" ) % name );
        }
        return false;
      }

      module_names += name;

      reader.on_module( name, filename );
    }
    else if ( command == ".begin" )
    {
      if ( params.size() != 0 )
      {
        if ( error )
        {
          *error = "Wrong number of parameters for .begin command";
        }
        return false;
      }

      reader.on_begin();

      if ( !settings.read_gates )
      {
        return true;
      }
    }
    else if ( command == ".end" )
    {
      if ( params.size() != 0 )
      {
        if ( error )
        {
          *error = "Wrong number of parameters for .end command";
        }
        return false;
      }

      if ( !variable_indices.empty() )
      {
        variable_indices.pop();
      }
      reader.on_end();
    }
    else
    {
      if ( command[0] == '-' || command[0] == '1' || command[0] == '0' )
      {
        // truth table line
        if ( params.size() )
        {
          if ( error )
          {
            *error = "Params in truth table line";
          }
          return false;
        }

        std::vector<boost::optional<bool> > cube( command.size() );
        std::transform( command.begin(), command.end(), cube.begin(), transform_to_constants() );

        reader.on_truth_table_line( truth_table_index++, cube.begin(), cube.end() );
      }
      else
      {
        // gate
        std::vector<variable> line_indices;
        boost::transform( params, std::back_inserter( line_indices ), [&variable_indices]( const std::string& s ) {
            bool polarity = s[0] != '-';
            return make_var( variable_indices.top()[polarity ? s : s.substr( 1u )], polarity );
          } );

        boost::any gate_type;

        if ( boost::range::find( module_names, command ) != module_names.end() )
        {
          module_tag module_t;
          module_t.name = command;
          gate_type = module_t;
        }
        else if ( boost::starts_with( command, "stg" ) )
        {
          const auto p = split_string_pair( command.substr( 3u, command.size() - 4u ), "[" );
          assert( line_indices.size() > 2 );
          stg_tag stg_t;
          stg_t.function = boost::dynamic_bitset<>( convert_hex2bin( p.second ) );
          gate_type = stg_t;
        }
        else
        {
          auto tag = settings.string_to_target_tag( command );
          if ( tag )
          {
            gate_type = *tag;
          }
          else
          {
            if ( error )
            {
              *error = "unknown gate command: " + command;
            }
          }
        }

        reader.on_gate( gate_type, line_indices );
      }
    }
  }

  return true;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
