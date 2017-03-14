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

#include "lut_based_synthesis.hpp"

#include <fstream>
#include <stack>
#include <vector>

#include <boost/format.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/gia/gia.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/functions/linear_classification.hpp>
#include <classical/io/read_blif.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <classical/optimization/exorcismq.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/target_tags.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/add_line_to_circuit.hpp>
#include <reversible/functions/circuit_from_string.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/synthesis/esop_synthesis.hpp>
#include <reversible/synthesis/optimal_quantum_circuits.hpp>

namespace cirkit
{

/******************************************************************************
 * Order heuristics (new)                                                     *
 ******************************************************************************/

class lut_order_heuristic_new
{
public:
  enum step_type { pi, po, compute, uncompute };

  /* describes a single computation step */
  struct step
  {
    int                  node;          /* the node to synthesize */
    unsigned             target;        /* the target line for the result */
    step_type            type;          /* which step to perform */
    std::stack<unsigned> clean_ancilla; /* number of clean ancillae */
  };

  using step_vec = std::vector<step>;

public:
  explicit lut_order_heuristic_new( const gia_graph& gia )
    : _gia( gia )
  {
  }

  virtual unsigned compute_steps() = 0;
  inline const step_vec& steps() const { return _steps; }

  inline unsigned& node_to_line( int index ) { return _node_to_line[index]; }
  inline unsigned node_to_line( int index ) const { return _node_to_line.find( index )->second; }
  inline unsigned& operator[]( int index ) { return _node_to_line[index]; }

  std::vector<unsigned> compute_line_map( int index ) const
  {
    std::vector<unsigned> line_map;

    _gia.foreach_lut_fanin( index, [this, &line_map]( int fanin ) {
        const auto it = _node_to_line.find( fanin );
        if ( it == _node_to_line.end() )
        {
          std::cout << "no line for node " << fanin << std::endl;
          assert( false );
        }
        line_map.push_back( node_to_line( fanin ) );
      } );

    const auto it = _node_to_line.find( index );
    if ( it == _node_to_line.end() )
    {
      std::cout << "no line for node " << index << std::endl;
      assert( false );
    }
    line_map.push_back( node_to_line( index ) );
    return line_map;
  }

  unsigned num_clean_ancilla()
  {
    return _constants.size();
  }

protected:
  void add_default_input_steps()
  {
    _gia.foreach_input( [this]( int index, int e ) {
        const auto line = _next_free++;
        node_to_line( index ) = line;
        add_step( index, line, step_type::pi );
      } );
  }

  void add_default_output_steps()
  {
    _gia.foreach_output( [this]( int index, int e ) {
        const auto driver = abc::Gia_ObjFaninId0p( _gia, abc::Gia_ManCo( _gia, e ) );
        add_step( index, node_to_line( driver ), step_type::po );
      } );
  }

  void add_step( int index, unsigned target, step_type type )
  {
    if ( !_dry_run )
    {
      _steps.push_back( {index, target, type, _constants} );
    }
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

  void add_constants( unsigned max )
  {
    while ( _next_free < max )
    {
      _constants.push( _next_free++ );
    }
  }

  void free_constant( unsigned line )
  {
    _constants.push( line );
  }

  inline const gia_graph& gia() const { return _gia; }

  inline unsigned next_free() const { return _next_free; }

  void set_mem_point()
  {
    _constants_mem = _constants;
    _next_free_mem = _next_free;
  }

  void return_to_mem_point()
  {
    std::swap( _constants, _constants_mem );
    std::swap( _next_free, _next_free_mem );
  }

  void set_dry_run( bool dry_run )
  {
    _dry_run = dry_run;
  }

private:
  const gia_graph& _gia;
  step_vec _steps;
  std::unordered_map<int, unsigned> _node_to_line;
  std::stack<unsigned> _constants;

  /* for memory */
  std::stack<unsigned> _constants_mem;
  unsigned _next_free_mem;

  bool _dry_run = false;

protected:
  unsigned _next_free = 0u;
};

class defer_lut_order_heuristic_new : public lut_order_heuristic_new
{
public:
  defer_lut_order_heuristic_new( const gia_graph& gia )
    : lut_order_heuristic_new( gia )
  {
  }

public:
  virtual unsigned compute_steps()
  {
    set_mem_point();
    set_dry_run( true );
    const auto next_free = compute_steps_int();
    set_dry_run( false );
    return_to_mem_point();

    return compute_steps_int( next_free );
  }

private:
  unsigned compute_steps_int( unsigned add_frees = 0u )
  {
    gia().init_lut_refs();

    add_default_input_steps();

    if ( add_frees )
    {
      add_constants( add_frees );
    }

    adjust_indegrees();

    gia().foreach_lut( [this]( int index ) {
        const auto target = request_constant();
        (*this)[index] = target;

        add_step( index, target, lut_order_heuristic_new::compute );

        /* start uncomputing */
        if ( gia().lut_ref_num( index ) == 0 )
        {
          decrease_children_indegrees( index );
          uncompute_children( index );
        }
      } );

    add_default_output_steps();

    return next_free();
  }

