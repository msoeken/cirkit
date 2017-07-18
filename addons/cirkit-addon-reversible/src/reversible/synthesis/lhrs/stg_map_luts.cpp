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

#include "stg_map_luts.hpp"

#include <core/utils/timer.hpp>
#include <classical/abc/gia/gia_utils.hpp>
#include <reversible/functions/add_gates.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class stg_map_luts_impl
{
public:
  stg_map_luts_impl( circuit& circ, const gia_graph& function,
                     const std::vector<unsigned>& line_map,
                     const std::vector<unsigned>& ancillas,
                     const stg_map_luts_params& params,
                     stg_map_luts_stats& stats )
    : circ( circ ),
      function( function ),
      line_map( line_map ),
      ancillas( ancillas ),
      params( params ),
      stats( stats )
  {
  }

  void run()
  {
    assert( !function.has_mapping() );

    const auto num_inputs = function.num_inputs();

    /* if very small fall back to precomputed database */
    if ( num_inputs <= params.max_cut_size )
    {
      auto tt = function.truth_table( 0 ).to_ulong();
      stg_map_precomp( circ, tt, num_inputs, line_map, *params.map_precomp_params, *stats.map_precomp_stats );
      return;
    }

    const auto mapping = compute_mapping();

    if ( !mapping.has_mapping() )
    {
      stg_map_esop( circ, mapping, line_map, *params.map_esop_params, *stats.map_esop_stats );
      return;
    }

    mapping.init_truth_tables();

    /* LUT based mapping */
    auto pi_index = 0u;
    auto anc_index = 0u;
    auto ins_index = 0u;
    std::vector<unsigned> lut_to_line( mapping.size() );
    std::vector<unsigned> synth_order( 2 * ( mapping.lut_count() - 1 ) + 1, 0 );

    mapping.foreach_input( [this, &lut_to_line, &pi_index]( int index, int e ) {
        lut_to_line[index] = line_map[pi_index++];
      } );

    auto root = abc::Gia_ObjFaninId0p( mapping, abc::Gia_ManCo( mapping, 0 ) );
    mapping.foreach_lut( [&]( int index ) {
        if ( index == root )
        {
          lut_to_line[index] = line_map[pi_index++];
          synth_order[ins_index] = index;
        }
        else
        {
          lut_to_line[index] = ancillas[anc_index++];
          synth_order[ins_index] = synth_order[synth_order.size() - 1 - ins_index] = index;
          ++ins_index;
        }
      } );

    std::unordered_map<unsigned, std::pair<unsigned, unsigned>> esop_circ_cache;

    for ( auto index : synth_order )
    {
      const auto num_inputs = mapping.lut_size( index );
      std::vector<unsigned> local_line_map;
      local_line_map.reserve( num_inputs + 1u );

      mapping.foreach_lut_fanin( index, [&local_line_map, &lut_to_line]( int fanin ) {
          local_line_map.push_back( lut_to_line[fanin] );
        } );
      local_line_map.push_back( lut_to_line[index] );

      if ( num_inputs == 0 )
      {
        assert( false );
      }
      else if ( num_inputs == 1 )
      {
        assert( mapping.lut_truth_table( index ) == 1 );
        append_cnot( circ, make_var( local_line_map[0], false ), local_line_map[1] );
      }
      else if ( num_inputs <= params.max_cut_size )
      {
        stg_map_precomp( circ, mapping.lut_truth_table( index ), num_inputs, local_line_map, *params.map_precomp_params, *stats.map_precomp_stats );
      }
      else
      {
        const auto it = esop_circ_cache.find( index );
        if ( it == esop_circ_cache.end() )
        {
          if ( params.map_esop_params->progress )
          {
            std::cout << "\n";
          }
          const auto lut = mapping.extract_lut( index );

          const auto begin = circ.num_gates();
          stg_map_esop( circ, lut, local_line_map, *params.map_esop_params, *stats.map_esop_stats );
          esop_circ_cache.insert( {index, {begin, circ.num_gates()}} );

          if ( params.map_esop_params->progress )
          {
            std::cout << "\e[A";
          }
        }
        else
        {
          for ( auto i = it->second.first; i < it->second.second; ++i )
          {
            circ.append_gate() = circ[i];
          }
        }
      }
    }
  }

private:
  gia_graph compute_mapping()
  {
    switch ( params.strategy )
    {
    case stg_map_luts_params::mapping_strategy::mindb:
      return compute_mapping_mindb();
    case stg_map_luts_params::mapping_strategy::bestfit:
      return compute_mapping_bestfit();
    default:
      assert( false );
    }
  }

  gia_graph compute_mapping_mindb()
  {
    const auto num_inputs = function.num_inputs();

    for ( auto k = 3; k <= params.max_cut_size; ++k )
    {
      const auto mapping = [this, k]() {
        increment_timer t( &stats.mapping_runtime );
        const auto mapping = function.if_mapping( make_settings_from( std::make_pair( "lut_size", static_cast<unsigned>( k ) ),
                                                                      "area_mapping",
                                                                      std::make_pair( "area_iters", params.area_iters ),
                                                                      std::make_pair( "flow_iters", params.flow_iters ) ) );
        if ( params.satlut )
        {
          mapping.satlut_mapping();
        }
        return mapping;
      }();

      if ( mapping.lut_count() - 1 <= static_cast<int>( ancillas.size() ) )
      {
        /* mapping is small enough, we're lucky */
        return mapping;
      }
      else if ( k == params.max_cut_size )
      {
        /* last round and no fit, we need to merge some LUTs */
        auto num_ancilla = mapping.lut_count() - 1; /* no need to store the output LUT */

        if ( ancillas.empty() )
        {
          /* mapping won't help, so we return our function */
          return function;
        }

        while ( num_ancilla > static_cast<int>( ancillas.size() ) )
        {
          abc::Gia_ManMergeTopLuts( mapping );
          --num_ancilla;
        }

        /* now we merged top LUTs, but the big LUT can now have more inputs than our function */
        auto root = abc::Gia_ObjFaninId0p( mapping, abc::Gia_ManCo( mapping, 0 ) );
        if ( mapping.lut_size( root ) > num_inputs )
        {
          /* mapping is not better than function */
          return function;
        }

        return mapping;
      }
      /* else we continue */
    }

    assert( false );
  }

  gia_graph compute_mapping_bestfit()
  {
    const auto num_inputs = function.num_inputs();

    for ( auto k = 4; k < num_inputs; ++k )
    {
      const auto mapping = [this, k]() {
        increment_timer t( &stats.mapping_runtime );
        const auto mapping = function.if_mapping( make_settings_from( std::make_pair( "lut_size", static_cast<unsigned>( k )  ),
                                                                     "area_mapping",
                                                                     std::make_pair( "area_iters", params.area_iters ),
                                                                     std::make_pair( "flow_iters", params.flow_iters ) ) );
        if ( k <= 6 && params.satlut )
        {
          mapping.satlut_mapping();
        }
        return mapping;
      }();

      /* Do we have enough ancillas? */
      if ( mapping.lut_count() - 1 <= static_cast<int>( ancillas.size() )  )
      {
        return mapping;
      }
    }

    /* if no mapping fits, we return the function */
    return function;
  }

private:
  circuit& circ;
  const gia_graph& function;
  const std::vector<unsigned>& line_map;
  const std::vector<unsigned>& ancillas;
  const stg_map_luts_params& params;
  stg_map_luts_stats& stats;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void stg_map_luts( circuit& circ, const gia_graph& function,
                   const std::vector<unsigned>& line_map,
                   const std::vector<unsigned>& ancillas,
                   const stg_map_luts_params& params,
                   stg_map_luts_stats& stats )
{
  stg_map_luts_impl impl( circ, function, line_map, ancillas, params, stats );
  impl.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
