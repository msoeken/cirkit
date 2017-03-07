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

#include "reed_muller_synthesis.hpp"

#include <math.h>

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/variant.hpp>

#include <core/functor.hpp>
#include <core/utils/timer.hpp>

#include <reversible/functions/add_gates.hpp>
#include <reversible/functions/clear_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/fully_specified.hpp>
#include <reversible/io/print_circuit.hpp>

#include "synthesis_utils_p.hpp"

using namespace boost::assign;

namespace cirkit
{

  using spectra_t = std::vector<boost::dynamic_bitset<>>;

  struct to_value
  {
    using result_type = bool;

    bool operator()( const boost::optional<bool>& b ) const
    {
      return *b;
    }
  };

  template<typename CubeIterator>
  unsigned long cube_to_value( CubeIterator first, CubeIterator second )
  {
    boost::dynamic_bitset<> input;

    using boost::adaptors::transformed;
    boost::for_each( boost::make_iterator_range( first, second ) | transformed( to_value() ), [&input]( bool b ) { input.push_back( b ); } );
    return input.to_ulong();
  }

  void apply_cnot( spectra_t& f, unsigned c, unsigned t )
  {
    for ( auto& row : f )
    {
      row[t] = row[t] ^ row[c];
    }
  }

  boost::dynamic_bitset<> multiply_columns( const spectra_t& f, const std::vector<unsigned>& columns )
  {
    boost::dynamic_bitset<> m( f.size() );

    // Initial
    for ( unsigned r = 0u; r < f.size(); ++r )
    {
      m.set( r, f[r].test( columns.at( 0u ) ) );
    }

    for ( unsigned i = 1u; i < columns.size(); ++i )
    {
      boost::dynamic_bitset<> mnew( f.size(), 0 );

      for ( unsigned r = 0u; r < f.size(); ++r )
      {
        if ( m.test( r ) )
        {
          for ( unsigned r2 = 0u; r2 < f.size(); ++r2 )
          {
            if ( f[r2].test( columns.at( i ) ) )
            {
              mnew.flip( r | r2 );
            }
          }
        }
      }

      m = mnew;
    }

    return m;
  }

  void apply_toffoli( spectra_t& f, const std::vector<unsigned>& controls, unsigned t )
  {
    boost::dynamic_bitset<> c = multiply_columns( f, controls );

    for ( unsigned r = 0u; r < f.size(); ++r )
    {
      f[r][t] = f[r][t] ^ c[r];
    }
  }

  void apply_toffoli_front( spectra_t& f, const std::vector<unsigned>& controls, unsigned t )
  {
    if ( f.empty() ) return;

    // Control Mask
    unsigned cmask = 0u;
    for ( unsigned c : controls )
    {
      cmask |= 1u << c;
    }

    // Target Mask
    unsigned tmask = 1u << t;

    // for each column
    for ( unsigned j : boost::irange( 0u, (unsigned)f[0u].size() ) )
    {
      // for each row
      for ( unsigned r : boost::irange( 0u, (unsigned)f.size() ) )
      {
        // match?
        if ( ( r & tmask ) && f[r].test( j ) )
        {
          // Clear the bit
          unsigned mask = r & ~tmask;
          f[mask | cmask].flip( j );
        }
      }
    }
  }

  void apply_gate( circuit& circ, const std::vector<spectra_t*>& funcs, unsigned offset, unsigned& insert_at, const boost::variant<std::vector<unsigned>, unsigned>& controls, unsigned t )
  {
    std::vector<unsigned> _controls;

    if ( const std::vector<unsigned>* op = boost::get<std::vector<unsigned> >( &controls ) )
    {
      _controls.assign( op->begin(), op->end() );
    }
    else if ( const unsigned* op = boost::get<unsigned>( &controls ) )
    {
      _controls += *op;
    }

    switch ( _controls.size() )
    {
    case 0u:
      insert_not( circ, insert_at, t );
      insert_at += offset;
      funcs[offset]->at( 0u ).reset( t );
      apply_toffoli_front( *funcs[1u - offset], _controls, t );
      break;

    case 1u:
      insert_cnot( circ, insert_at, _controls.at( 0u ), t );
      insert_at += offset;
      apply_cnot( *funcs[offset], _controls.at( 0u ), t );
      apply_toffoli_front( *funcs[1u - offset], _controls, t );
      break;

    default:
      insert_toffoli( circ, insert_at, _controls, t );
      insert_at += offset;
      apply_toffoli( *funcs[offset], _controls, t );
      apply_toffoli_front( *funcs[1u - offset], _controls, t );
      break;
    }
  }

  void print_spectra( const spectra_t& f )
  {
    for ( unsigned i = 0u; i < f.size(); ++i )
    {
      std::cout << f[i] << std::endl;
    }
  }

