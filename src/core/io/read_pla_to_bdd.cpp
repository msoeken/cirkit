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

#include "read_pla_to_bdd.hpp"

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/counting_range.hpp>

#include <core/utils/timer.hpp>

#include "pla_parser.hpp"

using namespace boost::assign;

namespace cirkit
{

  struct pla_t
  {
    boost::optional<unsigned> num_inputs;
    boost::optional<unsigned> num_outputs;
    std::vector<std::string> input_labels;
    std::vector<std::string> output_labels;
    std::string type;
    std::vector<std::pair<std::string, std::string> > cubes;
  };

  class parse_pla_processor : public pla_processor
  {
  public:
    parse_pla_processor( pla_t& _pla ) : pla( _pla ) {}

    void on_num_inputs( unsigned num_inputs )
    {
      pla.num_inputs = num_inputs;
    }

    void on_num_outputs( unsigned num_outputs )
    {
      pla.num_outputs = num_outputs;
    }

    void on_input_labels( const std::vector<std::string>& input_labels )
    {
      pla.input_labels = input_labels;
      if ( !pla.num_inputs ) pla.num_inputs = input_labels.size();
    }

    void on_output_labels( const std::vector<std::string>& output_labels )
    {
      pla.output_labels = output_labels;
      if ( !pla.num_outputs ) pla.num_outputs = output_labels.size();
    }

    void on_type( const std::string& type )
    {
      pla.type = type;
    }

    void on_cube( const std::string& in, const std::string& out )
    {
      pla.cubes += std::make_pair( in, out );
    }
  private:
    pla_t& pla;
  };

  bool semantic_parse( pla_t& p )
  {
    // TODO transform into error messages
    assert( p.num_inputs );
    assert( p.num_outputs );

    // Auto Generate input and output labels if they do not exist
    if ( p.input_labels.empty() )
    {
      for ( unsigned i : boost::counting_range( 0u, *p.num_inputs ) )
      {
        p.input_labels += boost::str( boost::format( "i%d" ) % i );
      }
    }

    if ( p.output_labels.empty() )
    {
      for ( unsigned i : boost::counting_range( 0u, *p.num_outputs ) )
      {
        p.output_labels += boost::str( boost::format( "o%d" ) % i );
      }
    }

    // TODO transform into error messages
    assert( *p.num_inputs == p.input_labels.size() );
    assert( *p.num_outputs == p.output_labels.size() );

    return true;
  }

  bool parse( pla_t& pla, const std::string& filename )
  {
    std::ifstream is;
    is.open( filename.c_str(), std::ifstream::in );

    parse_pla_processor p( pla );
    pla_parser( is, p );

    if ( !semantic_parse( pla ) )
    {
      return false;
    }

    return true;
  }

  bool read_pla_to_bdd( BDDTable& bdd, const std::string& filename,
                        const properties::ptr& settings,
                        const properties::ptr& statistics )
  {
    using boost::adaptors::map_values;

    /* settings */
    auto input_generation_func = get( settings, "input_generation_func", generation_func_type( []( DdManager* manager, unsigned pos ) {
          return Cudd_bddNewVar( manager ); } ) );
    auto ordering              = get( settings, "ordering",              std::vector<unsigned>() );

    /* timing */
    properties_timer t( statistics );

    pla_t pla;

    if ( !parse( pla, filename ) )
    {
      return false;
    }

    // Check ordering
    assert( ordering.empty() || ordering.size() == *pla.num_inputs );

    // Inputs
    boost::transform( pla.input_labels,
                      std::back_inserter( bdd.inputs ),
                      []( const std::string& label ) { return std::make_pair( label, (DdNode*)0 ); } );
    auto pos = 0u;
    boost::generate( bdd.inputs | map_values, [&]() { return input_generation_func( bdd.cudd, pos++ ); } );

    // Outputs
    boost::transform( pla.output_labels,
                      std::back_inserter( bdd.outputs ),
                      [&]( const std::string& label ) { return std::make_pair( label, Cudd_ReadLogicZero( bdd.cudd ) ); } );
    boost::for_each( bdd.outputs | map_values, Cudd_Ref );


    // Iterate through cubes
    for ( const auto& cube : pla.cubes )
    {
      const auto& in = cube.first;
      const auto& out = cube.second;

      DdNode *tmp, *var;
      DdNode* prod = Cudd_ReadOne( bdd.cudd );
      Cudd_Ref( prod );

      for ( auto i = 0u; i < *pla.num_inputs; ++i )
      {
        if ( in[i] == '-' ) continue;

        var = ordering.empty() ? bdd.inputs[i].second : bdd.inputs[ordering[i]].second;
        Cudd_Ref( var );
        if ( in[i] == '0' ) var = Cudd_Not( var );
        tmp = Cudd_bddAnd( bdd.cudd, prod, var );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, prod );
        Cudd_RecursiveDeref( bdd.cudd, var );
        prod = tmp;
      }

      for ( auto i = 0u; i < *pla.num_outputs; ++i )
      {
        if ( out[i] == '0' || out[i] == '~' ) continue;

        tmp = Cudd_bddOr( bdd.cudd, bdd.outputs[i].second, prod );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, bdd.outputs[i].second );
        bdd.outputs[i].second = tmp;
      }