  void adjust_indegrees()
  {
    gia().foreach_output( [this]( int index, int e ) {
        const auto driver = abc::Gia_ObjFaninId0p( gia(), abc::Gia_ManCo( gia(), e ) );
        gia().lut_ref_dec( driver );
      } );
  }

  void decrease_children_indegrees( int index )
  {
    gia().foreach_lut_fanin( index, [this]( int fanin ) {
        if ( gia().is_lut( fanin ) )
        {
          gia().lut_ref_dec( fanin );
        }
      } );
  }

  void uncompute_children( int index )
  {
    gia().foreach_lut_fanin( index, [this]( int fanin ) {
        if ( gia().is_lut( fanin ) && gia().lut_ref_num( fanin ) == 0 )
        {
          uncompute_node( fanin );
        }
      } );
  }

  void uncompute_node( int index )
  {
    assert( gia().lut_ref_num( index ) == 0 );

    const auto target = (*this)[index];
    add_step( index, target, lut_order_heuristic_new::uncompute );
    free_constant( target );

    decrease_children_indegrees( index );
    uncompute_children( index );
  }
};

/******************************************************************************
 * Partial synthesizers (new)                                                 *
 ******************************************************************************/

class lut_partial_synthesizer_new
{
public:
  explicit lut_partial_synthesizer_new( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : _gia( gia ),
      settings( settings ),
      statistics( statistics )
  {
  }

  const circuit& lookup_or_compute( int index, bool lookup )
  {
    if ( lookup )
    {
      return circuits[index];
    }

    return circuits[index] = compute( index );
  }

protected:
  virtual circuit compute( int index ) const = 0;

  inline const gia_graph& gia() const
  {
    return _gia;
  }

private:
  const gia_graph& _gia;
  std::unordered_map<unsigned, circuit> circuits;

protected:
  const properties::ptr& settings;
  const properties::ptr& statistics;
};

class exorcism_lut_partial_synthesizer_new : public lut_partial_synthesizer_new
{
public:
  explicit exorcism_lut_partial_synthesizer_new( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer_new( gia, settings, statistics ),
      dry( get( settings, "dry", dry ) ),
      verbose( get( settings, "verbose", verbose ) )
  {
  }

  virtual ~exorcism_lut_partial_synthesizer_new()
  {
    set( statistics, "cover_time", cover_time );
    set( statistics, "exorcism_time", exorcism_time );
  }

protected:
  circuit compute( int index ) const
  {
    if ( dry )
    {
      return circuit( gia().lut_size( index ) + 1u );
    }
    else
    {
      // TODO runtime
      circuit local_circ;
      const auto lut = gia().extract_lut( index );

      auto esop = [this, &lut]() {
        increment_timer t( &cover_time );
        return lut.compute_esop_cover();
      }();

      esop = [this, &esop, &lut]() {
        increment_timer t( &exorcism_time );
        return exorcism_minimization( esop, lut.num_inputs(), lut.num_outputs() );
      }();

      esop_synthesis( local_circ, esop, lut.num_inputs(), lut.num_outputs() );
      return local_circ;
    }
  }

private:
  /* settings */
  bool dry = false;
  bool verbose = false;

public: /* for progress line */
  mutable double cover_time    = 0.0;
  mutable double exorcism_time = 0.0;
};

/******************************************************************************
 * Order heuristics                                                           *
 ******************************************************************************/

class lut_order_heuristic
{
public:
  enum step_type { pi, po, compute, uncompute };