  bool reed_muller_synthesis( circuit& circ, const binary_truth_table& spec, properties::ptr settings, properties::ptr statistics )
  {

    // Settings parsing
    const auto bidirectional = get( settings, "bidirectional", true );

    // Run-time measuring
    properties_timer t( statistics );

    // circuit has to be empty
    clear_circuit( circ );

    // truth table has to be fully specified
    if ( !fully_specified( spec ) )
    {
      set_error_message( statistics, "truth table `spec` is not fully specified." );
      return false;
    }

    // Determine Function Vectors from Specification
    unsigned n = spec.num_outputs();
    spectra_t func( 1u << n, boost::dynamic_bitset<>( n ) );
    spectra_t ifunc( 1u << n, boost::dynamic_bitset<>( n ) );

    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      unsigned long ipos = cube_to_value( it->first.first, it->first.second );
      binary_truth_table::cube_type output( it->second.first, it->second.second );

      for ( unsigned i = 0u; i < n; ++i )
      {
        func[ipos].set( i, *output.at( i ) );
      }

      if ( bidirectional )
      {
        unsigned long opos = cube_to_value( it->second.first, it->second.second );
        binary_truth_table::cube_type input( it->first.first, it->first.second );
        for ( unsigned i = 0u; i < n; ++i )
        {
          ifunc[opos].set( i, *input.at( i ) );
        }
      }
    }

    // Determine Reed Muller Spectra fom Function Vectors
    {
      unsigned i, j, k, m, p;
      for ( m = 1u; m < ( 1u << n ); m = 2 * m )
      {
        for ( i = 0u; i < ( 1u << n ); i = i + 2 * m )
        {
          for ( j = i, p = k = i + m; j < p; j = j + 1, k = k + 1 )
          {
            func[k] = func[k] ^ func[j];
            if ( bidirectional )
            {
              ifunc[k] = ifunc[k] ^ ifunc[j];
            }
          }
        }
      }
    }

    // Synthesis
    // copy metadata
    circ.set_lines( n );
    copy_metadata( spec, circ );

    std::vector<spectra_t*> funcs;
    funcs += &func,&ifunc;
    unsigned insert_at = 0u;

    // Step A (i = 0)
    for ( unsigned j = 0u; j < n; ++j )
    {
      unsigned offset = bidirectional && ( ifunc[0u].count() < func[0u].count() ) ? 1u : 0u;

      if ( funcs[offset]->at( 0u ).test( j ) )
      {
        apply_gate( circ, funcs, offset, insert_at, std::vector<unsigned>(), j );
      }
    }

    for ( unsigned i = 1u; i < ( 1u << n ) - 1; ++i )
    {
      // Step B (i = 2^(k-1), variable rows)
      if ( ( log( i ) / log( 2.0 ) ) == ceil( ( log( i ) / log( 2.0 ) ) ) )
      {
        unsigned k = (unsigned)ceil( ( log( i ) / log( 2.0 ) ) );
        unsigned offset = bidirectional && ( hamming_distance( i, ifunc[0u].to_ulong() ) < hamming_distance( i, func[0u].to_ulong() ) ) ? 1u : 0u;
        const boost::dynamic_bitset<>& func_offset = funcs[offset]->at( i );
        if ( !func_offset.test( k ) )
        {
          using boost::adaptors::reversed;

          unsigned s = *boost::find_if( boost::irange( 0u, n ) | reversed, [&func_offset]( unsigned j ) { return func_offset.test( j ); } );
          apply_gate( circ, funcs, offset, insert_at, s, k );
        }

        for ( unsigned j = 0u; j < n; ++j )
        {
          if ( j != k && func_offset.test( j ) )
          {
            apply_gate( circ, funcs, offset, insert_at, k, j );
          }
        }
      }
      // Step C (i != 2^(k-1), non-variable rows)
      else
      {
        unsigned offset = bidirectional && ( hamming_distance( i, ifunc[0u].to_ulong() ) < hamming_distance( i, func[0u].to_ulong() ) ) ? 1u : 0u;

        // already empty?
        if ( funcs[offset]->at( i ).none() ) continue;

        // Find s?
        using boost::adaptors::reversed;
        unsigned s = n;
        for ( unsigned j : boost::irange( 0u, n ) | reversed )
        {
          if ( funcs[offset]->at( i ).test( j ) && ( i & ( 1u << j ) ) == 0u )
          {
            s = j;
            break;
          }
        }

        assert( s != n );

        // Before CNOTs
        std::vector<unsigned> targets;
        for ( unsigned j = 0u; j < n; ++j )
        {
          if ( j != s && funcs[offset]->at( i ).test( j ) )
          {
            apply_gate( circ, funcs, offset, insert_at, s, j );
            targets += j;
          }
        }

        // Toffoli Gate
        std::vector<unsigned> controls;
        for ( unsigned j = 0u; j < n; ++j )
        {
          if ( i & ( 1u << j ) )
          {
            controls += j;
          }
        }
        apply_gate( circ, funcs, offset, insert_at, controls, s );

        // After CNOTs
        for ( unsigned j : targets )
        {
          apply_gate( circ, funcs, offset, insert_at, s, j );
        }
      }
    }

    return true;
  }

  truth_table_synthesis_func reed_muller_synthesis_func( properties::ptr settings, properties::ptr statistics )
  {
    truth_table_synthesis_func f = [&settings, &statistics]( circuit& circ, const binary_truth_table& spec ) {
      return reed_muller_synthesis( circ, spec, settings, statistics );
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
