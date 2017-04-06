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
#include <vector>

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <core/utils/conversion_utils.hpp>
#include <core/utils/graph_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/temporary_filename.hpp>
#include <core/utils/terminal.hpp>
#include <core/utils/timer.hpp>
#include <classical/abc/gia/gia.hpp>
#include <classical/abc/gia/gia_utils.hpp>
#include <classical/abc/utils/abc_run_command.hpp>
#include <classical/functions/linear_classification.hpp>
#include <classical/io/read_blif.hpp>
#include <classical/optimization/exorcism_minimization.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/gate.hpp>
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
 * Private functions                                                          *
 ******************************************************************************/

gate& append_stg_from_line_map( circuit& circ, uint64_t spec, const std::vector<unsigned>& line_map )
{
  auto& g = circ.append_gate();

  const auto num_vars = line_map.size() - 1;

  for ( auto i = 0u; i < num_vars; ++i )
  {
    g.add_control( make_var( line_map[i], true ) );
  }
  g.add_target( line_map.back() );

  stg_tag stg;
  stg.function = boost::dynamic_bitset<>( 1 << num_vars, spec );
  g.set_type( stg );

  return g;
}

/******************************************************************************
 * Merge properties                                                           *
 ******************************************************************************/

properties::ptr merge_properties( const properties::ptr& p1, const properties::ptr& p2 )
{
  const auto p = std::make_shared<properties>();
  for ( const auto& kv : *p1 )
  {
    set( p, kv.first, kv.second );
  }
  for ( const auto& kv : *p2 )
  {
    set( p, kv.first, kv.second );
  }
  return p;
}

/******************************************************************************
 * Order heuristics                                                           *
 ******************************************************************************/

class lut_order_heuristic
{
public:
  enum step_type { pi, po, inv_po, compute, uncompute };

  /* describes a single computation step */
  struct step
  {
    int                   node;          /* the node to synthesize */
    unsigned              target;        /* the target line for the result */
    step_type             type;          /* which step to perform */
    std::vector<unsigned> clean_ancilla; /* number of clean ancillae */
  };

  using step_vec = std::vector<step>;

public:
  explicit lut_order_heuristic( const gia_graph& gia )
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
        const auto driver = abc::Gia_ObjFaninLit0p( _gia, abc::Gia_ManCo( _gia, e ) );
        add_step( index, node_to_line( abc::Abc_Lit2Var( driver ) ), abc::Abc_LitIsCompl( driver ) ? step_type::inv_po : step_type::po );
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
      const auto line = _constants.back();
      _constants.pop_back();
      return line;
    }

    return _next_free++;
  }

  void add_constants( unsigned max )
  {
    while ( _next_free < max )
    {
      _constants.insert( _constants.begin(), _next_free++ );
    }
  }

  void free_constant( unsigned line )
  {
    _constants.push_back( line );
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
  std::vector<unsigned> _constants;

  /* for memory */
  std::vector<unsigned> _constants_mem;
  unsigned _next_free_mem;

  bool _dry_run = false;

protected:
  unsigned _next_free = 0u;
};

std::ostream& operator<<( std::ostream& os, const lut_order_heuristic::step& s )
{
  switch ( s.type )
  {
  case lut_order_heuristic::pi:        os << "PI"; break;
  case lut_order_heuristic::po:        os << "PO"; break;
  case lut_order_heuristic::inv_po:    os << "PO'"; break;
  case lut_order_heuristic::compute:   os << "COMPUTE"; break;
  case lut_order_heuristic::uncompute: os << "UNCOMPUTE"; break;
  }
  os << boost::format( " %d ↦ %d" ) % s.node % s.target;
  return os;
}

class defer_lut_order_heuristic : public lut_order_heuristic
{
public:
  defer_lut_order_heuristic( const gia_graph& gia )
    : lut_order_heuristic( gia )
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

        add_step( index, target, lut_order_heuristic::compute );

