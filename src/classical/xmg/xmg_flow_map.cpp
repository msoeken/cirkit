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

#include "xmg_flow_map.hpp"

#include <vector>

#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>

#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/xmg/xmg_cover.hpp>
#include <classical/xmg/xmg_cuts_paged.hpp>
#include <classical/xmg/xmg_utils.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#define L(x) if ( verbose ) { std::cout << x; }
#define LN(x) if ( verbose ) { std::cout << x << std::endl; }

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class xmg_flow_map_manager
{
public:
  xmg_flow_map_manager( xmg_graph& xmg, const properties::ptr& settings );

  void run();

private:
  void find_best_cuts();
  void extract_cover();

private:
  xmg_graph&            xmg;
  std::vector<unsigned> node_to_cut;
  std::vector<unsigned> node_to_level;

  std::shared_ptr<xmg_cuts_paged> cuts;

  /* settings */
  unsigned cut_size;
  bool     progress;
  bool     verbose;
};

xmg_flow_map_manager::xmg_flow_map_manager( xmg_graph& xmg, const properties::ptr& settings )
  : xmg( xmg ),
    node_to_cut( xmg.size() ),
    node_to_level( xmg.size() )
{
  cut_size = get( settings, "cut_size", 4u );
  progress = get( settings, "progress", false );
  verbose  = get( settings, "verbose",  false );
}

void xmg_flow_map_manager::run()
{
  /* compute cuts */
  auto cuts_settings = std::make_shared<properties>();
  cuts_settings->set( "progress", progress );

  cuts = std::make_shared<xmg_cuts_paged>( xmg, cut_size, cuts_settings );
  LN( boost::format( "[i] enumerated %d cuts in %.2f secs" ) % cuts->total_cut_count() % cuts->enumeration_time() );

  find_best_cuts();
  extract_cover();
}

void xmg_flow_map_manager::find_best_cuts()
{
  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( xmg.size(), progress ? std::cout : null_out );

  for ( auto node : xmg.topological_nodes() )
  {
    ++show_progress;

    if ( xmg.is_input( node ) )
    {
      assert( cuts->count( node ) == 1u );

      node_to_cut[node] = cuts->cuts( node ).front().address();
      node_to_level[node] = 0u;
    }
    else
    {
      auto best_level = std::numeric_limits<unsigned>::max();
      auto best_cut = 0u;

      for ( const auto& cut : cuts->cuts( node ) )
      {
        if ( cut.size() == 1u ) { continue; } /* ignore singleton cuts */

        auto local_max_level = 0u;

        for ( auto leaf : cut )
        {
          local_max_level = std::max( local_max_level, node_to_level[leaf] );
        }

        if ( local_max_level < best_level )
        {
          best_level = local_max_level;
          best_cut = cut.address();
        }
      }

      node_to_cut[node] = best_cut;
      node_to_level[node] = best_level + 1u;
    }
  }
}

void xmg_flow_map_manager::extract_cover()
{
  boost::dynamic_bitset<> visited( xmg.size() );

  xmg_cover cover( cut_size, xmg );
  auto deque = xmg_output_deque( xmg );

  while ( !deque.empty() )
  {
    auto node = deque.front();
    deque.pop_front();

    if ( xmg.is_input( node ) || visited[node] ) { continue; }
    visited[node] = true;

    auto cut = cuts->from_address( node_to_cut[node] );
    cover.add_cut( node, cut );

    for ( auto leaf : cut )
    {
      deque.push_back( leaf );
    }
  }

  xmg.set_cover( cover );
}

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void xmg_flow_map( xmg_graph& xmg, const properties::ptr& settings, const properties::ptr& statistics )
{
  xmg_flow_map_manager mgr( xmg, settings );

  properties_timer t( statistics );
  mgr.run();
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
