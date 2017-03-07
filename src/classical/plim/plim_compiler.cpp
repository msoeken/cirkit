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

#include "plim_compiler.hpp"

#include <map>
#include <queue>
#include <stack>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>

#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/functions/compute_levels.hpp>
#include <classical/utils/memristor_costs.hpp>
#include <classical/mig/mig_utils.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

#define L(x) if ( verbose ) { std::cout << x << std::endl; }

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

template<typename IndexType>
class auto_index_generator
{
public:
  enum class request_strategy { lifo, fifo };

  auto_index_generator( request_strategy strategy )
    : strategy( strategy )
  {
  }

  IndexType request()
  {
    if ( !free.empty() )
    {
      IndexType index;

      if ( strategy == request_strategy::lifo )
      {
        index = free.back();
        free.pop_back();
      }
      else
      {
        index = free.front();
        free.pop_front();
      }

      return index;
    }
    else
    {
      return IndexType::from_index( ++max );
    }
  }

  void release( IndexType i )
  {
    free.push_back( i );
  }

private:
  request_strategy strategy;
  unsigned max = 0u;
  std::deque<IndexType> free;
};

struct compilation_compare
{
  compilation_compare( const mig_graph& mig, bool enable = true )
    : mig( mig ),
      _fanout_levels( num_vertices( mig ) ),
      enable( enable )
  {
    fanouts = precompute_ingoing_edges( mig );
    _fanout_count = precompute_in_degrees( mig );
    levels = compute_levels( mig, max_level );

    using namespace std::placeholders;
    boost::transform( boost::make_iterator_range( boost::vertices( mig ) ), _fanout_levels.begin(), std::bind( &compilation_compare::fanout_levels, this, _1 ) );
  }

  bool operator()( mig_node a, mig_node b ) const
  {
    if ( !enable ) { return a > b; }

    /* return "false" means a is preferred over b */
    /* return "true" means a is worse than b */
    const auto nfa = number_of_releasing_fanins( a );
    const auto nfb = number_of_releasing_fanins( b );
    if ( nfa != nfb )
    {
      return nfa < nfb;
    }

    const auto fla = _fanout_levels[a];
    const auto flb = _fanout_levels[b];

    if ( fla.second < flb.first )
    {
      return true;
    }

    if ( flb.second < fla.first )
    {
      return false;
    }

    /* last resort, nodes are incomparable, just pick id */
    return a > b;
  }

  inline unsigned fanout_count( mig_node a ) const { return _fanout_count[a]; }
  inline unsigned remove_fanout( mig_node a ) { return --_fanout_count[a]; }

private:
  unsigned number_of_releasing_fanins( mig_node a ) const
  {
    auto sum = 0u;
    for ( const auto& c : get_children( mig, a ) )
    {
      if ( _fanout_count[c.node] == 1u )
      {
        ++sum;
      }
    }
    return sum;
  }

  std::pair<unsigned, unsigned> fanout_levels( mig_node a ) const
  {
    auto min = 0u;
    auto max = max_level;

    const auto it = fanouts.find( a );

    if ( it != fanouts.end() )
    {
      for ( const auto& e : it->second )
      {
        const auto parent = boost::source( e, mig );
        const auto it2 = levels.find( parent );
        if ( it2 == levels.end() ) continue;
        const auto level = it2->second;
        min = std::min( min, level );
        max = std::max( max, level );
      }
    }

    return std::make_pair( min, max );
  }

private:
  const mig_graph& mig;
  std::map<mig_node, std::vector<mig_edge>> fanouts;
  std::vector<std::pair<unsigned, unsigned>> _fanout_levels;
  std::vector<unsigned> _fanout_count;
  unsigned max_level;
  std::map<mig_node, unsigned> levels;
  bool enable = true;
};

bool all_children_computed( mig_node n, const mig_graph& mig, const boost::dynamic_bitset<>& computed )
{
  for ( const auto& adj : boost::make_iterator_range( boost::adjacent_vertices( n, mig ) ) )
  {
    if ( !computed[adj] ) { return false; }
  }
  return true;
  //const auto children = get_children( mig, n );

  //return computed[children[0].node] && computed[children[1].node] && computed[children[2].node];
}

inline std::pair<unsigned, unsigned> three_without( unsigned x )
{
  return std::make_pair( x == 0u ? 1u : 0u, x == 2u ? 1u : 2u );
}

}

namespace std
{

template<>
struct hash<cirkit::mig_function>
{
  std::size_t operator()( const cirkit::mig_function& f ) const
  {
    return std::hash<unsigned>()( ( f.node << 1u ) | f.complemented );
  }
};

}

