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

#include "exorcism_minimization.hpp"

#include <fcntl.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <core/io/pla_parser.hpp>
#include <core/utils/bdd_utils.hpp>
#include <core/utils/system_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/abc_api.hpp>
#include <classical/abc/abc_manager.hpp>
#include <classical/abc/functions/cirkit_to_gia.hpp>

#include <base/exor/exor.h>
#include <misc/vec/vecInt.h>
#include <misc/vec/vecWec.h>

namespace abc
{
extern cinfo g_CoverInfo;
extern int s_fDecreaseLiterals;
int Abc_ExorcismMain( Vec_Wec_t * vEsop, int nIns, int nOuts, char * pFileNameOut, int Quality, int Verbosity, int nCubesMax, int fUseQCost );
Gia_Man_t * Eso_ManCompute( Gia_Man_t * pGia, int fVerbose, Vec_Wec_t ** pvRes );

void AddCubesToStartingCover( Vec_Wec_t * vEsop );
int ReduceEsopCover();

Cube* IterCubeSetStart();
Cube* IterCubeSetNext();
varvalue GetVar( Cube * pC, int Var );
}

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class exorcism_processor : public pla_processor
{
public:
  exorcism_processor( const cube_function_t& on_cube_f ) : on_cube_f( on_cube_f ) {}

  void on_cube( const std::string& in, const std::string& out )
  {
    /* only one output functions are supported */
    assert( out.size() == 1u );

    unsigned n = in.size();
    boost::dynamic_bitset<> values( n );
    boost::dynamic_bitset<> care( n );

    for ( unsigned i = 0u; i < n; ++i )
    {
      values[i] = ( in[i] == '1' );
      care[i]   = ( in[i] != '-' );
    }

    cube_t cube = std::make_pair( values, care );

    if ( on_cube_f )
    {
      on_cube_f( cube );
    }

    ++_cube_count;
    _literal_count += care.count();
  }

  unsigned cube_count() const
  {
    return _cube_count;
  }

  unsigned literal_count() const
  {
    return _literal_count;
  }

private:
  const cube_function_t& on_cube_f;
  unsigned _cube_count = 0u;
  unsigned _literal_count = 0u;
};

/******************************************************************************
 * Exorcism optimization scripts                                              *
 ******************************************************************************/

std::istream& operator>>( std::istream& in, exorcism_script& script )
{
  std::string token;
  in >> token;
  if ( token == "none" || token == "0" )
  {
    script = exorcism_script::none;
  }
  else if ( token == "def" || token == "1" )
  {
    script = exorcism_script::def;
  }
  else if ( token == "def_wo4" || token == "2" )
  {
    script = exorcism_script::def_wo4;
  }
  else if ( token == "j2r" || token == "3" )
  {
    script = exorcism_script::j2r;
  }
  else
  {
    in.setstate( std::ios_base::failbit );
  }
  return in;
}

std::ostream& operator<<( std::ostream& out, const exorcism_script& script )
{
  switch ( script )
  {
  case exorcism_script::none:
    return out << "none";
  case exorcism_script::def:
    return out << "def";
  case exorcism_script::def_wo4:
    return out << "def_wo4";
  case exorcism_script::j2r:
    return out << "j2r";
  }

  return out;
}

