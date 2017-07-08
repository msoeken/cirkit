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

#include "line_reduction.hpp"

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

#include <memory>

#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/iota.hpp>

#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/add_circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/expand_circuit.hpp>
#include <reversible/functions/find_lines.hpp>
#include <reversible/io/print_circuit.hpp>
#include <reversible/io/write_realization.hpp>
#include <reversible/io/read_realization.hpp>

#include <reversible/simulation/partial_simulation.hpp>
#include <reversible/simulation/simple_simulation.hpp>
#include <reversible/synthesis/embed_truth_table.hpp>
#include <reversible/synthesis/transformation_based_synthesis.hpp>

using namespace boost::assign;

namespace cirkit
{
#ifndef __WIN32
  template<typename Func>
  bool timed_synthesis( circuit& circ, Func& func, binary_truth_table const& spec, unsigned timeout )
  {
    if ( timeout == 0u )
    {
      return func( circ, spec );
    }

    char name[] =  "revkit_synthesis_XXXXXXX";
    int fd = mkstemp( name );

    if ( fd == -1 )
    {
      return false;
    }
    close( fd );

    pid_t pid = fork ();

    if ( pid == 0 )
    {
      struct rlimit r;
      r.rlim_cur = r.rlim_max = timeout;
      setrlimit( RLIMIT_CPU, &r );

      bool result = func( circ, spec );

      if ( result )
      {
        write_realization( circ, name );
      }

      _exit( 0 );
    }
    else
    {
      int status;
      pid_t w;

      do {
        w = waitpid( pid, &status, 0 );

        if ( w == -1 )
        {
          perror( "waitpid" );
          exit( EXIT_FAILURE );
        }

        if ( WIFEXITED( status ) ) {
        } else if ( WIFSIGNALED( status ) ) {
        } else if ( WIFSTOPPED( status ) ) {
        } else if ( WIFCONTINUED( status ) ) {
        }
      } while ( !WIFEXITED( status ) && !WIFSIGNALED( status ) );

      circuit newCirc;
      if ( !read_realization( newCirc, name ) )
        return false;
      else
        copy_circuit( newCirc, circ );

      remove( name );

      return true;
    }

    assert ( false && "This should never be reached." );
  }
#endif


  struct has_control_at
  {
    explicit has_control_at( unsigned i ) : _i( i ) {}

    bool operator()( const gate& g ) const
    {
      return boost::find_if( g.controls(), [this]( const variable& v ) { return v.line() == this->_i; } ) != boost::end( g.controls() );
    }

  private:
    unsigned _i;
  };

  struct has_control_or_target_at
  {
    explicit has_control_or_target_at( unsigned i ) : _i( i ) {}

    bool operator()( const gate& g ) const
    {
      return boost::find_if( g.controls(), [this]( const variable& v ) { return v.line() == this->_i; } ) != boost::end( g.controls() )
        || boost::find( g.targets(), _i ) != boost::end( g.targets() );
    }

  private:
    unsigned _i;
  };

  //// embed_and_synthesize ////

  embed_and_synthesize::embed_and_synthesize()
    : embedding( embed_truth_table_func() )
     , synthesis( transformation_based_synthesis_func() )
     , timeout ( 0u )
  {
  }

  bool embed_and_synthesize::operator()( circuit& circ, binary_truth_table& spec, const std::vector<unsigned>& output_order )
  {
    embedding.settings()->set( "output_order", output_order );
    if ( !embedding( spec, spec ) )
    {
      return false;
    }

#ifdef __WIN32
    return synthesis( circ, spec );
#else
    return timed_synthesis( circ, synthesis, spec, timeout );
#endif
  }

