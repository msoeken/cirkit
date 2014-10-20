/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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

#include <iostream>
#include <regex>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/program_options.hpp>
#include <core/utils/string_utils.hpp>

using namespace boost::assign;
using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string path;
  std::string pattern = "([^\\.]*)\\.real";
  std::string columns;
  std::string global;

  program_options opts;
  opts.add_options()
    ( "path",    value( &path ),                 "Path of circuit files" )
    ( "pattern", value_with_default( &pattern ), "Pattern for parsing, must contain at least one capture group for benchmark name and may contain a second one for column name" )
    ( "columns", value( &columns ),              "Columns, e.g. \"00=Col 1,01=Col 2\"" )
    ( "global",  value( &global ),               "Global properties (comma separated)" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "path" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* Parse columns */
  typedef std::vector<std::pair<std::string, std::string>> columns_vector_t;
  columns_vector_t columns_vector;
  if ( !columns.empty() )
  {
    foreach_string( columns, ",", [&columns_vector]( const std::string& s ) {
        columns_vector += split_string_pair( s, "=" );
      } );
  }

  /* Global and local parameters */
  std::vector<std::string> globals;
  split_string( globals, global, "," );

  typedef std::tuple<unsigned, double> column_t;
  typedef std::vector<std::tuple<std::string, std::vector<unsigned>, std::vector<column_t>>> table_t;

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
          table += std::make_tuple( name, std::vector<unsigned>( globals.size() ), std::vector<column_t>( has_column_id ? columns_vector.size() : 1u ) );
          prow = &table.back();
        }
        else
        {
          prow = &*it;
        }

        /* has column? */
        auto index = has_column_id ? boost::find_if( columns_vector, [&m]( const columns_vector_t::value_type& col ) { return col.first == m[2u]; } ) - columns_vector.begin() : 0u;
        std::tuple<unsigned, double> entry;

        line_parser( boost::str( boost::format( "%s/%s.log" ) % f.path().parent_path().string() % f.path().stem().string() ), {
            { std::regex( "^([^:]*): *(.*)$" ), [&globals, &prow, &entry]( const std::smatch& m ) {
                auto it = boost::find( globals, m[1u] );
                if ( it != globals.end() )
                {
                  std::get<1>( *prow )[it - globals.begin()] = boost::lexical_cast<unsigned>( m[2u] );
                }

                if ( m[1u] == "Runtime" )
                {
                  std::get<1>( entry ) = boost::lexical_cast<double>( m[2u] );
                }
                else if ( m[1u] == "Gates" )
                {
                  std::get<0>( entry ) = boost::lexical_cast<unsigned>( m[2u] );
                }
              }
            },
            { std::regex( "^Runtime: *(.*)$" ), [&entry]( const std::smatch& m ) { std::get<1>( entry ) = boost::lexical_cast<double>( m[1u] ); } },
            { std::regex( "^Gates: *(.*)$" ), [&entry]( const std::smatch& m ) { std::get<0>( entry ) = boost::lexical_cast<unsigned>( m[1u] ); } }
          } );
          std::get<2>( *prow )[index] = entry;
      }
    }
  }

  /* Print table */
  for ( const auto& row : table )
  {
    std::cout << boost::format( "| %20s |" ) % std::get<0>( row );

    for ( auto entry : std::get<1>( row ) )
    {
      std::cout << boost::format( "%10d |" ) % entry;
    }

    for ( unsigned i = 0u; i < ( columns.empty() ? 1u : columns_vector.size() ); ++i )
    {
      std::cout << boost::format( " %10d | %6.2f |" ) % std::get<0>( std::get<2>( row )[i] ) % std::get<1>( std::get<2>( row )[i] );
    }
    std::cout << std::endl;
  }

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
