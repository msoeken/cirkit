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
    int                  node;          /* the node to synthesize */
    unsigned             target;        /* the target line for the result */
    step_type            type;          /* which step to perform */
    std::stack<unsigned> clean_ancilla; /* number of clean ancillae */
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
    add_step( index, target, lut_order_heuristic::uncompute );
    free_constant( target );

    decrease_children_indegrees( index );
    uncompute_children( index );
  }
};

/******************************************************************************
 * Partial synthesizers                                                       *
 ******************************************************************************/

class lut_partial_synthesizer
{
public:
  explicit lut_partial_synthesizer( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : _gia( gia ),
      settings( settings ),
      statistics( statistics )
  {
  }

  circuit* lookup_or_compute( int index, unsigned free_lines, bool lookup )
  {
    if ( lookup )
    {
      return &circuits[index];
    }

    const auto circ = compute( index, free_lines );
    if ( circ == boost::none )
    {
      return nullptr;
    }
    else
    {
      return &( circuits[index] = *circ );
    }
  }

protected:
  virtual boost::optional<circuit> compute( int index, unsigned free_lines ) const = 0;

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

class exorcism_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit exorcism_lut_partial_synthesizer( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer( gia, settings, statistics ),
      dry( get( settings, "dry", dry ) ),
      verbose( get( settings, "verbose", verbose ) ),
      exorcism_runtime( settings->get<double*>( "exorcism_runtime" ) ),
      cover_runtime( settings->get<double*>( "cover_runtime" ) )
  {
  }

protected:
  boost::optional<circuit> compute( int index, unsigned free_lines ) const
  {
    if ( dry )
    {
      return circuit( gia().lut_size( index ) + 1u );
    }
    else
    {
      circuit local_circ;
      const auto lut = gia().extract_lut( index );

      auto esop = [this, &lut]() {
        increment_timer t( cover_runtime );
        return lut.compute_esop_cover();
      }();

      esop = [this, &esop, &lut]() {
        increment_timer t( exorcism_runtime );
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

  double* cover_runtime;
  double* exorcism_runtime;
};

class lutdecomp_lut_partial_synthesizer : public lut_partial_synthesizer
{
public:
  explicit lutdecomp_lut_partial_synthesizer( const gia_graph& gia, const properties::ptr& settings, const properties::ptr& statistics )
    : lut_partial_synthesizer( gia, settings, statistics ),
      class_circuits( 3u ),
      class_counter( 3u ),
      class_hash( 3u ),
      mapping_runtime( settings->get<double*>( "mapping_runtime" ) ),
      class_runtime( settings->get<double*>( "class_runtime" ) ),
      exorcism_runtime( settings->get<double*>( "exorcism_runtime" ) ),
      cover_runtime( settings->get<double*>( "cover_runtime" ) ),
      pt( "lutdecomp %w secs\n", true ),
      progress( get( settings, "verbose", progress ) )
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

    gia.init_truth_tables();
  }

  virtual ~lutdecomp_lut_partial_synthesizer()
  {
    set( statistics, "class_counter", class_counter );
  }

protected:
  boost::optional<circuit> compute( int index, unsigned free_lines ) const
  {
    const auto num_inputs = gia().lut_size( index );

    if ( num_inputs < 5 )
    {
      const auto tt_spec = gia().lut_truth_table( index );
      const auto affine_class = classify( tt_spec, num_inputs );

      return class_circuits[num_inputs - 2u][class_index[num_inputs - 2u].at( affine_class )];
    }
    else
    {
      const auto sub_lut = [this, index]() {
        increment_timer t( mapping_runtime );
        const auto lut = gia().extract_lut( index );
        return lut.if_mapping( make_settings_from( std::make_pair( "lut_size", 4u ), "area_mapping" ) );
      }();
      sub_lut.init_truth_tables();

      std::vector<unsigned> lut_to_line( sub_lut.size() );

      /* count ancillas and determine root gate */
      auto num_ancilla = sub_lut.lut_count() - 1;

      if ( num_ancilla > static_cast<int>( free_lines ) )
      {
        if ( free_lines == 0u )
        {
          return boost::none;
        }
        else
        {
          while ( num_ancilla > static_cast<int>( free_lines ) )
          {
            abc::Gia_ManMergeTopLuts( sub_lut );
            --num_ancilla;
          }
        }
      }

      auto root = abc::Gia_ObjFaninId0p( sub_lut, abc::Gia_ManCo( sub_lut, 0 ) );

      /* second pass: map LUTs to lines, and compute classes */
      auto pi_index = 0u;
      auto anc_index = num_inputs + 1u;
      auto ins_index = 0u;
      std::vector<unsigned> synth_order( 2 * num_ancilla + 1, 99 );
      std::vector<uint64_t> aff_class( sub_lut.size() );

      sub_lut.foreach_input( [&lut_to_line, &pi_index]( int index, int e ) {
          lut_to_line[index] = pi_index++;
        } );

      sub_lut.foreach_lut( [&]( int index ) {
          if ( index == root )
          {
            lut_to_line[index] = pi_index++;
            synth_order[ins_index] = index;
          }
          else
          {
            lut_to_line[index] = anc_index++;
            synth_order[ins_index] = synth_order[synth_order.size() - 1 - ins_index] = index;
            ++ins_index;
          }

          if ( sub_lut.lut_size( index ) >= 2 && sub_lut.lut_size( index ) < 5 )
          {
            aff_class[index] = classify( sub_lut.lut_truth_table( index ), sub_lut.lut_size( index ) );
          }
        } );

      circuit local_circ( num_inputs + 1 + num_ancilla );
      for ( auto index : synth_order )
      {
        const auto num_inputs = sub_lut.lut_size( index );
        std::vector<unsigned> line_map;
        line_map.reserve( num_inputs + 1u );

        sub_lut.foreach_lut_fanin( index, [&line_map, &lut_to_line]( int fanin ) {
            line_map.push_back( lut_to_line[fanin] );
          } );
        line_map.push_back( lut_to_line[index] );

        if ( num_inputs == 0 )
        {
          assert( false );
        }
        else if ( num_inputs == 1 )
        {
          assert( sub_lut.lut_truth_table( index ) == 1 );
          append_cnot( local_circ, make_var( line_map[0], false ), line_map[1] );
        }
        else if ( num_inputs < 5 )
        {
          const auto& aff_circ = class_circuits[num_inputs - 2u][class_index[num_inputs - 2u].at( aff_class[index] )];
          if ( aff_circ.lines() > num_inputs + 1u )
          {
            assert( aff_circ.lines() == num_inputs + 2u );

            // find any free line
            auto free_line = -1;
            for ( auto i = 0; i < num_inputs + 1 + num_ancilla; ++i )
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
          increment_timer t( mapping_runtime );
          append_circuit( local_circ, aff_circ, gate::control_container(), line_map );
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

          esop = [this, &esop, &lut]() {
            increment_timer t( exorcism_runtime );
            return exorcism_minimization( esop, lut.num_inputs(), lut.num_outputs() );
          }();

          circuit esop_circ;
          esop_synthesis( esop_circ, esop, lut.num_inputs(), lut.num_outputs() );
          append_circuit( local_circ, esop_circ, gate::control_container(), line_map );
          if ( progress )
          {
            std::cout << "\e[A";
          }
        }
      }

      return local_circ;
    }
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

  double* mapping_runtime;
  double* class_runtime;
  double* cover_runtime;
  double* exorcism_runtime;

  print_timer pt;

  bool progress = false;
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
      synthesizer( gia,
                   merge_properties(
                                    settings,
                                    make_settings_from(
                                                       std::make_pair( "exorcism_runtime", &exorcism_runtime ),
                                                       std::make_pair( "cover_runtime", &cover_runtime ) ) ),
                   statistics ),
      decomp_synthesizer( gia, merge_properties(
                                    settings,
                                    make_settings_from(
                                                       std::make_pair( "mapping_runtime", &mapping_runtime ),
                                                       std::make_pair( "class_runtime", &class_runtime ),
                                                       std::make_pair( "exorcism_runtime", &exorcism_runtime ),
                                                       std::make_pair( "cover_runtime", &cover_runtime ) ) ),
                          statistics ),
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
    progress_line pbar( "[i] step %5d/%5d   dd = %5d   ld = %5d   cvr = %6.2f   esop = %6.2f   map = %6.2f   clsfy = %6.2f   total = %6.2f\r", progress );
    pbar.keep_last();
    for ( const auto& step : order_heuristic->steps() )
    {
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

          append_cnot( circ, step.target, circ.lines() - 1 );
          assert( false );
        }
        else
        {
          outputs[step.target] = gia.output_name( abc::Gia_ManIdToCioId( gia, step.node ) );
          garbage[step.target] = false;

          if ( step.type == lut_order_heuristic::inv_po )
          {
            append_not( circ, step.target );
          }
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
  void synthesize_node( int index, bool lookup, const std::stack<unsigned>& clean_ancilla )
  {
    /* map circuit */
    auto line_map = order_heuristic->compute_line_map( index );
    const auto num_ancilla = clean_ancilla.size();

    circuit* local_circ;

    if ( lutdecomp )
    {
      local_circ = decomp_synthesizer.lookup_or_compute( index, num_ancilla, false );
      if ( !local_circ )
      {
        std::cout << "\n";
        local_circ = synthesizer.lookup_or_compute( index, num_ancilla, lookup );
        ++num_decomp_default;
        std::cout << "\e[A";
      }
      else
      {
        auto cstack = clean_ancilla;
        const auto req_lines = local_circ->lines() - line_map.size();
        for ( auto i = 0u; i < req_lines; ++i )
        {
          line_map.push_back( cstack.top() );
          cstack.pop();
        }
        ++num_decomp_lut;
      }
    }
    else
    {
      std::cout << "\n";
      local_circ = synthesizer.lookup_or_compute( index, num_ancilla, lookup );
      ++num_decomp_default;
      std::cout << "\e[A";
    }

    assert( local_circ );
    append_circuit( circ, *local_circ, gate::control_container(), line_map );
  }

private:
  circuit& circ;
  const gia_graph& gia;

  const properties::ptr& statistics;

  std::unordered_map<unsigned, circuit> computed_circuits;

  /* statistics */
  double   synthesis_runtime = 0.0;
  double   exorcism_runtime  = 0.0;
  double   cover_runtime     = 0.0;
  double   mapping_runtime   = 0.0;
  double   class_runtime     = 0.0;
  unsigned num_decomp_default = 0u;
  unsigned num_decomp_lut = 0u;

  std::shared_ptr<lut_order_heuristic> order_heuristic;
  exorcism_lut_partial_synthesizer synthesizer;
  lutdecomp_lut_partial_synthesizer decomp_synthesizer;

  bool verbose = false;
  bool progress = false;
  bool lutdecomp = false;
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