  /*
   * We assume that the circuit is optimized by preprocessing
   * and that each garbage line has at least one control as
   * last line type
   */
  unsigned find_best_garbage_line( const circuit& circ, const std::vector<unsigned>& lines_to_skip, const std::vector<unsigned>& original_lines, unsigned& last_control_position )
  {
    std::map<unsigned, unsigned> garbage_to_last_control;

    for ( unsigned i = 0u; i < circ.lines(); ++i ) {
      if ( circ.garbage().at( i ) && std::find( lines_to_skip.begin(), lines_to_skip.end(), original_lines.at( i ) ) == lines_to_skip.end() ) {
        circuit::const_reverse_iterator itPosition = std::find_if( circ.rbegin(), circ.rend(), has_control_at( i ) );

        /* unoptimized circuit? */
        if ( itPosition == circ.rend() )
        {
          continue;
        }

        unsigned position = circ.rend() - 1 - itPosition;

        garbage_to_last_control.insert( std::make_pair( i, position ) );
      }
    }

    if ( !garbage_to_last_control.size() ) {
      last_control_position = 0u;
      return circ.lines();
    } else {
      auto itMin = boost::min_element( garbage_to_last_control,
                                       []( const std::map<unsigned,unsigned>::value_type& v1,
                                           const std::map<unsigned,unsigned>::value_type& v2 ) {
                                         return v1.second < v2.second; } );
      last_control_position = itMin->second;
      return itMin->first;
    }
  }

  unsigned num_non_empty_lines( const circuit& circ, unsigned from, unsigned length )
  {
    std::set<unsigned> c;
    find_non_empty_lines( circ.begin() + from, circ.begin() + from + length, std::insert_iterator<std::set<unsigned> >( c, c.begin() ) );
    return c.size();
  }

  std::pair<circuit, std::vector<unsigned> > find_window_with_max_lines( const circuit& circ, unsigned end, unsigned max_lines )
  {
    unsigned start = end;

    while ( num_non_empty_lines( circ, start, end - start + 1 ) <= max_lines && start > 0 ) {
      --start;
    }

    if ( num_non_empty_lines( circ, start, end - start + 1 ) > max_lines ) {
      ++start;
    }

    std::vector<unsigned> filter;
    find_non_empty_lines( circ.begin() + start, circ.begin() + end + 1, std::back_inserter( filter ) );
    circuit rcircuit;
    copy_circuit( subcircuit( circ, start, end + 1 ), rcircuit, filter );
    return std::make_pair( rcircuit, filter );
  }

  unsigned find_constant_line( const circuit& circ, unsigned window_end )
  {
    unsigned best_line = circ.lines();
    unsigned min_gate_index = circ.num_gates();

    // go through all constants
    for ( std::vector<constant>::const_iterator it = circ.constants().begin(); it != circ.constants().end(); ++it )
    {
      // if constant
      if ( *it )
      {
        unsigned index = it - circ.constants().begin();

        // if line is empty until window_end
        if ( std::find_if( circ.begin(), circ.begin() + window_end, has_control_or_target_at( index ) ) == circ.begin() + window_end )
        {
          unsigned gate_index = std::find_if( circ.begin(), circ.end(), has_control_or_target_at( index ) ) - circ.begin();
          if ( gate_index >= circ.num_gates() ) // this would imply an empty line, optimization
          {
            continue;
          }

          if ( gate_index < min_gate_index )
          {
            best_line = index;
            min_gate_index = gate_index;
          }
        }
      }
    }

    return best_line;
  }

  /* returns a set of the function of the line, which is 0,1 if it is supposed to be the constant line,
     2 if it needs to be used afterwards, or -1 if it is not needed anymore. */
  void garbage_to_ov( const circuit& circ, const circuit& window, const std::vector<unsigned>& line_mapping,
                      unsigned garbage_line, std::vector<short>& ov, bool constant_value )
  {
    for ( unsigned i = 0u; i < window.lines(); ++i )
    {
      unsigned mapped_line = line_mapping.at( i );

      if ( mapped_line == garbage_line )
      {
        ov += constant_value ? 1 : 0;
      }
      else if ( !circ.garbage().at( mapped_line ) )
      {
        ov += 2;
      }
      else
      {
        if ( std::find_if( circ.begin() + window.offset() + window.num_gates(), circ.end(), has_control_or_target_at( mapped_line ) ) == circ.end() )
        {
          ov += -1;
        }
        else
        {
          ov += 2;
        }
      }
    }
  }