      Cudd_RecursiveDeref( bdd.cudd, prod );
    }

    return true;
  }

  bool read_pla_to_characteristic_bdd( BDDTable& bdd, const std::string& filename, bool inputs_first, bool with_output_zero_patterns )
  {
    using boost::adaptors::map_values;

    pla_t pla;

    if ( !parse( pla, filename ) )
    {
      return false;
    }

    // Variables
    std::vector<std::string> labels;
    if ( inputs_first )
    {
      boost::push_back( labels, pla.input_labels );
      boost::push_back( labels, pla.output_labels );
    }
    else
    {
      boost::push_back( labels, pla.output_labels );
      boost::push_back( labels, pla.input_labels );
    }

    // Inputs
    boost::transform( labels,
                      std::back_inserter( bdd.inputs ),
                      []( const std::string& label ) { return std::make_pair( label, (DdNode*)0 ); } );
    boost::generate( bdd.inputs | map_values, [&]() { return Cudd_bddNewVar( bdd.cudd ); } );

    auto xnodes = inputs_first ? boost::make_iterator_range( bdd.inputs.begin(), bdd.inputs.begin() + *pla.num_inputs )
                               : boost::make_iterator_range( bdd.inputs.begin() + *pla.num_outputs, bdd.inputs.end() );
    auto ynodes = inputs_first ? boost::make_iterator_range( bdd.inputs.begin() + *pla.num_inputs, bdd.inputs.end() )
                               : boost::make_iterator_range( bdd.inputs.begin(), bdd.inputs.begin() + *pla.num_outputs );

    // Outputs
    DdNode *f = Cudd_ReadLogicZero( bdd.cudd );
    Cudd_Ref( f );

    // Intermediate BDDs
    DdNode *ys = Cudd_ReadOne( bdd.cudd ), *tmp, *tmp2;
    for ( const auto& node : ynodes )
    {
      tmp = Cudd_bddAnd( bdd.cudd, ys, node.second );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( bdd.cudd, ys );
      ys = tmp;
    }

    // Iterate through cubes
    for ( const auto& cube : pla.cubes )
    {
      const std::string& in = cube.first;
      const std::string& out = cube.second;

      // Input patterns of f
      DdNode *h = Cudd_bddExistAbstract( bdd.cudd, f, ys );
      Cudd_Ref( h );

      // Input cube
      DdNode* input = Cudd_ReadOne( bdd.cudd );
      Cudd_Ref( input );

      for ( unsigned i = 0u; i < *pla.num_inputs; ++i )
      {
        if ( in[i] == '-' ) continue;

        tmp = Cudd_bddAnd( bdd.cudd, input, in[i] == '0' ? Cudd_Not( xnodes[i].second ) : xnodes[i].second );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, input );
        input = tmp;
      }

      // Output cube
      DdNode *output = Cudd_ReadOne( bdd.cudd );
      Cudd_Ref( output );

      for ( unsigned i = 0u; i < *pla.num_outputs; ++i )
      {
        if ( out[i] == '0' || out[i] == '~' || out[i] == '-' ) continue;

        tmp = Cudd_bddAnd( bdd.cudd, output, ynodes[i].second );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, output );
        output = tmp;
      }

      // New outputs
      tmp = Cudd_bddAnd( bdd.cudd, Cudd_Not( h ), input );
      Cudd_Ref( tmp );
      tmp2 = Cudd_bddAnd( bdd.cudd, tmp, output );
      Cudd_Ref( tmp2 );
      Cudd_RecursiveDeref( bdd.cudd, tmp );
      tmp = Cudd_bddOr( bdd.cudd, f, tmp2 );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( bdd.cudd, f );
      Cudd_RecursiveDeref( bdd.cudd, tmp2 );
      f = tmp;

      // Update outputs
      tmp = Cudd_bddAnd( bdd.cudd, h, input );
      Cudd_Ref( tmp );
      tmp2 = Cudd_bddOr( bdd.cudd, Cudd_Not( tmp ), output ); // h => output
      Cudd_Ref( tmp2 );
      Cudd_RecursiveDeref( bdd.cudd, tmp );
      tmp = Cudd_bddAnd( bdd.cudd, f, tmp2 );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( bdd.cudd, tmp2 );
      f = tmp;

      // Cleanup
      Cudd_RecursiveDeref( bdd.cudd, input );
      Cudd_RecursiveDeref( bdd.cudd, output );
      Cudd_RecursiveDeref( bdd.cudd, h );
    }

    // Assign 0s
    for ( unsigned i = 0u; i < *pla.num_outputs; ++i )
    {
      DdNode *var = ynodes[i].second;
      DdNode *f0, *f1, *lhs, *rhs;
      f0 = Cudd_Cofactor( bdd.cudd, f, Cudd_Not( var ) );
      Cudd_Ref( f0 );
      f1 = Cudd_Cofactor( bdd.cudd, f, var );
      Cudd_Ref( f1 );
      tmp = Cudd_bddAnd( bdd.cudd, f1, Cudd_Not( f0 ) );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( bdd.cudd, f0 );
      Cudd_RecursiveDeref( bdd.cudd, f1 );
      lhs = Cudd_bddAnd( bdd.cudd, tmp, var );
      Cudd_Ref( lhs );
      Cudd_RecursiveDeref( bdd.cudd, tmp );

      tmp = Cudd_bddUnivAbstract( bdd.cudd, f, var );
      Cudd_Ref( tmp );
      rhs = Cudd_bddAnd( bdd.cudd, tmp, Cudd_Not( var ) );
      Cudd_Ref( rhs );
      Cudd_RecursiveDeref( bdd.cudd, tmp );

      tmp = Cudd_bddOr( bdd.cudd, lhs, rhs );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( bdd.cudd, f );
      Cudd_RecursiveDeref( bdd.cudd, lhs );
      Cudd_RecursiveDeref( bdd.cudd, rhs );
      f = tmp;
    }

    if ( with_output_zero_patterns )
    {
      // Input patterns of f
      DdNode *h = Cudd_Not( Cudd_bddExistAbstract( bdd.cudd, f, ys ) );
      Cudd_Ref( h );

      DdNode *yzero = Cudd_ReadOne( bdd.cudd );
      for ( const auto& node : ynodes )
      {
        tmp = Cudd_bddAnd( bdd.cudd, yzero, Cudd_Not( node.second ) );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, yzero );
        yzero = tmp;
      }

      tmp = Cudd_bddAnd( bdd.cudd, yzero, h );
      Cudd_Ref( tmp );
      Cudd_RecursiveDeref( bdd.cudd, yzero );
      Cudd_RecursiveDeref( bdd.cudd, h );

      tmp2 = Cudd_bddOr( bdd.cudd, f, tmp );
      Cudd_Ref( tmp2 );
      Cudd_RecursiveDeref( bdd.cudd, f );
      Cudd_RecursiveDeref( bdd.cudd, tmp );
      f = tmp2;
    }

    bdd.outputs += std::make_pair( "f", f );
    bdd.num_real_outputs = *pla.num_outputs;

    Cudd_RecursiveDeref( bdd.cudd, ys );
    boost::for_each( bdd.inputs | map_values, [&](DdNode* node) { Cudd_RecursiveDeref( bdd.cudd, node ); } );

    return true;
  }

  BDDTable::BDDTable()
  {
    cudd = Cudd_Init( 0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
  }

  BDDTable::BDDTable( DdManager * manager )
    : external_manager( true )
  {
    cudd = manager;
  }

  BDDTable::~BDDTable()
  {
    if ( !external_manager )
    {
      using boost::adaptors::map_values;

      boost::for_each( outputs | map_values, [this]( DdNode* node ) { Cudd_RecursiveDeref( this->cudd, node ); } );
      Cudd_Quit( cudd );
    }
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