  /* describes a single computation step */
  struct step
  {
    lut_vertex_t         node;          /* the node to synthesize */
    unsigned             target;        /* the target line for the result */
    step_type            type;          /* which step to perform */
    std::stack<unsigned> clean_ancilla; /* number of clean ancillae */
  };

  using step_vec = std::vector<step>;

public:
  explicit lut_order_heuristic( const lut_graph_t& lut )
    : _lut( lut ),
      _node_to_line( num_vertices( lut ) ),
      indegrees( precompute_in_degrees( lut ) )
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

  unsigned num_clean_ancilla()
  {
    return _constants.size();
  }

protected:
  void add_default_input_steps()
  {
    const auto type = boost::get( boost::vertex_lut_type, lut() );
    for ( auto n : boost::make_iterator_range( vertices( lut() ) ) )
    {
      if ( type[n] != lut_type_t::pi ) continue;

      const auto line = _next_free++;
      _node_to_line[n] = line;
      add_step( n, line, step_type::pi );
    }
  }

  void add_default_output_steps()
  {
    const auto type = boost::get( boost::vertex_lut_type, lut() );
    for ( auto n : boost::make_iterator_range( vertices( lut() ) ) )
    {
      if ( type[n] != lut_type_t::po ) continue;

      const auto node = *( boost::adjacent_vertices( n, lut() ).first );
      add_step( n, _node_to_line[node], step_type::po );
    }
  }

