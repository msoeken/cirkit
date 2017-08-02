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

#include "xmg_rewrite.hpp"

#include <core/utils/range_utils.hpp>
#include <core/utils/timer.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

std::map<xmg_node, xmg_function> init_visited_table( const xmg_graph& xmg, xmg_graph& xmg_new )
{
  std::map<xmg_node, xmg_function> old_to_new;

  old_to_new[0] = xmg_new.get_constant( false );
  for ( const auto& pi : xmg.inputs() )
  {
    old_to_new[pi.first] = xmg_new.create_pi( pi.second );
  }

  return old_to_new;
}

xmg_function xmg_rewrite_top_down_rec( const xmg_graph& xmg, xmg_node node,
                                       xmg_graph& xmg_new,
                                       const maj_rewrite_func_t& on_maj,
                                       const xor_rewrite_func_t& on_xor,
                                       std::map<xmg_node, xmg_function>& old_to_new,
                                       const xmg_substitutes_map_t& substitutes )
{
  /* reroute node if it is in substutitutes */
  auto complement = false;
  xmg_substitutes_map_t::value_type::const_iterator it_s{};
  if ( substitutes && ( it_s = substitutes->find( node ) ) != substitutes->end() )
  {
    node = it_s->second.node;
    complement = it_s->second.complemented;
  }

  /* visited */
  const auto it = old_to_new.find( node );
  if ( it != old_to_new.end() )
  {
    return it->second;
  }

  xmg_function f;
  if ( xmg.is_maj( node ) )
  {
    const auto c = xmg.children( node );
    f = on_maj( xmg_new,
                xmg_rewrite_top_down_rec( xmg, c[0].node, xmg_new, on_maj, on_xor, old_to_new, substitutes ) ^ c[0].complemented,
                xmg_rewrite_top_down_rec( xmg, c[1].node, xmg_new, on_maj, on_xor, old_to_new, substitutes ) ^ c[1].complemented,
                xmg_rewrite_top_down_rec( xmg, c[2].node, xmg_new, on_maj, on_xor, old_to_new, substitutes ) ^ c[2].complemented );
  }
  else if ( xmg.is_xor( node ) )
  {
    const auto c = xmg.children( node );
    f = on_xor( xmg_new,
                xmg_rewrite_top_down_rec( xmg, c[0].node, xmg_new, on_maj, on_xor, old_to_new, substitutes ) ^ c[0].complemented,
                xmg_rewrite_top_down_rec( xmg, c[1].node, xmg_new, on_maj, on_xor, old_to_new, substitutes ) ^ c[1].complemented );
  }
  else
  {
    /* cannot happen */
    assert( false );
  }

  f.complemented = ( f.complemented != complement ); /* Boolean XOR */
  old_to_new.insert( {node, f} );
  return f;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

xmg_function rewrite_default_maj( xmg_graph& xmg_new, const xmg_function& a, const xmg_function& b, const xmg_function& c )
{
  return xmg_new.create_maj( a, b, c );
}

xmg_function rewrite_default_xor( xmg_graph& xmg_new, const xmg_function& a, const xmg_function& b )
{
  return xmg_new.create_xor( a, b );
}

xmg_graph xmg_rewrite_top_down( const xmg_graph& xmg,
                                const maj_rewrite_func_t& on_maj,
                                const xor_rewrite_func_t& on_xor,
                                const properties::ptr& settings,
                                const properties::ptr& statistics )
{
  /* settings */
  const auto prefill     = get( settings, "prefill",     prefill_func_t() );
  const auto init        = get( settings, "init",        xmg_init_func_t() );
  const auto substitutes = get( settings, "substitutes", xmg_substitutes_map_t() );

  /* statistics */
  properties_timer t( statistics );

  xmg_graph xmg_new( xmg.name() );

  /* init */
  if ( init )
  {
    init( xmg_new );
  }

  /* create constant and PIs */
  auto old_to_new = init_visited_table( xmg, xmg_new );

  /* prefill */
  if ( prefill )
  {
    prefill( xmg_new, old_to_new );
  }

  /* map nodes */
  for ( const auto& po : xmg.outputs() )
  {
    xmg_new.create_po( xmg_rewrite_top_down_rec( xmg, po.first.node, xmg_new, on_maj, on_xor, old_to_new, substitutes ) ^ po.first.complemented, po.second );
  }

  return xmg_new;
}

std::vector<xmg_function> xmg_rewrite_top_down_inplace( xmg_graph& dest,
                                                        const xmg_graph& xmg,
                                                        const maj_rewrite_func_t& on_maj,
                                                        const xor_rewrite_func_t& on_xor,
                                                        const std::vector<xmg_function>& pi_mapping,
                                                        const properties::ptr& settings,
                                                        const properties::ptr& statistics )
{
  /* settings */
  const auto prefill = get( settings, "prefill", prefill_func_t() );

  /* statistics */
  properties_timer t( statistics );

  /* create constant and PIs */
  std::map<xmg_node, xmg_function> old_to_new;

  old_to_new[0] = dest.get_constant( false );
  for ( const auto& pi : index( xmg.inputs() ) )
  {
    old_to_new[pi.value.first] = pi_mapping[pi.index];
  }

  /* prefill */
  if ( prefill )
  {
    prefill( dest, old_to_new );
  }

  /* map nodes */
  std::vector<xmg_function> outputs;
  for ( const auto& po : xmg.outputs() )
  {
    outputs.push_back( xmg_rewrite_top_down_rec( xmg, po.first.node, dest, on_maj, on_xor, old_to_new, boost::none ) ^ po.first.complemented );
  }

  return outputs;
}

xmg_graph xmg_rewrite_bottom_up( const xmg_graph& xmg,
                                 const maj_rewrite_func_t& on_maj,
                                 const xor_rewrite_func_t& on_xor,
                                 const properties::ptr& settings,
                                 const properties::ptr& statistics )
{
  /* settings */
  const auto init    = get( settings, "init",    xmg_init_func_t() );

  /* statistics */
  properties_timer t( statistics );

  xmg_graph xmg_new( xmg.name() );

  /* init */
  if ( init )
  {
    init( xmg_new );
  }

  /* create constant and PIs */
  auto old_to_new = init_visited_table( xmg, xmg_new );

  /* map nodes */
  for ( auto node : xmg.topological_nodes() )
  {
    xmg_function f;
    if ( xmg.is_maj( node ) )
    {
      const auto c = xmg.children( node );
      f = on_maj( xmg_new,
                  old_to_new[c[0].node] ^ c[0].complemented,
                  old_to_new[c[1].node] ^ c[1].complemented,
                  old_to_new[c[2].node] ^ c[2].complemented );
    }
    else if ( xmg.is_xor( node ) )
    {
      const auto c = xmg.children( node );
      f = on_xor( xmg_new,
                  old_to_new[c[0].node] ^ c[0].complemented,
                  old_to_new[c[1].node] ^ c[1].complemented );
    }
    else
    {
      continue;
    }
    old_to_new[node] = f;
  }

  for ( const auto& po : xmg.outputs() )
  {
    xmg_new.create_po( old_to_new[po.first.node] ^ po.first.complemented, po.second );
  }

  return xmg_new;
}

xmg_graph xmg_strash( const xmg_graph& xmg, const properties::ptr& settings, const properties::ptr& statistics )
{
  return xmg_rewrite_top_down( xmg, rewrite_default_maj, rewrite_default_xor, settings, statistics );
}

xmg_graph xmg_merge( const xmg_graph& xmg1, const xmg_graph& xmg2, const properties::ptr& settings, const properties::ptr& statistics )
{
  auto xmg = xmg_strash( xmg1 );

  std::vector<xmg_function> pi_mapping;
  for ( const auto& pi : xmg.inputs() )
  {
    pi_mapping.push_back( xmg_function( pi.first, false ) );
  }

  const auto outputs = xmg_rewrite_top_down_inplace( xmg, xmg2, rewrite_default_maj, rewrite_default_xor, pi_mapping, settings, statistics );

  for ( const auto& po : index( xmg2.outputs() ) )
  {
    xmg.create_po( outputs[po.index], po.value.second );
  }

  return xmg;
}

xmg_graph xmg_to_mig( const xmg_graph& xmg, const properties::ptr& settings, const properties::ptr& statistics )
{
  settings->set( "init", xmg_init_func_t( []( xmg_graph& xmg_new ) { xmg_new.set_native_xor( false ); } ) );
  return xmg_strash( xmg, settings, statistics );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
