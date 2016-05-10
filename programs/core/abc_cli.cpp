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
 * This is a demonstration of the CLI API
 *
 * We show how to make a simple Mini ABC version based on the CLI API
 *
 * @author Mathias Soeken
 */

#define LIN64

#include <aig/gia/gia.h>

#include <boost/format.hpp>

#include <lscli/command.hpp>
#include <lscli/utils.hpp>

namespace cirkit
{

/******************************************************************************
 * store entry abc::Gia_Man_t*                                                *
 ******************************************************************************/

/* register abc::Gia_Man_t* as store element for AIGs */
template<>
struct store_info<abc::Gia_Man_t*>
{
  static constexpr const char* key         = "aigs";
  static constexpr const char* option      = "aig";
  static constexpr const char* mnemonic    = "a";
  static constexpr const char* name        = "AIG";
  static constexpr const char* name_plural = "AIGs";
};

struct io_aiger_tag_t {};

template<>
inline std::string store_entry_to_string<abc::Gia_Man_t*>( abc::Gia_Man_t* const& aig )
{
  return boost::str( boost::format( "%s i/o = %d/%d" ) % Gia_ManName( aig ) % Gia_ManPiNum( aig ) % Gia_ManPoNum( aig ) );
}

template<>
inline void print_store_entry_statistics<abc::Gia_Man_t*>( std::ostream& os, abc::Gia_Man_t* const& aig )
{
  abc::Gps_Par_t Pars;
  memset( &Pars, 0, sizeof(abc::Gps_Par_t) );
  Gia_ManPrintStats( aig, &Pars );
}

template<>
inline bool store_can_read_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( const cli_options& opts )
{
  return true;
}

template<>
inline abc::Gia_Man_t* store_read_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( const std::string& filename, const cli_options& opts )
{
  return abc::Gia_AigerRead( (char*)filename.c_str(), 0, 0 );
}

template<>
inline bool store_can_write_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( const cli_options& opts )
{
  return true;
}

template<>
inline void store_write_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( abc::Gia_Man_t* const& aig, const std::string& filename, const cli_options& opts )
{
  abc::Gia_AigerWrite( aig, (char*)filename.c_str(), 1, 0 );
}

/******************************************************************************
 * custom commands                                                            *
 ******************************************************************************/

class miter_command : public command
{
public:
  miter_command( const environment::ptr& env ) : command( env, "create miter" )
  {
    opts.add_options()
      ( "id1", po::value( &id1 )->default_value( id1 ), "store id of first circuit" )
      ( "id2", po::value( &id2 )->default_value( id2 ), "store id of second circuit" )
      ;
  }

protected:
  bool execute()
  {
    auto& aigs = env->store<abc::Gia_Man_t*>();

    aigs.extend();
    aigs.current() = abc::Gia_ManMiter( aigs[id1], aigs[id2], 0, 0, 0, 0, 0 );

    return true;
  }

private:
  unsigned id1 = 0;
  unsigned id2 = 1;
};

}

int main( int argc, char ** argv )
{
  using namespace cirkit;

  cli_main<abc::Gia_Man_t*> cli( "abc" );

  cli.set_category( "I/O" );
  ADD_READ_COMMAND( aiger, "Aiger" );
  ADD_WRITE_COMMAND( aiger, "Aiger" );

  cli.set_category( "Verification" );
  ADD_COMMAND( miter );

  return cli.run( argc, argv );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