  void add_step( lut_vertex_t node, unsigned target, step_type type )
  {
    if ( !_dry_run )
    {
      _steps.push_back( {node, target, type, _constants} );
    }
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

  void add_constants( unsigned max )
  {
    while ( _next_free < max )
    {
      _constants.push( _next_free++ );
    }
  }

  void free_constant( unsigned line )
  {
    _constants.push( line );
  }

  inline const lut_graph_t& lut() const { return _lut; }

  inline unsigned next_free() const { return _next_free; }

  void set_mem_point()
  {
    _constants_mem = _constants;
    _next_free_mem = _next_free;
    _node_to_line_mem = _node_to_line;
    indegrees_mem = indegrees;
  }

  void return_to_mem_point()
  {
    std::swap( _constants, _constants_mem );
    std::swap( _next_free, _next_free_mem );
    std::swap( _node_to_line, _node_to_line_mem );
    std::swap( indegrees, indegrees_mem );
  }

  void set_dry_run( bool dry_run )
  {
    _dry_run = dry_run;
  }

private:
  const lut_graph_t& _lut;
  std::vector<unsigned> _node_to_line;
  step_vec _steps;
  std::stack<unsigned> _constants;

  /* for memory */
  std::stack<unsigned> _constants_mem;
  unsigned _next_free_mem;
  std::vector<unsigned> _node_to_line_mem;
  std::vector<unsigned> indegrees_mem;

  bool _dry_run = false;

protected:
  unsigned _next_free = 0u;
  std::vector<unsigned> indegrees;
};

class defer_lut_order_heuristic : public lut_order_heuristic
{
public:
  defer_lut_order_heuristic( const lut_graph_t& lut )
    : lut_order_heuristic( lut )
  {
  }

public:
  virtual unsigned compute_steps()
  {
    set_mem_point();
    set_dry_run( true );
    const auto next_free = compute_steps_int();
    set_dry_run( false );
    return_to_mem_point();

    return compute_steps_int( next_free );
  }

private:
  unsigned compute_steps_int( unsigned add_frees = 0u )
  {
    add_default_input_steps();

    if ( add_frees )
    {
      add_constants( add_frees );
    }

    adjust_indegrees();

    foreach_topological( lut(), [this]( const lut_vertex_t& n ) {
        const auto type = get( boost::vertex_lut_type, lut() );

        if ( type[n] != lut_type_t::internal ) return true;

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

  void adjust_indegrees()
  {
    const auto type = get( boost::vertex_lut_type, lut() );
    for ( auto n : boost::make_iterator_range( vertices( lut() ) ) )
    {
      if ( type[n] != lut_type_t::po ) { continue; }

      assert( boost::out_degree( n, lut() ) == 1 );

      indegrees[*( boost::adjacent_vertices( n, lut() ).first )]--;
    }
  }

  void decrease_children_indegrees( lut_vertex_t node )
  {
    const auto type = get( boost::vertex_lut_type, lut() );
    for ( auto n : boost::make_iterator_range( adjacent_vertices( node, lut() ) ) )
    {
      if ( type[n] != lut_type_t::internal ) { continue; }

      indegrees[n]--;
    }
  }

  void uncompute_children( lut_vertex_t node )
  {
    const auto type = get( boost::vertex_lut_type, lut() );
    for ( auto n : boost::make_iterator_range( adjacent_vertices( node, lut() ) ) )
    {
      if ( type[n] != lut_type_t::internal ) { continue; }

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
};

/******************************************************************************
 * Partial synthesizers                                                       *
 ******************************************************************************/

class lut_partial_synthesizer
{
public:
  explicit lut_partial_synthesizer( const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics )
    : _lut( lut ),
      settings( settings ),
      statistics( statistics )
  {
  }

  const circuit& lookup_or_compute( lut_vertex_t node, bool lookup )
  {
    if ( lookup )
    {
      return circuits[node];
    }

    return circuits[node] = compute( node );
  }

protected:
  virtual circuit compute( lut_vertex_t node ) const = 0;

  inline const lut_graph_t& lut() const
  {
    return _lut;
  }

  std::string write_blif( lut_vertex_t node ) const
  {
    const auto fanin = boost::out_degree( node, lut() );
    const auto spec = get( boost::vertex_lut, lut() );
    const auto inputs = create_name_list( "x%d", fanin );

    const auto filename = blifname.name();// + std::to_string( counter++ );

    std::ofstream os( filename, std::ofstream::out );
    os << ".model top" << std::endl
       << ".inputs " << boost::join( inputs, " " ) << std::endl
       << ".outputs f" << std::endl
       << ".names " << boost::join( inputs, " " ) << " f" << std::endl
       << spec[node]
       << ".end" << std::endl;
    os.close();

    return filename;
  }

private:
  const lut_graph_t& _lut;

  std::unordered_map<unsigned, circuit> circuits;

  temporary_filename blifname{ "/tmp/lbs-%d.blif" };

  mutable unsigned counter = 0u;

protected:
  const properties::ptr& settings;
  const properties::ptr& statistics;
};

class exorcism_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit exorcism_lut_partial_synthesizer( const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer( lut, settings, statistics ),
      dry( get( settings, "dry", dry ) ),
      verbose( get( settings, "verbose", verbose ) )
  {
  }

  virtual ~exorcism_lut_partial_synthesizer()
  {
    set( statistics, "exorcism_time", exorcism_time );
  }

protected:
  circuit compute( lut_vertex_t node ) const
  {
    if ( dry )
    {
      return circuit( boost::out_degree( node, lut() ) + 1u );
    }
    else
    {
      increment_timer t( &exorcism_time );

      const auto blifname = write_blif( node );
      const auto es_settings = std::make_shared<properties>();
      es_settings->set( "esopname", esopname.name() );
      es_settings->set( "skip_parsing", true );
      es_settings->set( "verbose",      verbose );
      es_settings->set( "very_verbose", false );
      const auto es_statistics = std::make_shared<properties>();
      exorcism_minimization_blif( blifname, es_settings, es_statistics );

      circuit local_circ;
      esop_synthesis( local_circ, esopname.name() );

      return local_circ;
    }
  }

private:
  temporary_filename esopname{ "/tmp/lbs-%d.esop" };

  /* settings */
  bool dry = false;
  bool verbose = false;

public: /* for progress line */
  mutable double exorcism_time = 0.0;
};

class exorcismq_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit exorcismq_lut_partial_synthesizer( const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics ) : lut_partial_synthesizer( lut, settings, statistics ) {}

protected:
  circuit compute( lut_vertex_t node ) const
  {
    const auto blifname = write_blif( node );
    const auto es_settings = std::make_shared<properties>();
    es_settings->set( "esopname", esopname.name() );
    exorcismq_minimization_from_blif( blifname, es_settings );

    circuit local_circ;
    esop_synthesis( local_circ, esopname.name() );

    return local_circ;
  }

private:
  temporary_filename esopname{ "/tmp/lbs-%d.esop" };
};

class hybrid_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit hybrid_lut_partial_synthesizer( const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics ) : lut_partial_synthesizer( lut, settings, statistics ) {}

protected:
  circuit compute( lut_vertex_t node ) const
  {
    const auto blifname = write_blif( node );
    const auto es_settings = std::make_shared<properties>();
    es_settings->set( "esopname", esopname1.name() );
    exorcism_minimization_blif( blifname, es_settings );

    es_settings->set( "esopname", esopname2.name() );
    exorcismq_minimization_from_esop( esopname1.name(), es_settings );

    circuit local_circ;
    esop_synthesis( local_circ, esopname2.name() );

    return local_circ;
  }

private:
  temporary_filename esopname1{ "/tmp/lbs-%d.esop" };
  temporary_filename esopname2{ "/tmp/lbs-%d.esop" };
};

class lutdecomp_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit lutdecomp_lut_partial_synthesizer( const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer( lut, settings, statistics ),
      class_circuits( 3u ),
      class_counter( 3u ),
      class_hash( 3u )
  {
    class_counter[0u].resize( 3u );
    class_counter[1u].resize( 6u );
    class_counter[2u].resize( 18u );

    for ( auto i = 0u; i < 3u; ++i )
    {
      for ( auto j = 0u; j < class_counter[i].size(); ++j )
      {
        class_circuits[i].push_back( circuit_from_string( optimal_quantum_circuits::affine_classification[i][j] ) );
      }
    }
  }

  virtual ~lutdecomp_lut_partial_synthesizer()
  {
    set( statistics, "class_counter", class_counter );
    set( statistics, "class_runtime", class_runtime );
    set( statistics, "mapping_runtime", mapping_runtime );
  }

protected:
  circuit compute( lut_vertex_t node ) const
  {
    const auto num_inputs = boost::out_degree( node, lut() );
    auto num_ancilla = 0u;

    if ( num_inputs < 5 )
    {
      const auto spec = get( boost::vertex_lut, lut() );

      const auto& sop_spec = spec[node];
      const auto tt_spec = tt_from_sop_spec( sop_spec );
      const auto affine_class = classify( tt_spec.to_ulong(), num_inputs );

      return class_circuits[num_inputs - 2u][class_index[num_inputs - 2u].at( affine_class )];
    }
    else
    {
      //std::cout << boost::format( "[i] node %d has %d inputs" ) % node % num_inputs << std::endl;

      std::string blifname;
      lut_graph_t sub_lut;

      {
        increment_timer t( &mapping_runtime );
        blifname = write_blif( node );
        abc_run_command_no_output( boost::str( boost::format( "read_blif %s; strash; %s; %s; if -K 4 -a; write_blif %s" )
                                               % blifname % "dc2" % "dc2" /*abc_command_constants::resyn2_command % abc_command_constants::resyn2_command*/ % blifname ) );
        sub_lut = read_blif( blifname );
      }

      std::vector<unsigned> lut_to_line( boost::num_vertices( sub_lut ), 0u );
      const auto type = boost::get( boost::vertex_lut_type, sub_lut );
      const auto sub_spec = boost::get( boost::vertex_lut, sub_lut );

      /* first pass: count ancillas and determine root gate */
      lut_vertex_t root{};
      for ( auto v : boost::make_iterator_range( boost::vertices( sub_lut ) ) )
      {
        if ( type[v] == lut_type_t::internal ) { ++num_ancilla; }
        if ( type[v] == lut_type_t::po )
        {
          root = boost::target( *boost::out_edges( v, sub_lut ).first, sub_lut );
        }
      }
      --num_ancilla; /* do not account for the root gate */

      /* second pass: map LUTs to lines, and compute classes */
      auto pi_index = 0u;
      auto anc_index = num_inputs + 1u;
      auto ins_index = 0u;
      std::vector<unsigned> synth_order( 2 * num_ancilla + 1, 99 );
      std::vector<uint64_t> aff_class( boost::num_vertices( sub_lut ) );
      std::vector<unsigned> top( boost::num_vertices( sub_lut ) );
      boost::topological_sort( sub_lut, top.begin() );
      for ( auto v : top )
      {
        if ( type[v] == lut_type_t::pi )
        {
          lut_to_line[v] = pi_index++;
        }
        if ( type[v] == lut_type_t::internal )
        {
          if ( v == root )
          {
            lut_to_line[v] = num_inputs;
            synth_order[ins_index] = v;
          }
          else
          {
            lut_to_line[v] = anc_index++;
            synth_order[ins_index] = synth_order[synth_order.size() - 1 - ins_index] = v;
            ++ins_index;
          }

          if ( boost::out_degree( v, sub_lut ) >= 2 )
          {
            aff_class[v] = classify( tt( convert_hex2bin( sub_spec[v] ) ).to_ulong(), boost::out_degree( v, sub_lut ) );
          }
        }
      }

      circuit local_circ( num_inputs + 1 + num_ancilla );
      for ( auto v : synth_order )
      {
        const auto num_inputs = boost::out_degree( v, sub_lut );
        std::vector<unsigned> line_map;
        line_map.reserve( num_inputs + 1u );

        for ( auto w : boost::make_iterator_range( boost::adjacent_vertices( v, sub_lut ) ) )
        {
          line_map.push_back( lut_to_line[w] );
        }
        line_map.push_back( lut_to_line[v] );

        switch ( num_inputs )
        {
        case 0u:
          assert( false );
          break;
        case 1u:
          assert( sub_spec[v] == "1" );
          append_cnot( local_circ, make_var( line_map[0], false ), line_map[1] );
          break;
        default:
          const auto& aff_circ = class_circuits[num_inputs - 2u][class_index[num_inputs - 2u].at( aff_class[v] )];
          if ( aff_circ.lines() > num_inputs + 1u )
          {
            assert( aff_circ.lines() == num_inputs + 2u );

            // find any free line
            auto free_line = -1;
            for ( auto i = 0u; i < num_inputs + 1 + num_ancilla; ++i )
            {
              if ( std::find( line_map.begin(), line_map.end(), i ) == line_map.end() )
              {
                free_line = i;
                break;
              }
            }

            assert( free_line != -1 );

            line_map.push_back( free_line );
          }
          append_circuit( local_circ, aff_circ, gate::control_container(), line_map );
          break;
        }
      }

      return local_circ;
    }
  }

private:
  inline uint64_t classify( uint64_t func, unsigned num_vars ) const
  {
    increment_timer t( &class_runtime );

    uint64_t afunc{};
    const auto it = class_hash[num_vars - 2u].find( func );
    if ( it == class_hash[num_vars - 2u].end() )
    {
      afunc = exact_affine_classification_output( func, num_vars );
      class_hash[num_vars - 2u].insert( std::make_pair( func, afunc ) );
    }
    else
    {
      afunc = it->second;
    }
    ++class_counter[num_vars - 2u][class_index[num_vars - 2u].at( afunc )];
    return afunc;
  }

private:
  std::vector<std::unordered_map<uint64_t, unsigned>> class_index = {
    {{0x0, 0}, {0x1, 1}, {0x3, 2}},
    {{0x00, 0}, {0x01, 1}, {0x03, 2}, {0x07, 3}, {0x0f, 4}, {0x17, 5}},
    {{0x0000, 0},{0x0001, 1},{0x0003, 2},{0x0007, 3},{0x000f, 4},{0x0017, 5},{0x001f, 6},{0x003f, 7},{0x007f, 8},{0x00ff, 9},{0x0117, 10},{0x011f, 11},{0x013f, 12},{0x017f, 13},{0x033f, 14},{0x0356, 15},{0x0357, 16},{0x035f, 17}}
  };

  std::vector<std::vector<circuit>> class_circuits;
  mutable std::vector<std::unordered_map<uint64_t, uint64_t>> class_hash;

private: /* statistics */
  mutable std::vector<std::vector<unsigned>> class_counter;

public: /* for progress printing */
  mutable double class_runtime = 0.0;
  mutable double mapping_runtime = 0.0;
};

/******************************************************************************
 * Manager (new)                                                              *
 ******************************************************************************/

class lut_based_synthesis_manager_new
{
public:
  lut_based_synthesis_manager_new( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : circ( circ ),
      gia( gia ),
      statistics( statistics ),
      order_heuristic( std::make_shared<defer_lut_order_heuristic_new>( gia ) ),
      synthesizer( gia, settings, statistics ),
      //decomp_synthesizer( lut, settings, statistics ),
      verbose( get( settings, "verbose", verbose ) ),
      progress( get( settings, "progress", progress ) ),
      lutdecomp( get( settings, "lutdecomp", lutdecomp ) )
  {
  }

  bool run()
  {
    clear_circuit( circ );

    const auto lines = order_heuristic->compute_steps();
    circ.set_lines( lines );

    std::vector<std::string> inputs( lines, "0" );
    std::vector<std::string> outputs( lines, "0" );
    std::vector<constant> constants( lines, false );
    std::vector<bool> garbage( lines, true );

    auto step_index = 0u;
    //progress_line p( "[i] step %5d/%5d   dd = %5d   ld = %5d   esop = %6.2f   map = %6.2f   clsfy = %6.2f   total = %6.2f\r", progress );
    progress_line p( "[i] step %5d/%5d   dd = %5d   ld = %5d   cvr = %6.2f   esop = %6.2f   total = %6.2f\r", progress );
    for ( const auto& step : order_heuristic->steps() )
    {
      p( ++step_index, order_heuristic->steps().size(), num_decomp_default, num_decomp_lut, synthesizer.cover_time, synthesizer.exorcism_time, /*decomp_synthesizer.mapping_runtime, decomp_synthesizer.class_runtime, */synthesis_time );
      increment_timer t( &synthesis_time );

      switch ( step.type )
      {
      case lut_order_heuristic::pi:
        inputs[step.target] = outputs[step.target] = "x"; // TODO name[step.node];
        constants[step.target] = boost::none;
        break;

      case lut_order_heuristic::po:
        if ( outputs[step.target] != "0" )
        {
          circ.set_lines( circ.lines() + 1 );
          inputs.push_back( "0" );
          constants.push_back( false );
          outputs.push_back( "y" /*name[step.node]*/ ); // TODO
          garbage.push_back( false );

          append_cnot( circ, step.target, circ.lines() - 1 );
        }
        else
        {
          outputs[step.target] = "y"; // name[step.node];
          garbage[step.target] = false;
        }
        break;

      case lut_order_heuristic::compute:
        synthesize_node( step.node, false, step.clean_ancilla );
        break;

      case lut_order_heuristic::uncompute:
        synthesize_node( step.node, true, step.clean_ancilla );
        break;
      }
    }

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );

    set( statistics, "num_decomp_default", num_decomp_default );
    set( statistics, "num_decomp_lut", num_decomp_lut );

    return true;
  }

private:
  void synthesize_node( int index, bool lookup, const std::stack<unsigned>& clean_ancilla )
  {
    /* map circuit */
    auto line_map = order_heuristic->compute_line_map( index );

    circuit local_circ;

    // if ( lutdecomp )
    // {
    //   local_circ = decomp_synthesizer.lookup_or_compute( node, lookup );
    //   const auto num_ancilla = local_circ.lines() - line_map.size();
    //   if ( num_ancilla > clean_ancilla.size() )
    //   {
    //     local_circ = synthesizer.lookup_or_compute( node, lookup );
    //     ++num_decomp_default;
    //   }
    //   else
    //   {
    //     auto cstack = clean_ancilla;
    //     for ( auto i = 0u; i < num_ancilla; ++i )
    //     {
    //       line_map.push_back( cstack.top() );
    //       cstack.pop();
    //     }
    //     ++num_decomp_lut;
    //   }
    // }
    // else
    {
      local_circ = synthesizer.lookup_or_compute( index, lookup );
      ++num_decomp_default;
    }

    append_circuit( circ, local_circ, gate::control_container(), line_map );
  }

private:
  circuit& circ;
  const gia_graph& gia;

  const properties::ptr& statistics;

  std::unordered_map<unsigned, circuit> computed_circuits;

  std::shared_ptr<lut_order_heuristic_new> order_heuristic;
  exorcism_lut_partial_synthesizer_new synthesizer;
  //lutdecomp_lut_partial_synthesizer decomp_synthesizer;

  bool verbose = false;
  bool progress = false;
  bool lutdecomp = false;

private: /* statistics */
  double   synthesis_time = 0.0;
  unsigned num_decomp_default = 0u;
  unsigned num_decomp_lut = 0u;
};

/******************************************************************************
 * Manager                                                                    *
 ******************************************************************************/

class lut_based_synthesis_manager
{
public:
  lut_based_synthesis_manager( circuit& circ, const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics )
    : circ( circ ),
      lut( lut ),
      statistics( statistics ),
      order_heuristic( std::make_shared<defer_lut_order_heuristic>( lut ) ),
      synthesizer( lut, settings, statistics ),
      decomp_synthesizer( lut, settings, statistics ),
      verbose( get( settings, "verbose", verbose ) ),
      progress( get( settings, "progress", progress ) ),
      lutdecomp( get( settings, "lutdecomp", lutdecomp ) )
  {
  }

  bool run()
  {
    clear_circuit( circ );

    const auto lines = order_heuristic->compute_steps();
    circ.set_lines( lines );

    std::vector<std::string> inputs( lines, "0" );
    std::vector<std::string> outputs( lines, "0" );
    std::vector<constant> constants( lines, false );
    std::vector<bool> garbage( lines, true );

    const auto name = boost::get( boost::vertex_name, lut );

    auto step_index = 0u;
    progress_line p( "[i] step %5d/%5d   dd = %5d   ld = %5d   esop = %6.2f   map = %6.2f   clsfy = %6.2f   total = %6.2f\r", progress );
    for ( const auto& step : order_heuristic->steps() )
    {
      p( ++step_index, order_heuristic->steps().size(), num_decomp_default, num_decomp_lut, synthesizer.exorcism_time, decomp_synthesizer.mapping_runtime, decomp_synthesizer.class_runtime, synthesis_time );
      increment_timer t( &synthesis_time );

      switch ( step.type )
      {
      case lut_order_heuristic::pi:
        inputs[step.target] = outputs[step.target] = name[step.node];
        constants[step.target] = boost::none;
        break;

      case lut_order_heuristic::po:
        if ( outputs[step.target] != "0" )
        {
          circ.set_lines( circ.lines() + 1 );
          inputs.push_back( "0" );
          constants.push_back( false );
          outputs.push_back( name[step.node] );
          garbage.push_back( false );

          append_cnot( circ, step.target, circ.lines() - 1 );
        }
        else
        {
          outputs[step.target] = name[step.node];
          garbage[step.target] = false;
        }
        break;

      case lut_order_heuristic::compute:
        synthesize_node( step.node, false, step.clean_ancilla );
        break;

      case lut_order_heuristic::uncompute:
        synthesize_node( step.node, true, step.clean_ancilla );
        break;
      }
    }

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );

    set( statistics, "num_decomp_default", num_decomp_default );
    set( statistics, "num_decomp_lut", num_decomp_lut );

    return true;
  }

private:
  void synthesize_node( lut_vertex_t node, bool lookup, const std::stack<unsigned>& clean_ancilla )
  {
    /* map circuit */
    auto line_map = order_heuristic->compute_line_map( node );

    circuit local_circ;

    if ( lutdecomp )
    {
      local_circ = decomp_synthesizer.lookup_or_compute( node, lookup );
      const auto num_ancilla = local_circ.lines() - line_map.size();
      if ( num_ancilla > clean_ancilla.size() )
      {
        local_circ = synthesizer.lookup_or_compute( node, lookup );
        ++num_decomp_default;
      }
      else
      {
        auto cstack = clean_ancilla;
        for ( auto i = 0u; i < num_ancilla; ++i )
        {
          line_map.push_back( cstack.top() );
          cstack.pop();
        }
        ++num_decomp_lut;
      }
    }
    else
    {
      local_circ = synthesizer.lookup_or_compute( node, lookup );
      ++num_decomp_default;
    }

    append_circuit( circ, local_circ, gate::control_container(), line_map );
  }

private:
  circuit& circ;
  const lut_graph_t& lut;

  const properties::ptr& statistics;

  std::unordered_map<unsigned, circuit> computed_circuits;

  std::shared_ptr<lut_order_heuristic> order_heuristic;
  exorcism_lut_partial_synthesizer synthesizer;
  lutdecomp_lut_partial_synthesizer decomp_synthesizer;

  bool verbose = false;
  bool progress = false;
  bool lutdecomp = false;

private: /* statistics */
  double   synthesis_time = 0.0;
  unsigned num_decomp_default = 0u;
  unsigned num_decomp_lut = 0u;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool lut_based_synthesis( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  lut_based_synthesis_manager_new mgr( circ, gia, settings, statistics );
  const auto result = mgr.run();

  return result;
}

bool lut_based_synthesis( circuit& circ, const lut_graph_t& lut, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  lut_based_synthesis_manager mgr( circ, lut, settings, statistics );
  const auto result = mgr.run();

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
