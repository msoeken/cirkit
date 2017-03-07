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

/**
 * This is a demonstration of the CLI API
 *
 * We show how to make a simple Mini ABC version based on the CLI API
 *
 * @author Mathias Soeken
 */

#define LIN64

#include <base/wlc/wlc.h>
#include <aig/gia/gia.h>

#include <boost/format.hpp>

#include <alice/command.hpp>
#include <alice/alice.hpp>

namespace alice
{

/******************************************************************************
 * store entry abc::Gia_Man_t*                                                *
 ******************************************************************************/

/* register abc::Gia_Man_t* as store element for AIGs */
template<>
struct store_info<abc::Gia_Man_t*>
{
  static constexpr const char* key         = "aigs";  /* internal key, must be unique for each store */
  static constexpr const char* option      = "aig";   /* long flag for general commands, e.g., `store --aig` */
  static constexpr const char* mnemonic    = "a";     /* short flag for general commands, e.g., `store -a` */
  static constexpr const char* name        = "AIG";   /* singular name for option descriptions */
  static constexpr const char* name_plural = "AIGs";  /* plural name for option descriptions */
};

/* I/O tag to implement `read_aiger` and `write_aiger` */
struct io_aiger_tag_t {};

/* return some short text for each AIG in `store -a` */
template<>
inline std::string store_entry_to_string<abc::Gia_Man_t*>( abc::Gia_Man_t* const& aig )
{
  return boost::str( boost::format( "%s i/o = %d/%d" ) % abc::Gia_ManName( aig ) % abc::Gia_ManPiNum( aig ) % abc::Gia_ManPoNum( aig ) );
}

/* print statistics on `ps -a` */
template<>
inline void print_store_entry_statistics<abc::Gia_Man_t*>( std::ostream& os, abc::Gia_Man_t* const& aig )
{
  abc::Gps_Par_t Pars;
  memset( &Pars, 0, sizeof(abc::Gps_Par_t) );
  abc::Gia_ManPrintStats( aig, &Pars );
}

/* enable `read_aiger` for AIGs */
template<>
inline bool store_can_read_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( command& cmd )
{
  return true;
}

/* implement `read_aiger` for AIGs */
template<>
inline abc::Gia_Man_t* store_read_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( const std::string& filename, const command& cmd )
{
  return abc::Gia_AigerRead( (char*)filename.c_str(), 0, 0, 0 );
}

/* enable `write_aiger` for AIGs */
template<>
inline bool store_can_write_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( command& cmd )
{
  return true;
}

/* implement `write_aiger` for AIGs */
template<>
inline void store_write_io_type<abc::Gia_Man_t*, io_aiger_tag_t>( abc::Gia_Man_t* const& aig, const std::string& filename, const command& cmd )
{
  abc::Gia_AigerWrite( aig, (char*)filename.c_str(), 1, 0 );
}

/******************************************************************************
 * store entry abc::Wlc_Ntk_t*                                                *
 ******************************************************************************/

/* register abc::Wlc_Ntk_t* as store element for word-level networks */
template<>
struct store_info<abc::Wlc_Ntk_t*>
{
  static constexpr const char* key         = "wlcs";  /* internal key, must be unique for each store */
  static constexpr const char* option      = "wlc";   /* long flag for general commands, e.g., `store --aig` */
  static constexpr const char* mnemonic    = "w";     /* short flag for general commands, e.g., `store -a` */
  static constexpr const char* name        = "WLC";   /* singular name for option descriptions */
  static constexpr const char* name_plural = "WLCs";  /* plural name for option descriptions */
};

/* I/O tag to implement `read_verilog` and `write_verilog` */
struct io_verilog_tag_t {};

/* return some short text for each WLC in `store -w` */
template<>
inline std::string store_entry_to_string<abc::Wlc_Ntk_t*>( abc::Wlc_Ntk_t* const& wlc )
{
  return boost::str( boost::format( "%s i/o = %d/%d" ) % wlc->pName % abc::Wlc_NtkPiNum( wlc ) % abc::Wlc_NtkPoNum( wlc ) );
}

/* print statistics on `ps -w` */
template<>
inline void print_store_entry_statistics<abc::Wlc_Ntk_t*>( std::ostream& os, abc::Wlc_Ntk_t* const& wlc )
{
  abc::Wlc_NtkPrintStats( wlc, 0, 0, 0 );
}

/* enable `read_verilog` for WLCs */
template<>
inline bool store_can_read_io_type<abc::Wlc_Ntk_t*, io_verilog_tag_t>( command& cmd )
{
  return true;
}

/* implement `read_verilog` for WLCs */
template<>
inline abc::Wlc_Ntk_t* store_read_io_type<abc::Wlc_Ntk_t*, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return abc::Wlc_ReadVer( (char*)filename.c_str(), nullptr );
}

