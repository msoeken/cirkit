/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

#include "lut_based_synthesis.hpp"

#include <fstream>
#include <stack>
#include <vector>

#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>

#define timer timer_class
#include <boost/progress.hpp>
#undef timer

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class lut_order_heuristic
{
public:
  enum step_type { pi, po, compute, uncompute };

  /* describes a single computation step */
  struct step
  {
    lut_vertex_t node;    /* the node to synthesize */
    unsigned     target;  /* the target line for the result */
    step_type    type;    /* which step to perform */
  };

  using step_vec = std::vector<step>;

public:
  explicit lut_order_heuristic( const lut_graph_t& lut )
    : _lut( lut ),
      _node_to_line( num_vertices( lut ) )
  {
  }

  virtual unsigned compute_steps() = 0;
  inline const step_vec& steps() const { return _steps; }

  inline unsigned& operator[]( lut_vertex_t node ) { return _node_to_line[node]; }

  std::vector<unsigned> compute_line_map( lut_vertex_t node ) const
  {
    std::vector<unsigned> line_map;

    for ( auto child : boost::make_iterator_range( boost::adjacent_vertices( node, lut() ) ) )
    {
      line_map.push_back( _node_to_line[child] );
    }
    line_map.push_back( _node_to_line[node] );

    return line_map;
  }

protected:
  void add_default_input_steps()
  {
    const auto type = boost::get( boost::vertex_gate_type, lut() );
    for ( auto n : boost::make_iterator_range( vertices( lut() ) ) )
    {
      if ( type[n] != gate_type_t::pi ) continue;

      const auto line = _next_free++;
      _node_to_line[n] = line;
      add_step( n, line, step_type::pi );
    }
  }

  void add_default_output_steps()
  {
    const auto type = boost::get( boost::vertex_gate_type, lut() );
    for ( auto n : boost::make_iterator_range( vertices( lut() ) ) )
    {
      if ( type[n] != gate_type_t::po ) continue;

      const auto node = *( boost::adjacent_vertices( n, lut() ).first );
      add_step( n, _node_to_line[node], step_type::po );
    }
  }

  void add_step( lut_vertex_t node, unsigned target, step_type type )
  {
    _steps.push_back( {node, target, type} );
  }

  unsigned request_constant()
  {
    if ( !_constants.empty() )
    {
      const auto line = _constants.top();
      _constants.pop();
      return line;
    }

    return _next_free++;
  }

  void free_constant( unsigned line )
  {
    _constants.push( line );
  }

  inline const lut_graph_t& lut() const { return _lut; }

  inline unsigned next_free() const { return _next_free; }

private:
  const lut_graph_t& _lut;
  unsigned _next_free = 0u;
  std::vector<unsigned> _node_to_line;
  step_vec _steps;
  std::stack<unsigned> _constants;
};

class defer_lut_order_heuristic : public lut_order_heuristic
{
public:
  defer_lut_order_heuristic( const lut_graph_t& lut )
    : lut_order_heuristic( lut ),
      indegrees( precompute_in_degrees( lut ) )
  {
  }

public:
  virtual unsigned compute_steps()
  {
    add_default_input_steps();
    adjust_indegrees();

    foreach_topological( lut(), [this]( const lut_vertex_t& n ) {
        const auto type = get( boost::vertex_gate_type, lut() );

        if ( type[n] != gate_type_t::internal ) return true;

        const auto target = request_constant();
        (*this)[n] = target;
        add_step( n, target, lut_order_heuristic::compute );

        /* start uncomputing? */
        if ( indegrees[n] == 0 )
        {
          decrease_children_indegrees( n );
          uncompute_children( n );
        }

        return true;
      } );

    add_default_output_steps();

    return next_free();
  }

private:
  void adjust_indegrees()
  {
    const auto type = get( boost::vertex_gate_type, lut() );
    for ( auto n : boost::make_iterator_range( vertices( lut() ) ) )
    {
      if ( type[n] != gate_type_t::po ) { continue; }

      assert( boost::out_degree( n, lut() ) == 1 );

      indegrees[*( boost::adjacent_vertices( n, lut() ).first )]--;
    }
  }

  void decrease_children_indegrees( lut_vertex_t node )
  {
    const auto type = get( boost::vertex_gate_type, lut() );
    for ( auto n : boost::make_iterator_range( adjacent_vertices( node, lut() ) ) )
    {
      if ( type[n] != gate_type_t::internal ) { continue; }

      indegrees[n]--;
    }
  }

