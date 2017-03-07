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

#include "rcbdd.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/counting_range.hpp>

#include <core/utils/range_utils.hpp>
#include <reversible/target_tags.hpp>

namespace cirkit
{

void rcbdd::initialize_manager()
{
  if ( !_manager )
  {
    _manager = Cudd();
  }
}

void rcbdd::create_variables( unsigned n, bool create_zs )
{
  for (unsigned i = _n; i < n; ++i)
  {
    _xs += _manager->bddVar();
    _ys += _manager->bddVar();
    if ( create_zs )
    {
      _zs += _manager->bddVar();
    }
  }

  _n = n;
}

BDD rcbdd::x( unsigned i ) const
{
  return _xs.at( i );
}

BDD rcbdd::y( unsigned i ) const
{
  return _ys.at( i );
}

BDD rcbdd::z( unsigned i ) const
{
  return _zs.at( i );
}

const std::vector<BDD> rcbdd::xs() const
{
  return _xs;
}

const std::vector<BDD> rcbdd::ys() const
{
  return _ys;
}

const std::vector<BDD> rcbdd::zs() const
{
  return _zs;
}

unsigned rcbdd::num_vars() const
{
  return _n;
}

const Cudd& rcbdd::manager() const
{
  return *_manager;
}

BDD rcbdd::chi() const
{
  return _chi;
}

void rcbdd::set_chi( BDD f )
{
  _chi = f;
}

void rcbdd::set_constant_value( bool v )
{
  _constant_value = v;
}

bool rcbdd::constant_value() const
{
  return _constant_value;
}

void rcbdd::set_num_inputs( unsigned n )
{
  _num_inputs = n;
}

void rcbdd::set_num_outputs( unsigned n )
{
  _num_outputs = n;
}

unsigned rcbdd::num_inputs() const
{
  return _num_inputs;
}

unsigned rcbdd::num_outputs() const
{
  return _num_outputs;
}

void rcbdd::set_input_labels( const std::vector<std::string>& labels )
{
  _input_labels.clear();
  boost::push_back( _input_labels, labels );
}

void rcbdd::set_output_labels( const std::vector<std::string>& labels )
{
  _output_labels.clear();
  boost::push_back( _output_labels, labels );
}

const std::vector<std::string> rcbdd::input_labels() const
{
  return _input_labels;
}

const std::vector<std::string> rcbdd::output_labels() const
{
  return _output_labels;
}

BDD rcbdd::cofactor( BDD f, unsigned var, bool input_polarity, bool output_polarity ) const
{
  return f.Cofactor(input_polarity ? _xs[var] : !_xs[var]).Cofactor(output_polarity ? _ys[var] : !_ys[var]);
}

BDD rcbdd::compose(const BDD& left, const BDD& right) const
{
  BDD fzs = _manager->bddOne();
  boost::for_each(_zs, [&fzs](const BDD& z){ fzs &= z; });
  return (move_ys_to_tmp(left) & move_xs_to_tmp(right)).ExistAbstract(fzs);
}

BDD rcbdd::move_xs_to_tmp(const BDD& f) const
{
  BDD func = f;

  for (unsigned i = 0u; i < _n; ++i)
  {
    func &= _xs[i].Xnor(_zs[i]);
  }

  return remove_xs(func);
}

BDD rcbdd::move_ys_to_tmp(const BDD& f) const
{
  BDD func = f;

  for (unsigned i = 0u; i < _n; ++i)
  {
    func &= _ys[i].Xnor(_zs[i]);
  }

  return remove_ys(func);
}

BDD rcbdd::move_tmp_to_ys(const BDD& f) const
{
  BDD func = f;

  for (unsigned i = 0u; i < _n; ++i)
  {
    func &= _ys[i].Xnor(_zs[i]);
  }

  return remove_tmp(func);
}

BDD rcbdd::move_ys_to_xs(const BDD& f) const
{
  BDD func = f;

  for (unsigned i = 0u; i < _n; ++i)
  {
    func &= _ys[i].Xnor(_xs[i]);
  }

  return remove_ys(func);
}

BDD rcbdd::remove_xs(const BDD& f) const
{
  BDD fxs = _manager->bddOne();
  boost::for_each(_xs, [&fxs](const BDD& x){ fxs &= x; });
  return f.ExistAbstract(fxs);
}

BDD rcbdd::remove_ys(const BDD& f) const
{
  BDD fys = _manager->bddOne();
  boost::for_each(_ys, [&fys](const BDD& y){ fys &= y; });
  return f.ExistAbstract(fys);
}

BDD rcbdd::remove_tmp(const BDD& f) const
{
  BDD fzs = _manager->bddOne();
  boost::for_each(_zs, [&fzs](const BDD& z){ fzs &= z; });
  return f.ExistAbstract(fzs);
}

BDD rcbdd::invert( const BDD&f ) const
{
  return move_tmp_to_ys( move_ys_to_xs( move_xs_to_tmp( f ) ) );
}

bool rcbdd::is_self_inverse( const BDD& f ) const
{
  return f == invert( f );
}

BDD rcbdd::create_from_gate( unsigned target, const BDD& controlf ) const
{
  BDD func = _manager->bddOne();

  for (unsigned i = 0u; i < _n; ++i)
  {
    if (i == target)
    {
      func &= _ys[i].Xnor(_xs[i] ^ controlf);
    }
    else
    {
      func &= _ys[i].Xnor(_xs[i]);
    }
  }

  return func;
}

BDD rcbdd::create_from_gate( const gate& g ) const
{
  assert( is_toffoli( g ) );

  BDD func = _manager->bddOne();

  for (unsigned i = 0u; i < _n; ++i)
  {
    if (i == g.targets().front())
    {
      BDD controlf = _manager->bddOne();
      for ( const auto& c : g.controls() )
      {
        controlf &= c.polarity() ? _xs[c.line()] : !_xs[c.line()];
      }
      func &= _ys[i].Xnor(_xs[i] ^ controlf);
    }
    else
    {
      func &= _ys[i].Xnor(_xs[i]);
    }
  }

  return func;
}

BDD rcbdd::create_from_circuit( const circuit& circ ) const
{
  BDD func;

  for ( const auto& g : index( circ ) )
  {
    BDD gfunc = create_from_gate( g.value );
    func = ( g.index == 0u ) ? gfunc : compose( func, gfunc );
  }

  return func;
}

void rcbdd::print_truth_table() const
{
  using boost::adaptors::transformed;

  DdGen *gen;
  int  *cube;
  CUDD_VALUE_TYPE value;

  unsigned vars = num_vars();
  unsigned n = num_inputs();
  unsigned m = num_outputs();
  std::cout << std::string( vars - n, 'c' )
            << ' '
            << std::string( n, 'x' )
            << " | "
            << std::string( m, 'y' )
            << ' '
            << std::string( vars - m, 'g' )
            << std::endl;
  std::cout << std::string( vars + 2u, '-' ) << '+' << std::string( vars + 2u, '-' ) << std::endl;

  Cudd_ForeachCube( manager().getManager(), _chi.getNode(), gen, cube, value )
  {
    std::cout << boost::join( boost::counting_range( 0u, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i], 1u ); } ), "" ).insert( vars - n, " " );
    std::cout << " | ";
    std::cout << boost::join( boost::counting_range( 0u, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i + 1u], 1u ); } ), "" ).insert( m, " " );
    std::cout << std::endl;
  }
}