/* enable `write_verilog` for WLCs */
template<>
inline bool store_can_write_io_type<abc::Wlc_Ntk_t*, io_verilog_tag_t>( command& cmd )
{
  return true;
}

/* implement `write_verilog` for WLCs */
template<>
inline void store_write_io_type<abc::Wlc_Ntk_t*, io_verilog_tag_t>( abc::Wlc_Ntk_t* const& wlc, const std::string& filename, const command& cmd )
{
  abc::Wlc_WriteVer( wlc, (char*)filename.c_str(), 0, 0 );
}

/******************************************************************************
 * conversion                                                                 *
 ******************************************************************************/

/* allow conversion from WLC to AIG with `convert --wlc_to_aig` */
template<>
inline bool store_can_convert<abc::Wlc_Ntk_t*, abc::Gia_Man_t*>()
{
  return true;
}

template<>
abc::Gia_Man_t* store_convert<abc::Wlc_Ntk_t*, abc::Gia_Man_t*>( abc::Wlc_Ntk_t* const& wlc )
{
  return abc::Wlc_NtkBitBlast( wlc, nullptr, -1, 2, 0, 0, 0 );
}

/******************************************************************************
 * custom commands                                                            *
 ******************************************************************************/

class miter_command : public command
{
public:
  miter_command( const environment::ptr& env ) : command( env, "Create miter" )
  {
    opts.add_options()
      ( "id1",   po::value( &id1 )->default_value( id1 ), "store id of first circuit" )
      ( "id2",   po::value( &id2 )->default_value( id2 ), "store id of second circuit" )
      ( "new,n",                                          "create new store entry" )
      ;
  }

protected:
  /* rules to check before execution:
   *
   * rules_t is a vector of pairs, each pair is a predicate and a string:
   * predicates are checked in order, if they are false, the command is not executed and the string is
   * outputted as error.
   */
  rules_t validity_rules() const
  {
    return {
      { [this]() { return id1 != id2; }, "ids must be different" },
      { [this]() {
          const auto& aigs = env->store<abc::Gia_Man_t*>();
          return id1 < aigs.size() && id2 < aigs.size();
        }, "ids are out of range" }
    };
  }

  bool execute()
  {
    auto& aigs = env->store<abc::Gia_Man_t*>();

    if ( is_set( "new" ) )
    {
      aigs.extend();
    }
    aigs.current() = abc::Gia_ManMiter( aigs[id1], aigs[id2], 0, 0, 0, 0, 0 );

    return true;
  }

  log_opt_t log() const
  {
    return log_opt_t({
        {"id1", id1},
        {"id2", id2}
      });
  }

private:
  unsigned id1 = 0;
  unsigned id2 = 1;
};

}

/******************************************************************************
 * main program                                                               *
 ******************************************************************************/

int main( int argc, char ** argv )
{
  using namespace alice;

  cli_main<abc::Gia_Man_t*, abc::Wlc_Ntk_t*> cli( "abc" );

  cli.set_category( "I/O" );
  ADD_READ_COMMAND( aiger, "Aiger" );
  ADD_WRITE_COMMAND( aiger, "Aiger" );
  ADD_READ_COMMAND( verilog, "Verilog" );
  ADD_WRITE_COMMAND( verilog, "Verilog" );

  cli.set_category( "Verification" );
  ADD_COMMAND( miter );

  return cli.run( argc, argv );
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