void reduce_cover_script_def( bool progress )
{
  int gain_total{};
  int iter_wo_improv = 0;

  unsigned iteration = 0;
  double runtime = 0.0;

  progress_line p( "[i] exorcism   iter = %3d   i/o = %2d/%2d   cubes = %6d/%6d   total = %6.2f", progress );

  do
  {
    increment_timer t( &runtime );
    p( ++iteration, abc::g_CoverInfo.nVarsIn, abc::g_CoverInfo.nVarsOut, abc::g_CoverInfo.nCubesInUse, abc::g_CoverInfo.nCubesBefore, runtime );

    gain_total = 0;

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    if ( iter_wo_improv > (int)(abc::g_CoverInfo.Quality>0) )
    {
      gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink4( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink4( 1|2|0 );

      gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink4( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink4( 1|2|0 );
    }

    if ( gain_total )
    {
      iter_wo_improv = 0;
    }
    else
    {
      ++iter_wo_improv;
    }
  } while ( iter_wo_improv < abc::g_CoverInfo.Quality + 1 );

  abc::s_fDecreaseLiterals = 1;
  for ( auto z = 0; z < 1; z++ )
  {
    gain_total  = 0;
    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
  }
}

void reduce_cover_script_def_wo4( bool progress )
{
  int gain_total{};
  int iter_wo_improv = 0;

  unsigned iteration = 0;
  double runtime = 0.0;

  progress_line p( "[i] exorcism   iter = %3d   i/o = %2d/%2d   cubes = %6d/%6d   total = %6.2f", progress );

  do
  {
    increment_timer t( &runtime );
    p( ++iteration, abc::g_CoverInfo.nVarsIn, abc::g_CoverInfo.nVarsOut, abc::g_CoverInfo.nCubesInUse, abc::g_CoverInfo.nCubesBefore, runtime );

    gain_total = 0;

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    if ( iter_wo_improv > (int)(abc::g_CoverInfo.Quality>0) )
    {
      gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );

      gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink3( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
      gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
    }

    if ( gain_total )
    {
      iter_wo_improv = 0;
    }
    else
    {
      ++iter_wo_improv;
    }
  } while ( iter_wo_improv < abc::g_CoverInfo.Quality + 1 );

  abc::s_fDecreaseLiterals = 1;
  for ( auto z = 0; z < 1; z++ )
  {
    gain_total  = 0;
    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
  }
}

void reduce_cover_script_j2r( bool progress )
{
  int gain_total{};

  unsigned iteration = 0;
  double runtime = 0.0;

  progress_line p( "[i] exorcism   iter = %3d   i/o = %2d/%2d   cubes = %6d/%6d   total = %6.2f", progress );

  while ( iteration < 2u )
  {
    increment_timer t( &runtime );
    p( ++iteration, abc::g_CoverInfo.nVarsIn, abc::g_CoverInfo.nVarsOut, abc::g_CoverInfo.nCubesInUse, abc::g_CoverInfo.nCubesBefore, runtime );

    gain_total = 0;

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|4 );
    gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
    gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|4 );
    gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
    gain_total += abc::IterativelyApplyExorLink2( 1|2|4 );
  }

  abc::s_fDecreaseLiterals = 1;
  for ( auto z = 0; z < 1; z++ )
  {
    gain_total  = 0;
    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );

    gain_total += abc::IterativelyApplyExorLink2( 1|2|0 );
    gain_total += abc::IterativelyApplyExorLink3( 1|2|0 );
  }
}