  void uncompute_children( lut_vertex_t node )
  {
    const auto type = get( boost::vertex_gate_type, lut() );
    for ( auto n : boost::make_iterator_range( adjacent_vertices( node, lut() ) ) )
    {
      if ( type[n] != gate_type_t::internal ) { continue; }

      if ( indegrees[n] == 0 )
      {
        uncompute_node( n );
      }
    }
  }

  void uncompute_node( lut_vertex_t node )
  {
    assert( indegrees[node] == 0 );

    const auto target = (*this)[node];
    add_step( node, target, lut_order_heuristic::uncompute );
    free_constant( target );

    decrease_children_indegrees( node );
    uncompute_children( node );
  }

private:
  std::vector<unsigned> indegrees;
};

class lut_based_synthesis_manager
{
public:
  lut_based_synthesis_manager( circuit& circ, const lut_graph_t& lut, const properties::ptr& settings )
    : circ( circ ),
      lut( lut ),
      order_heuristic( lut ),
      verbose( get( settings, "verbose", verbose ) ),
      progress( get( settings, "progress", progress ) )
  {
  }

  bool run()
  {
    const auto lines = order_heuristic.compute_steps();
    circ.set_lines( lines );

    null_stream ns;
    std::ostream null_out( &ns );
    boost::progress_display show_progress( order_heuristic.steps().size(), progress ? std::cout : null_out );

    std::vector<std::string> inputs( lines, "0" );
    std::vector<std::string> outputs( lines, "0" );
    std::vector<constant> constants( lines, false );
    std::vector<bool> garbage( lines, true );

    const auto name = boost::get( boost::vertex_name, lut );

    for ( const auto& step : order_heuristic.steps() )
    {
      ++show_progress;

      switch ( step.type )
      {
      case lut_order_heuristic::pi:
        inputs[step.target] = outputs[step.target] = name[step.node];
        constants[step.target] = boost::none;
        break;

      case lut_order_heuristic::po:
        assert( outputs[step.target] == "0" );
        outputs[step.target] = name[step.node];
        garbage[step.target] = false;
        break;

      case lut_order_heuristic::compute:
        synthesize_node( step.node, false );
        break;

      case lut_order_heuristic::uncompute:
        synthesize_node( step.node, true );
        break;
      }
    }

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );

    return true;
  }

private:
  void write_blif( lut_vertex_t node )
  {
    const auto fanin = boost::out_degree( node, lut );
    const auto spec = get( boost::vertex_lut, lut );
    const auto inputs = create_name_list( "x%d", fanin );

    std::ofstream os( blifname.name(), std::ofstream::out );
    os << ".model top" << std::endl
       << ".inputs " << boost::join( inputs, " " ) << std::endl
       << ".outputs f" << std::endl
       << ".names " << boost::join( inputs, " " ) << " f" << std::endl
       << spec[node]
       << ".end" << std::endl;
    os.close();
  }

  circuit compute_circuit( lut_vertex_t node, bool lookup )
  {
    if ( lookup )
    {
      return computed_circuits[node];
    }

    write_blif( node );
    const auto es_settings = std::make_shared<properties>();
    es_settings->set( "esopname", esopname.name() );
    exorcism_minimization_blif( blifname.name(), es_settings );

    circuit local_circ;
    esop_synthesis( local_circ, esopname.name() );

    computed_circuits.insert( {node, local_circ} );

    return local_circ;
  }

  void synthesize_node( lut_vertex_t node, bool lookup )
  {
    /* map circuit */
    const auto line_map = order_heuristic.compute_line_map( node );

    for ( const auto& g : compute_circuit( node, lookup ) )
    {
      assert( is_toffoli( g ) );

      gate::control_container controls;
      for ( const auto& c : g.controls() )
      {
        controls.push_back( make_var( line_map[c.line()], c.polarity() ) );
      }
      append_toffoli( circ, controls, line_map[g.targets().front()] );
    }
  }

private:
  circuit& circ;
  const lut_graph_t& lut;

  std::unordered_map<unsigned, circuit> computed_circuits;

  defer_lut_order_heuristic order_heuristic;

  temporary_filename blifname{ "/tmp/lbs-%d.blif" };
  temporary_filename esopname{ "/tmp/lbs-%d.esop" };

  bool verbose = false;
  bool progress = false;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool lut_based_synthesis( circuit& circ, const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  lut_based_synthesis_manager mgr( circ, lut, settings );
  return mgr.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