  bool create_window_specification( const circuit& window, binary_truth_table& window_spec, const std::vector<unsigned long long>& assignments, const std::vector<short>& ov,
                                    const simulation_func& simulation )
  {
    bool needs_simulation = boost::find( ov, 2 ) != ov.end();
    unsigned num_dcs = boost::count( ov, -1 );

    std::map<binary_truth_table::cube_type, unsigned> output_assignments;

    for ( unsigned long long i = 0ull; i < ( 1ull << window.lines() ); ++i )
    {
      // output cube
      if ( boost::find( assignments, i ) != assignments.end() ) // input has to be simulated
      {
        binary_truth_table::cube_type input_cube, output_cube;

        boost::dynamic_bitset<> simulation_input( window.lines(), i ), simulation_result;
        if ( needs_simulation )
        {
          simulation( simulation_result, window, simulation_input );
        }

        for ( std::vector<short>::const_iterator itOV = ov.begin(); itOV != ov.end(); ++itOV )
        {
          switch ( *itOV ) {
          case -1:
            break;
          case 0:
          case 1:
            output_cube += constant( *itOV == 1 );
            break;
          case 2:
          default:
            output_cube += constant( simulation_result.test( itOV - ov.begin() ) );
            break;
          }
        }

        // input cube
        for ( unsigned j = 0u; j < window.lines(); ++j )
        {
          input_cube.push_back( i & ( 1u << j ) );
        }

        window_spec.add_entry( input_cube, output_cube );

        if ( output_assignments.find( output_cube ) == output_assignments.end() )
        {
          output_assignments.insert( std::make_pair( output_cube, 1u ) );
        }
        else
        {
          if ( output_assignments[output_cube] >= ( 1u << num_dcs ) )
          {
            window_spec.clear();
            return false;
          }
          ++output_assignments[output_cube];
        }
      }
    }

    return true;
  }

  void ov_to_order_vector( const std::vector<short>& ov, std::vector<unsigned>& order )
  {
    order.clear();

    for ( std::vector<short>::const_iterator it = ov.begin(); it != ov.end(); ++it )
    {
      if ( *it != -1 )
      {
        order += ( it - ov.begin() );
      }
    }
  }

  /* removes the line line_to_remove and moves all the items on that line to line_to_use */
  void remove_line( circuit& circ, unsigned line_to_remove, unsigned line_to_use )
  {
    if ( line_to_use > line_to_remove )
    {
      --line_to_use;
    }

    for ( gate& g : circ )
    {
      // NOTE make it really possible to change the gate iterator
      // TODO negative control lines
      std::set<unsigned> c;
      for ( const auto& v : g.controls() )
      {
        assert( v.polarity() );
        if ( v.line() >= line_to_remove )
        {
          c += v.line();
        }
      }

      for ( const unsigned& control : c )
      {
        if ( control == line_to_remove )
        {
          g.remove_control( make_var( control ) );
          g.add_control( make_var( line_to_use ) );
        }
        else
        {
          g.remove_control( make_var( control ) );
          g.add_control( make_var( control - 1 ) );
        }
      }

      c.clear();
      for ( const auto& l : g.targets() )
      {
        if ( l >= line_to_remove )
        {
          c += l;
        }
      }

      for ( const unsigned& target : c )
      {
        if ( target == line_to_remove )
        {
          g.remove_target( target );
          g.add_target( line_to_use );
        }
        else
        {
          g.remove_target( target );
          g.add_target( target - 1 );
        }
      }
    }

    /* meta data */
    std::vector<std::string> inputs = circ.inputs();
    std::vector<std::string> outputs = circ.outputs();
    std::vector<constant> constants = circ.constants();
    std::vector<bool> garbage = circ.garbage();

    inputs.erase( inputs.begin() + line_to_remove );
    constants.erase( constants.begin() + line_to_remove );

    outputs.at( line_to_use ) = outputs.at( line_to_remove );
    garbage.at( line_to_use ) = garbage.at( line_to_remove );

    outputs.erase( outputs.begin() + line_to_remove );
    garbage.erase( garbage.begin() + line_to_remove );

    circ.set_lines( circ.lines() - 1 );

    circ.set_inputs( inputs );
    circ.set_outputs( outputs );
    circ.set_constants( constants );
    circ.set_garbage( garbage );
  }