void reduce_cover( bool progress, exorcism_script script )
{
  switch ( script )
  {
  case exorcism_script::none:
    break;
  case exorcism_script::def:
    reduce_cover_script_def( progress );
    break;
  case exorcism_script::def_wo4:
    reduce_cover_script_def_wo4( progress );
    break;
  case exorcism_script::j2r:
    reduce_cover_script_j2r( progress );
    break;
  }
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void exorcism_minimization( DdManager * cudd, DdNode * f, const properties::ptr& settings, const properties::ptr& statistics )
{
  return exorcism_minimization( bdd_to_cubes( cudd, f ), settings, statistics );
}

void exorcism_minimization( const cube_vec_t& cubes, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto verbose      = get( settings, "verbose",      false );
  const auto on_cube      = get( settings, "on_cube",      cube_function_t() );
  const auto esopname     = get( settings, "esopname",     std::string( "/tmp/test.esop" ) );
  const auto skip_parsing = get( settings, "skip_parsing", false );

  properties_timer t( statistics );

  if ( cubes.empty() )
  {
    return;
  }

  abc::Vec_Wec_t *esop = abc::Vec_WecAlloc( 0u );

  for ( const auto& cube : cubes )
  {
    auto * level = abc::Vec_WecPushLevel( esop );

    for ( auto i = 0u; i < cube.length(); ++i )
    {
      if ( !cube.care()[i] ) { continue; }

      abc::Vec_IntPush( level, ( i << 1u ) | !cube.bits()[i] );
    }
    abc::Vec_IntPush( level, -1 );
  }

  if ( verbose )
  {
    abc::Vec_WecPrint( esop, 0 );
  }

  /* redict STDOUT because of one output line in Abc_ExorcismMain */
  abc::Abc_ExorcismMain( esop, cubes.front().length(), 1, const_cast<char*>( esopname.c_str() ), 2, verbose, 20000, 0 );

  abc::Vec_WecFree( esop );

  /* Parse */
  if ( !skip_parsing )
  {
    exorcism_processor p( on_cube );
    pla_parser( esopname, p );

    set( statistics, "cube_count", p.cube_count() );
    set( statistics, "literal_count", p.literal_count() );
  }
}

gia_graph::esop_ptr exorcism_minimization( const gia_graph::esop_ptr& esop, unsigned ninputs, unsigned noutputs,
                                           const properties::ptr& settings,
                                           const properties::ptr& statistics )
{
  /* settings */
  const auto quality      = get( settings, "quality",      2u );
  const auto cubes_max    = get( settings, "cubes_max",    1000000u );
  const auto script       = get( settings, "script",       exorcism_script::def_wo4 );
  const auto progress     = get( settings, "progress",     false );
  const auto verbose      = get( settings, "verbose",      false );
  const auto very_verbose = get( settings, "very_verbose", false );

  properties_timer t( statistics );

  /* initialize */
  memset( &abc::g_CoverInfo, 0, sizeof( abc::cinfo ) );
  abc::g_CoverInfo.Quality = static_cast<int>( quality );
  abc::g_CoverInfo.Verbosity = very_verbose ? 2 : ( verbose ? 1 : 0 );
  abc::g_CoverInfo.nCubesMax = static_cast<int>( cubes_max );
  abc::PrepareBitSetModule();

  int rbits = ( ninputs * 2 ) % ( sizeof(unsigned) * 8 );
  int twords = ( ninputs * 2 ) / ( sizeof(unsigned) * 8 ) + ( rbits > 0 );
  abc::g_CoverInfo.nVarsIn = ninputs;
  abc::g_CoverInfo.nWordsIn = twords;

  rbits = ( noutputs * 2 ) % ( sizeof(unsigned) * 8 );
  twords = ( noutputs * 2 ) / ( sizeof(unsigned) * 8 ) + ( rbits > 0 );
  abc::g_CoverInfo.nVarsOut = noutputs;
  abc::g_CoverInfo.nWordsOut = twords;
  abc::g_CoverInfo.cIDs = 1;

  abc::g_CoverInfo.nCubesBefore = abc::Vec_WecSize( esop.get() );

  if ( abc::g_CoverInfo.nCubesBefore > abc::g_CoverInfo.nCubesMax )
  {
    std::cout << boost::format( "[e] the size of the starting cover is too large: %d (allowed: %d)" )
      % abc::g_CoverInfo.nCubesBefore % abc::g_CoverInfo.nCubesMax << std::endl;
    return gia_graph::esop_ptr( nullptr, &abc::Vec_WecFree );
  }

  /* prepare internal data structures */
  abc::g_CoverInfo.nCubesAlloc = abc::g_CoverInfo.nCubesBefore + ADDITIONAL_CUBES;
  if ( !abc::AllocateCover( abc::g_CoverInfo.nCubesAlloc, abc::g_CoverInfo.nWordsIn, abc::g_CoverInfo.nWordsOut ) )
  {
    std::cout << "[e] not enough memory to allocate cover" << std::endl;
    return gia_graph::esop_ptr( nullptr, &abc::Vec_WecFree );
  }

  abc::AllocateCubeSets( abc::g_CoverInfo.nVarsIn, abc::g_CoverInfo.nVarsOut );

  if ( !abc::AllocateQueques( abc::g_CoverInfo.nCubesAlloc * 4 ) )
  {
    std::cout << "[e] not enough memory to allocate queques" << std::endl;
    return gia_graph::esop_ptr( nullptr, &abc::Vec_WecFree );
  }

  /* reduce */
  abc::AddCubesToStartingCover( esop.get() );
  {
    properties_timer t( statistics, "exorcism_opt_time" );
    reduce_cover( progress, script );
  }

  /* extract cover */
  abc::Vec_Wec_t* esop_opt;
  abc::Vec_Int_t* level;
  esop_opt = abc::Vec_WecAlloc( abc::g_CoverInfo.nCubesInUse );

  abc::Cube *p;
  for ( p = abc::IterCubeSetStart(); p; p = abc::IterCubeSetNext() )
  {
    level = abc::Vec_WecPushLevel( esop_opt );

    for ( auto v = 0u; v < ninputs; ++v )
    {
      switch ( GetVar( p, v ) )
      {
      case abc::VAR_NEG: abc::Vec_IntPush( level, abc::Abc_Var2Lit( v, 1 ) ); break;
      case abc::VAR_POS: abc::Vec_IntPush( level, abc::Abc_Var2Lit( v, 0 ) ); break;
      case abc::VAR_ABS: break;
      }
    }

    auto coutputs = 0u;
    const auto wordsize = 8 * sizeof( unsigned );
    for ( auto w = 0; w < abc::g_CoverInfo.nWordsOut; ++w )
    {
      for ( auto v = 0ul; v < wordsize; ++v )
      {
        if ( p->pCubeDataOut[w] & ( 1 << v ) )
        {
          abc::Vec_IntPush( level, -coutputs - 1 );
        }
        if ( ++coutputs == noutputs ) break;
      }
    }
  }

  abc::DelocateCubeSets();
  abc::DelocateCover();
  abc::DelocateQueques();

  return gia_graph::esop_ptr( esop_opt, &abc::Vec_WecFree );
}

gia_graph::esop_ptr exorcism_minimization( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  const auto cover_method = get( settings, "cover_method", gia_graph::esop_cover_method::aig );
  const auto& esop = gia.compute_esop_cover( cover_method );
  return exorcism_minimization( esop, gia.num_inputs(), gia.num_outputs(), settings, statistics );
}

void write_esop( const gia_graph::esop_ptr& esop, unsigned ninputs, unsigned noutputs, const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );
  write_esop( esop, ninputs, noutputs, os );
}