void rcbdd::write_pla( const std::string& filename, bool full ) const
{
  using boost::adaptors::transformed;

  DdGen *gen;
  int  *cube;
  CUDD_VALUE_TYPE value;

  unsigned vars = num_vars();
  unsigned n = num_inputs();
  unsigned m = num_outputs();

  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );

  if ( full )
  {
    os << ".i " << vars << std::endl
       << ".o " << vars << std::endl
       << ".ilb";

    for ( auto i = 0u; i < vars - n; ++i )
    {
      os << " kappa" << ( i + 1 );
    }
    os << " " << boost::join( input_labels(), " " ) << std::endl
       << ".ob " << boost::join( output_labels(), " " );

    for ( auto i = 0u; i < vars - m; ++i )
    {
      os << " gamma" << ( i + 1 );
    }
    os << std::endl;
  }
  else
  {
    os << ".i " << n << std::endl
       << ".o " << m << std::endl
       << ".ilb " << boost::join( input_labels(), " " ) << std::endl
       << ".ob " << boost::join( output_labels(), " " ) << std::endl;
  }

  Cudd_ForeachCube( manager().getManager(), _chi.getNode(), gen, cube, value )
  {
    os << boost::join( boost::counting_range( full ? 0 : vars - n, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i], 1u ); } ), "" )
       << " "
       << boost::join( boost::counting_range( 0u, full ? vars : m ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i + 1u], 1u ); } ), "" )
       << std::endl;
  }

  fb.close();
}

void copy_meta_data( circuit& circ, const rcbdd& cf )
{
  circ.set_lines( cf.num_vars() );

  std::vector<std::string> inputs( cf.num_vars(), cf.constant_value() ? "1" : "0" );
  boost::copy( cf.input_labels(), inputs.end() - cf.num_inputs() );
  circ.set_inputs( inputs );

  std::vector<std::string> outputs( cf.num_vars(), "-" );
  boost::copy( cf.output_labels(), outputs.begin() );
  circ.set_outputs( outputs );

  std::vector<constant> constants( cf.num_vars(), constant() );
  std::fill( constants.begin(), constants.end() - cf.num_inputs(), cf.constant_value() );
  circ.set_constants( constants );

  std::vector<bool> garbage( cf.num_vars(), true );
  std::fill( garbage.begin(), garbage.begin() + cf.num_outputs(), false );
  circ.set_garbage( garbage );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
