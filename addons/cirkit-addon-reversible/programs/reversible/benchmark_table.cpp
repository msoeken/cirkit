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

/**
 * @author Mathias Soeken
 */

#include <iostream>
#include <regex>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>

using namespace boost::assign;
using namespace cirkit;

using entry_t             = boost::variant<unsigned, double>;
using column_t            = std::vector<entry_t>;
using table_t             = std::vector<std::tuple<std::string, column_t, std::vector<column_t>>>;
using label_type_vector_t = std::vector<std::pair<std::string, char>>;

label_type_vector_t parse_label_type_string( const std::string& str )
{
  label_type_vector_t vec;
  if ( !str.empty() )
  {
    foreach_string( str, ",", [&vec]( const std::string& s ) {
        auto p = split_string_pair( s, ":" );
        vec += std::make_pair( p.first, p.second[0u] );
      } );
  }
  return vec;
}

void parse_log_entry( column_t& column, const boost::smatch& m, const label_type_vector_t& label_types )
{
  auto it = boost::find_if( label_types, first_matches<label_type_vector_t::value_type>( m[1] ) );
  if ( it != label_types.end() )
  {
    if ( it->second == 'u' )
    {
      column[it - label_types.begin()] = boost::lexical_cast<unsigned>( m[2u] );
    }
    else
    {
      column[it - label_types.begin()] = boost::lexical_cast<double>( m[2u] );
    }
  }
}

void print_column( const column_t& column )
{
  for ( const auto& entry : column )
  {
    if ( const unsigned* u = boost::get<unsigned>( &entry ) )
    {
      std::cout << boost::format( "%10d |" ) % *u;
    }
    else if ( const double* d = boost::get<double>( &entry ) )
    {
      std::cout << boost::format( "%10.2f |" ) % *d;
    }
    else
    {
      assert( false );
    }
  }
}

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string path;
  std::string pattern = "([^\\.]*)\\.real";
  std::string columns;
  std::string global = "Lines:u";
  std::string local = "Gates:u,Runtime:f";

  program_options opts;
  opts.add_options()
    ( "path",    value( &path ),                 "Path of circuit files" )
    ( "pattern", value_with_default( &pattern ), "Pattern for parsing, must contain at least "
                                                 "one capture group for benchmark name and may "
                                                 "contain a second one for column name" )
    ( "columns", value( &columns ),              "Columns, e.g. \"00=Col 1,01=Col 2\"" )
    ( "global",  value( &global ),               "Global properties with type, e.g. \"Lines:u,Runtime:f\"" )
    ( "local",   value( &local ),                "Local properties with type, e.g. \"Lines:u,Runtime:f\"" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "path" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* Parse columns */
  using columns_vector_t = std::vector<std::pair<std::string, std::string>>;
  columns_vector_t columns_vector;
  if ( !columns.empty() )
  {
    foreach_string( columns, ",", [&columns_vector]( const std::string& s ) {
        columns_vector += split_string_pair( s, "=" );
      } );
  }

  /* Global and local parameters */
  auto globals = parse_label_type_string( global );
  auto locals = parse_label_type_string( local );

  table_t table;

  std::regex r( pattern );
  std::smatch m;
  boost::filesystem::path p( path );
  if ( exists( p ) && is_directory( p ) )
  {
    for ( const auto& f : boost::make_iterator_range( boost::filesystem::directory_iterator( p ), boost::filesystem::directory_iterator() ) )
    {
      auto filename = f.path().filename().string();
      if ( std::regex_match( filename, m, r ) )
      {
        auto has_column_id = m.size() == 3u;
        const auto& name = m[1u];

        /* add or retrieve row */
        table_t::value_type* prow = nullptr;
        auto it = boost::find_if( table, [&name]( const table_t::value_type& row ) { return std::get<0>( row ) == name; } );
        if ( it == table.end() )
        {
          table += std::make_tuple( name, std::vector<entry_t>( globals.size() ), std::vector<column_t>( has_column_id ? columns_vector.size() : 1u ) );
          prow = &table.back();
        }
        else
        {
          prow = &*it;
        }

        /* has column? */
        auto index = has_column_id ? boost::find_if( columns_vector, first_matches<columns_vector_t::value_type>( m[2u] ) ) - columns_vector.begin() : 0u;
        std::get<2>( *prow )[index] = column_t( locals.size() );

        line_parser( boost::str( boost::format( "%s/%s.log" ) % f.path().parent_path().string() % f.path().stem().string() ), {
            { boost::regex( "^([^:]*): *(.*)$" ), [&globals, &locals, &prow, &index]( const boost::smatch& m ) {
                parse_log_entry( std::get<1>( *prow ), m, globals );
                parse_log_entry( std::get<2>( *prow )[index], m, locals );
                auto itl = boost::find_if( locals, first_matches<label_type_vector_t::value_type>( m[1u] ) );
              }
            }
          } );
      }
    }
  }

  /* Print table */
  for ( const auto& row : table )
  {
    std::cout << boost::format( "| %20s |" ) % std::get<0>( row );
    print_column( std::get<1>( row ) );
    boost::for_each( std::get<2>( row ), print_column );
    std::cout << std::endl;
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
