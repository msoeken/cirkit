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

#include "environment.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

#include <boost/format.hpp>
#include <boost/variant.hpp>

#include <core/cli/command.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::string json_escape( const std::string& s )
{
  std::stringstream ss;

  for ( size_t i = 0; i < s.length(); ++i )
  {
    if ( s[i] == '\\' || s[i] == '"' )
    {
      ss << "\\" << s[i];
    }
    else if ( unsigned( s[i] ) < '\x20' )
    {
      ss << "\\u" << std::setfill( '0' ) << std::setw( 4 ) << std::hex << unsigned( s[i] );
    }
    else
    {
      ss << s[i];
    }
  }

  return ss.str();
}

class log_var_visitor : public boost::static_visitor<void>
{
public:
  log_var_visitor( std::ostream& os ) : os( os ) {}

  void operator()( const std::string& s ) const
  {
    os << "\"" << json_escape( s ) << "\"";
  }

  void operator()( int i ) const
  {
    os << i;
  }

  void operator()( unsigned i ) const
  {
    os << i;
  }

  void operator()( double d ) const
  {
    os << d;
  }

  void operator()( bool b ) const
  {
    os << ( b ? "true" : "false" );
  }

  template<typename T>
  void operator()( const std::vector<T>& v ) const
  {
    os << "[";

    bool first = true;

    for ( const auto& element : v )
    {
      if ( !first )
      {
        os << ", ";
      }
      else
      {
        first = false;
      }

      operator()( element );
    }

    os << "]";
  }

private:
  std::ostream& os;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void environment::start_logging( const std::string& filename )
{
  logger.open( filename.c_str(), std::ofstream::out );
  logger << "[";
}

void environment::log_command( const std::shared_ptr<command>& cmd, const std::string& cmdstring, const std::chrono::system_clock::time_point& start )
{
  log_command( cmd->log(), cmdstring, start );
}

void environment::log_command( const command_log_opt_t& cmdlog, const std::string& cmdstring, const std::chrono::system_clock::time_point& start )
{
  using boost::format;

  if ( !log_first_command )
  {
    logger << "," << std::endl;
  }
  else
  {
    log_first_command = false;
  }

  const auto start_c = std::chrono::system_clock::to_time_t( start );
  char timestr[20];
  std::strftime( timestr, sizeof( timestr ), "%F %T", std::localtime( &start_c ) );
  logger << format( "{\n"
                    "  \"command\": \"%s\",\n"
                    "  \"time\": \"%s\"" ) % json_escape( cmdstring ) % timestr;

  if ( cmdlog != boost::none )
  {
    log_var_visitor vis( logger );

    for ( const auto& p : *cmdlog )
    {
      logger << format( ",\n  \"%s\": " ) % p.first;
      boost::apply_visitor( vis, p.second );
    }
  }

  logger << "\n}";
}

void environment::stop_logging()
{
  logger << "]" << std::endl;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