namespace cirkit
{

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

plim_program
compile_for_plim( const mig_graph& mig,
                  const properties::ptr& settings,
                  const properties::ptr& statistics )
{
  /* settings */
  const auto verbose              = get( settings, "verbose", false );
  const auto progress             = get( settings, "progress", false );
  const auto enable_cost_function = get( settings, "enable_cost_function", true );
  const auto generator_strategy   = get( settings, "generator_strategy", 0u ); /* 0u: LIFO, 1u: FIFO */

  /* timing */
  properties_timer t( statistics );

  plim_program program;

  const auto& info = mig_info( mig );

  boost::dynamic_bitset<>                 computed( num_vertices( mig ) );
  std::unordered_map<mig_function, memristor_index> func_to_rram;
  auto_index_generator<memristor_index> memristor_generator(
      generator_strategy == 0u
          ? auto_index_generator<memristor_index>::request_strategy::lifo
          : auto_index_generator<memristor_index>::request_strategy::fifo );

  /* constant and all PIs are computed */
  computed.set( info.constant );
  for ( const auto& input : info.inputs )
  {
    computed.set( input );
    func_to_rram.insert( {{input, false}, memristor_generator.request()} );
  }

  /* keep a priority queue for candidates
     invariant: candidates elements' children are all computed */
  compilation_compare cmp( mig, enable_cost_function );
  std::priority_queue<mig_node, std::vector<mig_node>, compilation_compare> candidates( cmp );

  /* find initial candidates */
  for ( const auto& node : boost::make_iterator_range( vertices( mig ) ) )
  {
    /* PI and constant cannot be candidate */
    if ( out_degree( node, mig ) == 0 ) { continue; }

    if ( all_children_computed( node, mig, computed ) )
    {
      candidates.push( node );
    }
  }

  const auto parent_edges = precompute_ingoing_edges( mig );

  null_stream ns;
  std::ostream null_out( &ns );
  boost::progress_display show_progress( num_vertices( mig ), progress ? std::cout : null_out );

  /* synthesis loop */
  while ( !candidates.empty() )
  {
    ++show_progress;

    /* pick the best candidate */
    auto candidate = candidates.top();
    candidates.pop();

    L( "[i] compute node " << candidate );

    /* perform computation (e.g. mark which RRAM is used for this node) */
    const auto children = get_children( mig, candidate );
    boost::dynamic_bitset<> children_compl( 3u );
    for ( const auto& f : index( children ) )
    {
      children_compl.set( f.index, f.value.complemented );
    }

    /* indexes and registers */
    auto i_src_pos = 3u, i_src_neg = 3u, i_dst = 3u;
    plim_program::operand_t src_pos;
    plim_program::operand_t src_neg;
    memristor_index         dst;

    /* find the inverter */
    /* if there is one inverter */
    if ( children_compl.count() == 1u )
    {
      i_src_neg = children_compl.find_first();

      if ( children[i_src_neg].node == 0u )
      {
        src_neg = false;
      }
      else
      {
        src_neg = func_to_rram.at( {children[i_src_neg].node, false} );
      }
    }
    /* if there are more than one inverters, but one of them is a constant */
    else if ( children_compl.count() > 1u && children[children_compl.find_first()].node == 0u )
    {
      i_src_neg = children_compl.find_next( children_compl.find_first() );
      src_neg = func_to_rram.at( {children[i_src_neg].node, false} );
    }
    /* if there is no inverter but a constant */
    else if ( children_compl.count() == 0u && children[0u].node == 0u )
    {
      i_src_neg = 0u;
      src_neg = !children[0u].complemented;
    }
    /* if there are more than one inverters */
    else if ( children_compl.count() > 1u )
    {
      do /* in order to escape early */
      {
        /* pick an input that has multiple fanout */
        for ( auto i = 0u; i < 3u; ++i )
        {
          if ( !children_compl[i] ) continue;

          if ( cmp.fanout_count( children[i].node ) > 1u )
          {
            i_src_neg = i;
            src_neg = func_to_rram.at( {children[i_src_neg].node, false} );
            break;
          }
        }

        if ( i_src_neg < 3u ) { break; }

        i_src_neg = children_compl.find_first();
        src_neg = func_to_rram.at( {children[i_src_neg].node, false} );
      } while ( false );
    }
    /* if there is no inverter */
    else
    {
      do /* in order to escape early */
      {
        /* pick an input that has multiple fanout */
        for ( auto i = 0u; i < 3u; ++i )
        {
          const auto it_reg = func_to_rram.find( {children[i].node, true} );
          if ( it_reg != func_to_rram.end() )
          {
            i_src_neg = i;
            src_neg = it_reg->second;
            break;
          }
        }

        if ( i_src_neg < 3u ) { break; }

        /* pick an input that has multiple fanout */
        for ( auto i = 0u; i < 3u; ++i )
        {
          if ( cmp.fanout_count( children[i].node ) > 1u )
          {
            i_src_neg = i;
            break;
          }
        }

        /* or pick the first one */
        if ( i_src_neg == 3u ) { i_src_neg = 0u; }

        /* create new register for inversion */
        const auto inv_result = memristor_generator.request();

        program.invert( inv_result, func_to_rram.at( {children[i_src_neg].node, false} ) );
        func_to_rram.insert( {{children[i_src_neg].node, true}, inv_result} );
        src_neg = inv_result;
      } while ( false );
    }
    children_compl.reset( i_src_neg );

    /* find the destination */
    unsigned oa, ob;
    std::tie( oa, ob ) = three_without( i_src_neg );

    /* if there is a child with one fan-out */
    /* check whether they fulfill the requirements (non-constant and one fan-out) */
    const auto oa_c = children[oa].node != 0u && cmp.fanout_count( children[oa].node ) == 1u;
    const auto ob_c = children[ob].node != 0u && cmp.fanout_count( children[ob].node ) == 1u;

    if ( oa_c || ob_c )
    {
      /* first check for complemented cases (to avoid them for last operand) */
      std::unordered_map<mig_function, memristor_index>::const_iterator it;
      if ( oa_c && children[oa].complemented && ( it = func_to_rram.find( {children[oa].node, true} ) ) != func_to_rram.end() )
      {
        i_dst = oa;
        dst   = it->second;
      }
      else if ( ob_c && children[ob].complemented && ( it = func_to_rram.find( {children[ob].node, true} ) ) != func_to_rram.end() )
      {
        i_dst = ob;
        dst   = it->second;
      }
      else if ( oa_c && !children[oa].complemented )
      {
        i_dst = oa;
        dst   = func_to_rram.at( {children[oa].node, false} );
      }
      else if ( ob_c && !children[ob].complemented )
      {
        i_dst = ob;
        dst   = func_to_rram.at( {children[ob].node, false} );
      }
    }

    /* no destination found yet? */
    if ( i_dst == 3u )
    {
      /* create new work RRAM */
      dst = memristor_generator.request();

      /* is there a constant (if, then it's the first one) */
      if ( children[oa].node == 0u )
      {
        i_dst = oa;
        program.read_constant( dst, children[oa].complemented );
      }
      /* is there another inverter, then load it with that one? */
      else if ( children_compl.count() > 0u )
      {
        i_dst = children_compl.find_first();
        program.invert( dst, func_to_rram.at( {children[i_dst].node, false} ) );
      }
      /* otherwise, pick first one */
      else
      {
        i_dst = oa;
        program.assign( dst, func_to_rram.at( {children[i_dst].node, false} ) );
      }
    }

    /* positive operand */
    i_src_pos = 3u - i_src_neg - i_dst;
    const auto node = children[i_src_pos].node;

    if ( node == 0u )
    {
      src_pos = children[i_src_pos].complemented;
    }
    else if ( children[i_src_pos].complemented )
    {
      const auto it_reg = func_to_rram.find( {node, true} );
      if ( it_reg == func_to_rram.end() )
      {
        /* create new register for inversion */
        const auto inv_result = memristor_generator.request();

        program.invert( inv_result, func_to_rram.at( {node, false} ) );
        func_to_rram.insert( {{node, true}, inv_result} );
        src_pos = inv_result;
      }
      else
      {
        src_pos = it_reg->second;
      }
    }
    else
    {
      src_pos = func_to_rram.at( {node, false} );
    }

    program.compute( dst, src_pos, src_neg );
    func_to_rram.insert( {{candidate, false}, dst} );

    /* free free registers */
    for ( const auto& c : children )
    {
      if ( cmp.remove_fanout( c.node ) == 0u && c.node != 0u )
      {
        const auto reg = func_to_rram.at( {c.node, false} );
        if ( reg != dst )
        {
          memristor_generator.release( reg );
        }

        const auto it_reg = func_to_rram.find( {c.node, true} );
        if ( it_reg != func_to_rram.end() && it_reg->second != dst )
        {
          memristor_generator.release( it_reg->second );
        }
      }
    }

    /* update computed and find new candidates */
    computed.set( candidate );
    const auto it = parent_edges.find( candidate );
    if ( it != parent_edges.end() ) /* if it has parents */
    {
      for ( const auto& e : it->second )
      {
        const auto parent = boost::source( e, mig );

        if ( !computed[parent] && all_children_computed( parent, mig, computed ) )
        {
          candidates.push( parent );
        }
      }
    }

    L( "    - src_pos: " << i_src_pos << std::endl <<
       "    - src_neg: " << i_src_neg << std::endl <<
       "    - dst:     " << i_dst << std::endl );
  }

  set( statistics, "step_count", (int)program.step_count() );
  set( statistics, "rram_count", (int)program.rram_count() );

  std::vector<int> write_counts( program.write_counts().begin(), program.write_counts().end() );
  set( statistics, "write_counts", write_counts );

  return program;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