void write_esop( const gia_graph::esop_ptr& esop, unsigned ninputs, unsigned noutputs, std::ostream& os )
{
  os << boost::format( ".i %d" ) % ninputs << std::endl
     << boost::format( ".o %d" ) % noutputs << std::endl
     << boost::format( ".p %d" ) % abc::Vec_WecSize( esop.get() ) << std::endl
     << ".type esop" << std::endl;

  /* cubes */
  int i;
  abc::Vec_Int_t *vec;
  Vec_WecForEachLevel( esop.get(), vec, i )
  {
    std::string cubein( ninputs, '-' );
    std::string cubeout( noutputs, '0' );

    /* controls */
    for ( auto j = 0; j < abc::Vec_IntSize( vec ); ++j )
    {
      const auto lit = abc::Vec_IntEntry( vec, j );

      if ( lit < 0 )
      {
        const auto out = -lit - 1;
        cubeout[out] = '1';
      }
      else
      {
        const auto in = abc::Abc_Lit2Var( lit );
        cubein[in] = abc::Abc_LitIsCompl( lit ) ? '0' : '1';
      }
    }

    os << boost::format( "%s %s" ) % cubein % cubeout << std::endl;
  }

  os << ".e" << std::endl;
}

dd_based_esop_optimization_func dd_based_exorcism_minimization_func(properties::ptr settings, properties::ptr statistics)
{
  dd_based_esop_optimization_func f = [&settings, &statistics]( DdManager * cudd, DdNode * f ) {
    return exorcism_minimization( cudd, f, settings, statistics );
  };
  f.init( settings, statistics );
  return f;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
