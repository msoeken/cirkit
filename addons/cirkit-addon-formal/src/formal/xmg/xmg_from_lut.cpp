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

#include "xmg_from_lut.hpp"

#include <fstream>
#include <map>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/npn_canonization.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg_rewrite.hpp>
#include <formal/synthesis/exact_mig.hpp>
#include <formal/xmg/xmg_minlib.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class xmg_from_lut_mapping_manager
{
public:
  xmg_from_lut_mapping_manager( const lut_graph_t& lut, const properties::ptr& settings )
    : lut( lut ),
      verbose( get( settings, "verbose", false ) ),
      node_to_function( num_vertices( lut ) ),
      minlib( settings )
  {
    /* settings */
    dump_luts = get( settings, "dump_luts", std::string() );
    npn       = get( settings, "npn",       true );
    noxor     = get( settings, "noxor",     false );
  }

  xmg_graph run()
  {
    if ( !dump_luts.empty() )
    {
      dump_luts_to_file();
      return xmg;
    }

    if ( !noxor )
    {
      minlib.load_library_string( xmg_minlib_manager::npn2_s );
      minlib.load_library_string( xmg_minlib_manager::npn3_s );
      minlib.load_library_string( xmg_minlib_manager::npn4_s );

      read_library_from_file();
    }
    else
    {
      minlib.load_library_string( xmg_minlib_manager::npn2_s_mig );
      minlib.load_library_string( xmg_minlib_manager::npn3_s_mig );
      minlib.load_library_string( xmg_minlib_manager::npn4_s_mig );
    }

    compute_optimal_xmgs();

    if ( !noxor )
    {
      write_library_to_file();
    }

    initialize_xmg();
    map();

    return xmg;
  }

private:
  void compute_optimal_xmgs()
  {
    const auto tts = get( boost::vertex_lut, lut );
    const auto types = get( boost::vertex_lut_type, lut );

    auto ex_settings = std::make_shared<properties>();
    ex_settings->set( "verbose", verbose );

    for ( const auto& v : boost::make_iterator_range( vertices( lut ) ) )
    {
      if ( types[v] != lut_type_t::internal ) { continue; }
      if ( boost::out_degree( v, lut ) == 1u ) { continue; }

      tt t( convert_hex2bin( tts[v] ) );

      const auto size = tt_num_vars( t );

      if ( boost::out_degree( v, lut ) != size )
      {
        std::cout << "[e] size mismatch" << std::endl;
        std::cout << "[e] out degree: " << boost::out_degree( v, lut ) << std::endl;
        std::cout << "[e] num vars:   " << size << std::endl;
        std::cout << "[e] lut:        " << tts[v] << std::endl;
        std::cout << "[e] type:       " << static_cast<unsigned>( types[v] ) << std::endl;
        assert( false );
      }

      // tt_extend( t, 4u ); /* at least 4 variables */

      if ( optimal_xmgs.find( tts[v] ) != optimal_xmgs.end() )
      {
        if ( verbose )
        {
          //std::cout << "[i] used cached entry" << std::endl;
        }
        continue;
      }

      if ( verbose )
      {
        //std::cout << "[i] compute XMG for " << tt_to_hex( t ) << std::endl;
      }

      xmg_graph xmg;
      if ( npn )
      {
        xmg = minlib.find_xmg( t );
      }
      else
      {
        xmg = minlib.find_xmg_no_npn( t );
      }
      optimal_xmgs.insert( std::make_pair( tts[v], xmg ) );
    }
  }

  void initialize_xmg()
  {
    const auto names = boost::get( boost::vertex_name, lut );
    const auto types = boost::get( boost::vertex_lut_type, lut );

    for ( const auto& v : boost::make_iterator_range( vertices( lut ) ) )
    {
      switch ( types[v] )
      {
      case lut_type_t::gnd:
        node_to_function[v] = xmg.get_constant( false );
        break;

      case lut_type_t::vdd:
        node_to_function[v] = xmg.get_constant( true );
        break;

      case lut_type_t::pi:
        node_to_function[v] = xmg.create_pi( names[v] );
        break;

      default:
        break;
      }
    }
  }

  void map()
  {
    auto names = boost::get( boost::vertex_name, lut );
    auto tts = boost::get( boost::vertex_lut, lut );
    auto types = boost::get( boost::vertex_lut_type, lut );

    std::vector<lut_vertex_t> topsort( num_vertices( lut ) );
    boost::topological_sort( lut, topsort.begin() );
    for ( auto v : topsort )
    {
      switch ( types[v] )
      {
      case lut_type_t::internal:
        {
          std::vector<xmg_function> pi_mapping;
          for ( const auto& child : boost::make_iterator_range( boost::adjacent_vertices( v, lut ) ) )
          {
            pi_mapping.push_back( node_to_function[child] );
          }

          /* some special cases */
          if ( boost::out_degree( v, lut ) <= 1u )
          {
            if ( tts[v] == "2" )
            {
              node_to_function[v] = pi_mapping.front();
              break;
            }
            else if ( tts[v] == "1" )
            {
              node_to_function[v] = !pi_mapping.front();
              break;
            }

            std::cout << "[e] special case for " << tts[v] << " with " << boost::out_degree( v, lut ) << " input missing" << std::endl;
            assert( false );
          }

          const auto it = optimal_xmgs.find( tts[v] );
          assert ( it != optimal_xmgs.end() );

          const auto opt_xmg = it->second;
          if ( opt_xmg.inputs().size() != boost::out_degree( v, lut ) )
          {
            boost::dynamic_bitset<> phase;
            std::vector<unsigned> perm;
            tt orig( convert_hex2bin( tts[v] ) );
            auto npn = npn_canonization_lucky( orig, phase, perm );

            std::cout << "[e] problem when mapping LUT " << v << " with TT " << tts[v] << " to XMG" << std::endl;
            std::cout << "[e] LUT has " << boost::out_degree( v, lut ) << " inputs" << std::endl;
            std::cout << "[e] XMG has " << opt_xmg.inputs().size() << " inputs" << std::endl;
            std::cout << "[e] TT: " << orig << ", NPN: " << npn << std::endl;
            assert( false );
          }
          const auto outputs = xmg_rewrite_top_down_inplace( xmg, opt_xmg, rewrite_default_maj, rewrite_default_xor, pi_mapping );

          if ( outputs.size() != 1u )
          {
            std::cout << "[e] expected: 1, got: " << outputs.size() << std::endl;
            assert( false );
          }

          node_to_function[v] = outputs.front();
        } break;

      case lut_type_t::po:
        {
          xmg.create_po( node_to_function[*boost::adjacent_vertices( v, lut ).first], names[v] );
        } break;

      default:
        break;
      }
    }
  }

  void read_library_from_file()
  {
    if ( const auto* path = std::getenv( "CIRKIT_HOME" ) )
    {
      const auto filename = boost::str( boost::format( "%s/xmgmin.txt" ) % path );
      if ( boost::filesystem::exists( filename ) )
      {
        minlib.load_library_file( filename );
      }
    }
  }

  void write_library_to_file()
  {
    if ( const auto* path = std::getenv( "CIRKIT_HOME" ) )
    {
      minlib.write_library_file( boost::str( boost::format( "%s/xmgmin.txt" ) % path ) );
    }
  }

  void dump_luts_to_file()
  {
    std::ofstream os( dump_luts.c_str(), std::ofstream::out );

    const auto tts = get( boost::vertex_lut, lut );
    const auto types = get( boost::vertex_lut_type, lut );

    for ( const auto& v : boost::make_iterator_range( vertices( lut ) ) )
    {
      if ( types[v] != lut_type_t::internal ) { continue; }
      if ( boost::out_degree( v, lut ) <= 2u ) { continue; }

      tt t( convert_hex2bin( tts[v] ) );

      assert( tt_num_vars( t ) == boost::out_degree( v, lut ) );

      if ( npn )
      {
        boost::dynamic_bitset<> phase;
        std::vector<unsigned> perm;
        tt t_npn;

        if ( tt_num_vars( t ) == 5u )
        {
          t_npn = exact_npn_canonization( t, phase, perm );
        }
        else
        {
          t_npn = npn_canonization_lucky( t, phase, perm );
        }

        os << t_npn << std::endl;

        assert( tt_num_vars( t ) <= tt_num_vars( t_npn ) );
      }
      else
      {
        os << t << std::endl;
      }
    }

    os.close();
  }

private:
  const lut_graph_t& lut;
  bool verbose;
  std::string dump_luts; /* if not empty, no mapping is performed but only luts are dumped */
  bool npn = true;
  bool noxor = false;

  std::unordered_map<std::string, xmg_graph> optimal_xmgs;
  std::vector<xmg_function> node_to_function;
  xmg_graph xmg;
  xmg_minlib_manager minlib;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

xmg_graph xmg_from_lut_mapping( const lut_graph_t& lut,
                                const properties::ptr& settings,
                                const properties::ptr& statistics )
{
  /* timer */
  properties_timer t( statistics );

  /* compute optimal XMGs for each LUT */
  xmg_from_lut_mapping_manager mgr( lut, settings );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