        /* start uncomputing */
        if ( gia().lut_ref_num( index ) == 0 )
        {
          visited.clear();
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
        output_luts.push_back( driver );
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
    if ( is_visited( index ) ) return;
    assert( gia().lut_ref_num( index ) == 0 );

    if ( !is_output_lut( index ) )
    {
      const auto target = (*this)[index];
      add_step( index, target, lut_order_heuristic::uncompute );
      free_constant( target );
    }

    visited.push_back( index );

    decrease_children_indegrees( index );
    uncompute_children( index );
  }

  void print_lut_refs()
  {
    std::cout << "[i] LUT refs:";
    gia().foreach_lut( [this]( int index ) {
        std::cout << boost::format( "  %d:%d" ) % index % gia().lut_ref_num( index );
      });
    std::cout << std::endl;
  }

private:
  inline bool is_visited( int index ) const
  {
    return std::find( visited.begin(), visited.end(), index ) != visited.end();
  }

  inline bool is_output_lut( int index ) const
  {
    return std::find( output_luts.begin(), output_luts.end(), index ) != output_luts.end();
  }

  std::vector<int> visited;
  std::vector<int> output_luts;
};

/******************************************************************************
 * Partial synthesizers                                                       *
 ******************************************************************************/

class lut_partial_synthesizer
{
public:
  explicit lut_partial_synthesizer( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : _circ( circ ),
      _gia( gia ),
      settings( settings ),
      statistics( statistics )
  {
  }

  virtual bool compute( int index, const std::vector<unsigned>& line_map, const std::vector<unsigned>& ancilas ) const = 0;

protected:
  inline circuit& circ() const
  {
    return _circ;
  }

  inline const gia_graph& gia() const
  {
    return _gia;
  }

private:
  circuit& _circ;
  const gia_graph& _gia;

protected:
  const properties::ptr& settings;
  const properties::ptr& statistics;
};

class exorcism_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit exorcism_lut_partial_synthesizer( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer( circ, gia, settings, statistics ),
      dry( get( settings, "dry", false ) ),
      optimize_esop( get( settings, "optimize_esop", true ) ),
      progress( get( settings, "progress", false ) ),
      verbose( get( settings, "verbose", false ) ),
      dumpesop( get( settings, "dumpesop", std::string() ) ),
      exorcism_runtime( settings->get<double*>( "exorcism_runtime" ) ),
      cover_runtime( settings->get<double*>( "cover_runtime" ) ),
      esopfile_counter( settings->get<unsigned*>( "esopfile_counter" ) )
  {
  }

public:
  bool compute( int index, const std::vector<unsigned>& line_map, const std::vector<unsigned>& ancilas ) const
  {
    const auto lut = gia().extract_lut( index );

    auto esop = [this, &lut]() {
      increment_timer t( cover_runtime );
      return lut.compute_esop_cover();
    }();

    if ( optimize_esop )
    {
      esop = [this, &esop, &lut]() {
        increment_timer t( exorcism_runtime );
        const auto em_settings = make_settings_from( std::make_pair( "progress", progress ) );
        return exorcism_minimization( esop, lut.num_inputs(), lut.num_outputs(), em_settings );
      }();
    }

    if ( !dumpesop.empty() )
    {
      write_esop( esop, lut.num_inputs(), lut.num_outputs(),
                  boost::str( boost::format( "%s/esop-%d.esop" ) % dumpesop % ( *esopfile_counter )++ ) );
    }

    if ( dry ) return true;
    const auto es_settings = make_settings_from( std::make_pair( "line_map", line_map ) );
    esop_synthesis( circ(), esop, lut.num_inputs(), lut.num_outputs(), es_settings );

    return true;
  }

private:
  /* settings */
  bool dry           = false; /* dry run, do everything but do not add gates */
  bool optimize_esop = true;  /* optimize ESOP cover */
  bool progress      = false; /* show progress line */
  bool verbose       = false; /* be verbose */
  std::string dumpesop;  /* dump ESOP file for each ESP cover */

  double*   cover_runtime;
  double*   exorcism_runtime;
  unsigned* esopfile_counter;
};

class lutdecomp_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit lutdecomp_lut_partial_synthesizer( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer( circ, gia, settings, statistics ),
      class_counter( 3u ),
      class_hash( 3u ),
      lut_size_max( gia.max_lut_size() ),
      satlut( get( settings, "satlut", false ) ),
      optimize_esop( get( settings, "optimize_esop", true ) ),
      dry( get( settings, "dry", false ) ),
      progress( get( settings, "progress", false ) ),
      dumpesop( get( settings, "dumpesop", std::string() ) ),
      mapping_runtime( settings->get<double*>( "mapping_runtime" ) ),
      class_runtime( settings->get<double*>( "class_runtime" ) ),
      exorcism_runtime( settings->get<double*>( "exorcism_runtime" ) ),
      cover_runtime( settings->get<double*>( "cover_runtime" ) ),
      esopfile_counter( settings->get<unsigned*>( "esopfile_counter" ) )
  {
    class_counter[0u].resize( 3u );
    class_counter[1u].resize( 6u );
    class_counter[2u].resize( 18u );

    gia.init_truth_tables();
  }

  virtual ~lutdecomp_lut_partial_synthesizer()
  {
    set( statistics, "class_counter", class_counter );
  }

  bool compute( int index, const std::vector<unsigned>& line_map, const std::vector<unsigned>& ancillas ) const
  {
    const auto num_inputs = gia().lut_size( index );

    if ( num_inputs < 5 )
    {
      const auto tt_spec = gia().lut_truth_table( index );
      const auto affine_class = classify( tt_spec, num_inputs );

      if ( dry ) return true;

      append_stg_from_line_map( circ(), affine_class, line_map );
    }
    else
    {
      const auto sub_lut = [this, index]() {
        increment_timer t( mapping_runtime );
        const auto lut = gia().extract_lut( index );
        const auto sub_lut = lut.if_mapping( make_settings_from( std::make_pair( "lut_size", 4u ), "area_mapping" ) );
        if ( satlut )
        {
          sub_lut.satlut_mapping();
        }
        return sub_lut;
      }();
      sub_lut.init_truth_tables();

      std::vector<unsigned> lut_to_line( sub_lut.size() );

      /* count ancillas and determine root gate */
      auto num_ancilla = sub_lut.lut_count() - 1; /* no need to store the output LUT */

      if ( num_ancilla > static_cast<int>( ancillas.size() ) )
      {
        if ( ancillas.empty() )
        {
          return false;
        }
        else
        {
          while ( num_ancilla > static_cast<int>( ancillas.size() ) )
          {
            abc::Gia_ManMergeTopLuts( sub_lut );
            --num_ancilla;
          }
        }
      }

      auto root = abc::Gia_ObjFaninId0p( sub_lut, abc::Gia_ManCo( sub_lut, 0 ) );
      if ( sub_lut.lut_size( root ) > num_inputs )
      {
        return false;
      }

      /* second pass: map LUTs to lines, and compute classes */
      auto pi_index = 0u;
      auto anc_index = 0u;
      auto ins_index = 0u;
      std::vector<unsigned> synth_order( 2 * num_ancilla + 1, 99 );
      std::vector<uint64_t> aff_class( sub_lut.size() );

      sub_lut.foreach_input( [&lut_to_line, &pi_index, line_map]( int index, int e ) {
          lut_to_line[index] = line_map[pi_index++];
        } );

      sub_lut.foreach_lut( [&]( int index ) {
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

          if ( sub_lut.lut_size( index ) >= 2 && sub_lut.lut_size( index ) < 5 )
          {
            aff_class[index] = classify( sub_lut.lut_truth_table( index ), sub_lut.lut_size( index ) );
          }
        } );

      for ( auto index : synth_order )
      {
        const auto num_inputs = sub_lut.lut_size( index );
        std::vector<unsigned> local_line_map;
        local_line_map.reserve( num_inputs + 1u );

        sub_lut.foreach_lut_fanin( index, [&local_line_map, &lut_to_line]( int fanin ) {
            local_line_map.push_back( lut_to_line[fanin] );
          } );
        local_line_map.push_back( lut_to_line[index] );

        if ( num_inputs == 0 )
        {
          assert( false );
        }
        else if ( num_inputs == 1 )
        {
          assert( sub_lut.lut_truth_table( index ) == 1 );
          if ( dry ) continue;
          append_cnot( circ(), make_var( local_line_map[0], false ), local_line_map[1] );
        }
        else if ( num_inputs < 5 )
        {
          if ( dry ) continue;
          auto& g = append_stg_from_line_map( circ(), aff_class[index], local_line_map );
        }
        else
        {
          if ( progress )
          {
            std::cout << "\n";
          }
          const auto lut = sub_lut.extract_lut( index );

          auto esop = [this, &lut]() {
            increment_timer t( cover_runtime );
            return lut.compute_esop_cover();
          }();

          if ( optimize_esop )
          {
            esop = [this, &esop, &lut]() {
              increment_timer t( exorcism_runtime );
              const auto em_settings = make_settings_from( std::make_pair( "progress", progress ) );
              return exorcism_minimization( esop, lut.num_inputs(), lut.num_outputs(), em_settings );
            }();
          }

          if ( !dumpesop.empty() )
          {
            write_esop( esop, lut.num_inputs(), lut.num_outputs(),
                        boost::str( boost::format( "%s/esop-%d.esop" ) % dumpesop % ( *esopfile_counter )++ ) );
          }

          if ( dry ) continue;
          const auto es_settings = make_settings_from( std::make_pair( "line_map", local_line_map ) );
          esop_synthesis( circ(), esop, lut.num_inputs(), lut.num_outputs(), es_settings );
          if ( progress )
          {
            std::cout << "\e[A";
          }
        }
      }
    }

    return true;
  }

private:
  inline uint64_t classify( uint64_t func, unsigned num_vars ) const
  {
    increment_timer t( class_runtime );

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
    ++class_counter[num_vars - 2u][optimal_quantum_circuits::affine_classification_index[num_vars - 2u].at( afunc )];
    return afunc;
  }

private:
  mutable std::vector<std::unordered_map<uint64_t, uint64_t>> class_hash;

private: /* statistics */
  mutable std::vector<std::vector<unsigned>> class_counter;

  int lut_size_max = 0;

  bool satlut        = false; /* perform SAT-based LUT mapping as post-processing step to decrease mapping size */
  bool optimize_esop = true;  /* optimize ESOP cover */
  bool dry           = false; /* dry run, do everything but do not add gates */
  bool progress      = false; /* show progress line */
  std::string dumpesop;  /* dump ESOP file for each ESP cover */

  double*   mapping_runtime;
  double*   class_runtime;
  double*   cover_runtime;
  double*   exorcism_runtime;
  unsigned* esopfile_counter;
};

/******************************************************************************
 * Manager                                                                    *
 ******************************************************************************/

class lut_based_synthesis_manager
{
public:
  lut_based_synthesis_manager( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : circ( circ ),
      gia( gia ),
      statistics( statistics ),
      order_heuristic( std::make_shared<defer_lut_order_heuristic>( gia ) ),
      synthesizer( circ, gia,
                   merge_properties(
                                    settings,
                                    make_settings_from(
                                                       std::make_pair( "exorcism_runtime", &exorcism_runtime ),
                                                       std::make_pair( "cover_runtime", &cover_runtime ),
                                                       std::make_pair( "esopfile_counter", &esopfile_counter ) ) ),
                   statistics ),
      decomp_synthesizer( circ, gia, merge_properties(
                                    settings,
                                    make_settings_from(
                                                       std::make_pair( "mapping_runtime", &mapping_runtime ),
                                                       std::make_pair( "class_runtime", &class_runtime ),
                                                       std::make_pair( "exorcism_runtime", &exorcism_runtime ),
                                                       std::make_pair( "cover_runtime", &cover_runtime ),
                                                       std::make_pair( "esopfile_counter", &esopfile_counter ) ) ),
                          statistics ),
      verbose( get( settings, "verbose", false ) ),
      progress( get( settings, "progress", false ) ),
      lutdecomp( get( settings, "lutdecomp", false ) ),
      pbar( "[i] step %5d/%5d   dd = %5d   ld = %5d   cvr = %6.2f   esop = %6.2f   map = %6.2f   clsfy = %6.2f   total = %6.2f", progress )
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

    std::unordered_map<unsigned, lut_order_heuristic::step_type> orig_step_type; /* first step type of an output */

    auto step_index = 0u;
    pbar.keep_last();
    for ( const auto& step : order_heuristic->steps() )
    {
      if ( verbose )
      {
        std::cout << step << std::endl;
      }
      pbar( ++step_index, order_heuristic->steps().size(), num_decomp_default, num_decomp_lut, cover_runtime, exorcism_runtime, mapping_runtime, class_runtime, synthesis_runtime );
      increment_timer t( &synthesis_runtime );

      switch ( step.type )
      {
      case lut_order_heuristic::pi:
        inputs[step.target] = outputs[step.target] = gia.input_name( abc::Gia_ManIdToCioId( gia, step.node ) );
        constants[step.target] = boost::none;
        break;

      case lut_order_heuristic::po:
      case lut_order_heuristic::inv_po:
        if ( outputs[step.target] != "0" )
        {
          circ.set_lines( circ.lines() + 1 );
          inputs.push_back( "0" );
          constants.push_back( false );
          outputs.push_back( gia.output_name( abc::Gia_ManIdToCioId( gia, step.node ) ) );
          garbage.push_back( false );

          const auto pol = orig_step_type[step.target] == step.type ? true : false;
          append_cnot( circ, make_var( step.target, pol ), circ.lines() - 1 );
        }
        else
        {
          outputs[step.target] = gia.output_name( abc::Gia_ManIdToCioId( gia, step.node ) );
          garbage[step.target] = false;

          if ( step.type == lut_order_heuristic::inv_po )
          {
            append_not( circ, step.target );
          }
          orig_step_type.insert( {step.target, step.type} );
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
    set( statistics, "exorcism_runtime", exorcism_runtime );
    set( statistics, "cover_runtime", cover_runtime );
    set( statistics, "class_runtime", class_runtime );
    set( statistics, "mapping_runtime", mapping_runtime );
    return true;
  }

private:
  void synthesize_node( int index, bool lookup, const std::vector<unsigned>& clean_ancilla )
  {
    /* map circuit */
    const auto line_map = order_heuristic->compute_line_map( index );

    if ( lutdecomp && decomp_synthesizer.compute( index, line_map, clean_ancilla ) )
    {
      ++num_decomp_lut;
      return;
    }

    {
      const auto sp = pbar.subprogress();
      synthesizer.compute( index, line_map, clean_ancilla );
      ++num_decomp_default;
    }
  }

private:
  circuit& circ;
  const gia_graph& gia;

  const properties::ptr& statistics;

  std::unordered_map<unsigned, circuit> computed_circuits;

  /* statistics */
  double   synthesis_runtime  = 0.0;
  double   exorcism_runtime   = 0.0;
  double   cover_runtime      = 0.0;
  double   mapping_runtime    = 0.0;
  double   class_runtime      = 0.0;
  unsigned num_decomp_default = 0u;
  unsigned num_decomp_lut     = 0u;
  unsigned esopfile_counter   = 0u; /* if ESOP files should be dumbed, this counter is increased */

  std::shared_ptr<lut_order_heuristic> order_heuristic;
  exorcism_lut_partial_synthesizer synthesizer;
  lutdecomp_lut_partial_synthesizer decomp_synthesizer;

  /* settings (other settings are passed to decomposition classes directly and parsed there) */
  bool verbose = false;    /* be verbose */
  bool progress = false;   /* show progress line */
  bool lutdecomp = false;  /* enable LUT-based decomposition */

  progress_line pbar;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

bool lut_based_synthesis( circuit& circ, const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
{
  /* timing */
  properties_timer t( statistics );

  lut_based_synthesis_manager mgr( circ, gia, settings, statistics );
  const auto result = mgr.run();

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