  bool line_reduction( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {
    /* settings */
    const auto max_window_lines           = get( settings, "max_window_lines", 6u );
    const auto max_grow_up_window_lines   = get( settings, "max_grow_up_window_lines", 9u );
    const auto window_variables_threshold = get( settings, "window_variables_threshold", 17u );
    const auto simulation                 = get<simulation_func>( settings, "simulation", simple_simulation_func() );
    const auto window_synthesis           = get<window_synthesis_func>( settings, "window_synthesis", embed_and_synthesize() );

    /* statistics */
    auto num_considered_windows   = 0u;
    auto skipped_max_window_lines = 0u;
    auto skipped_ambiguous_line   = 0u;
    auto skipped_no_constant_line = 0u;
    auto skipped_synthesis_failed = 0u;

    properties_timer t( statistics );

    copy_circuit( base, circ );

    std::vector<unsigned> original_lines( base.lines() );
    boost::iota( original_lines, 0u );

    std::vector<unsigned> lines_to_skip;
    unsigned max_lines = max_window_lines;

    while ( true )
    {
      unsigned last_control_position;
      unsigned garbage_line = find_best_garbage_line( circ, lines_to_skip, original_lines, last_control_position );

      if ( garbage_line == circ.lines() )
      {
        break;
      }

      if ( statistics )
      {
        ++num_considered_windows;
      }

      circuit window;
      std::vector<unsigned> index_map;
      std::tie( window, index_map ) = find_window_with_max_lines( circ, last_control_position, max_lines );

      /* find constant line */
      unsigned constant_line = find_constant_line( circ, window.offset() + window.num_gates() );
      if ( constant_line == circ.lines() )
      {
        if ( statistics )
        {
          ++skipped_no_constant_line;
        }

        lines_to_skip += original_lines.at( garbage_line );
        max_lines = max_window_lines;

        continue;
      }

      /* index_map */
      std::vector<unsigned long long> assignments;

      if ( window.offset() == 0u ) // easy case: window starts on the left side
      {
        circuit zero;
        copy_circuit( subcircuit( circ, 0u, 0u ), zero, index_map );

        // non constant inputs
        unsigned non_constant_lines = 0u;
        std::vector<constant> zero_constants;
        for ( std::vector<unsigned>::const_iterator itIndexMap = index_map.begin(); itIndexMap != index_map.end(); ++itIndexMap )
        {
          zero_constants += circ.constants().at( *itIndexMap );
          if ( !circ.constants().at( *itIndexMap ) )
          {
            ++non_constant_lines;
          }
        }
        circuit zero_copy( zero.lines() );
        append_circuit( zero_copy, zero );
        zero_copy.set_constants( zero_constants );

        properties::ptr ps_settings( new properties() );
        ps_settings->set( "keep_full_output", true );

        for ( unsigned long long input = 0ull; input < ( 1ull << non_constant_lines ); ++input )
        {
          boost::dynamic_bitset<> input_vec( non_constant_lines, input ), output_vec;
          partial_simulation( output_vec, zero_copy, input_vec, ps_settings );

          assignments += output_vec.to_ulong();
        }
      }
      else
      {
        std::vector<unsigned> before_filter;
        find_non_empty_lines( circ.begin(), circ.begin() + window.offset() + window.num_gates(), std::back_inserter( before_filter ) );
        std::sort( before_filter.begin(), before_filter.end() );
        before_filter.resize( std::unique( before_filter.begin(), before_filter.end() ) - before_filter.begin() );

        /* determine the number of window variables (no constants) */
        std::vector<constant> before_window_constants;
        for ( const unsigned& line : before_filter )
        {
          before_window_constants.push_back( circ.constants().at( line ) );
        }
        unsigned window_vars = std::count( before_window_constants.begin(), before_window_constants.end(), constant() );

        if ( window_vars >= window_variables_threshold )
        {
          if ( statistics )
          {
            ++skipped_max_window_lines;
          }

          lines_to_skip += original_lines[garbage_line];
          max_lines = max_window_lines;
          continue;
        }

        /* in this case the window starts in the beginning and we need the constant inputs */
        circuit before_window_sub;
        copy_circuit( subcircuit( circ, 0u, window.offset() ), before_window_sub, before_filter );
        circuit before_window( before_window_sub.lines() );
        append_circuit( before_window, before_window_sub );
        before_window.set_constants( before_window_constants );
        before_window.set_garbage( std::vector<bool>( before_window.lines(), false ) );

        if ( window.lines() > 6 || window_vars < 12 )
        {
          properties::ptr ps_settings( new properties() );
          ps_settings->set( "keep_full_output", true );

          for ( unsigned long long input = 0ull; input < ( 1ull << window_vars ); ++input )
          {
            boost::dynamic_bitset<> input_vec( window_vars, input );
            boost::dynamic_bitset<> output;

            partial_simulation( output, before_window, input_vec, ps_settings );

            unsigned long long new_output = 0ull;

            // go through each line in window
            for ( unsigned long pos = 0; pos < index_map.size(); ++pos )
            {
              // line at pos
              unsigned line_index = index_map.at( pos );

              // this line relative in before_window
              unsigned before_window_line_index = std::find( before_filter.begin(), before_filter.end(), line_index ) - before_filter.begin();

              unsigned bit_at_before_window_line_index = output.test( before_window_line_index );

              new_output |= ( bit_at_before_window_line_index << pos );
            }

            assignments += new_output;
          }
        }
        else
        {
          lines_to_skip += original_lines[garbage_line];
          max_lines = max_window_lines;
          continue;
        }
      }

      std::vector<short> ov;
      std::vector<unsigned> order;
      garbage_to_ov( circ, window, index_map, garbage_line, ov, *circ.constants().at( constant_line ) );
      ov_to_order_vector( ov, order );

      /* create specification */
      binary_truth_table window_spec;
      if ( !create_window_specification( window, window_spec, assignments, ov, simulation ) )
      {
        if ( max_lines < max_grow_up_window_lines )
        {
          ++max_lines;
        }
        else
        {
          if ( statistics )
          {
            ++skipped_ambiguous_line;
          }

          lines_to_skip += original_lines[garbage_line];
          max_lines = max_window_lines;
        }
        continue;
      }

      circuit new_window, new_window_expanded;

      if ( !window_synthesis( new_window, window_spec, order ) )
      {
        if ( statistics )
        {
          ++skipped_synthesis_failed;
        }
        lines_to_skip += original_lines.at( garbage_line );
        max_lines = max_window_lines;
        continue;
      }

      expand_circuit( new_window, new_window_expanded, circ.lines(), index_map );

      // remember since we change the circuit
      circuit window_copy( window.lines() );
      append_circuit( window_copy, window );
      unsigned window_length = window.num_gates();
      unsigned window_offset = window.offset();

      for ( unsigned i = 0u; i < window_length; ++i )
      {
        circ.remove_gate_at( window_offset );
      }
      insert_circuit( circ, window_offset, new_window_expanded );
      remove_line( circ, constant_line, garbage_line );
    }

    if ( statistics )
    {
      statistics->set( "num_considered_windows", num_considered_windows );
      statistics->set( "skipped_max_window_lines", skipped_max_window_lines );
      statistics->set( "skipped_ambiguous_line", skipped_ambiguous_line );
      statistics->set( "skipped_no_constant_line", skipped_no_constant_line );
      statistics->set( "skipped_synthesis_failed", skipped_synthesis_failed );
    }

    return true;
  }

  optimization_func line_reduction_func( properties::ptr settings, properties::ptr statistics )
  {
    optimization_func f = [&settings, &statistics]( circuit& circ, const circuit& base ) {
      return line_reduction( circ, base, settings, statistics );
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
