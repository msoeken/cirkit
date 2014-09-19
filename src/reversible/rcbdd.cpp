/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rcbdd.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/irange.hpp>

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
    BDD gfunc = create_from_gate( g.second );
    func = ( g.first == 0u ) ? gfunc : compose( func, gfunc );
  }

  return func;
}

void rcbdd::print_truth_table()
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
    std::cout << boost::join( boost::irange( 0u, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i], 1u ); } ), "" ).insert( vars - n, " " );
    std::cout << " | ";
    std::cout << boost::join( boost::irange( 0u, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i + 1u], 1u ); } ), "" ).insert( m, " " );
    std::cout << std::endl;
  }
}

void rcbdd::write_pla( const std::string& filename )
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

  os << ".i " << n << std::endl
     << ".o " << m << std::endl
     << ".ilb " << boost::join( input_labels(), " " ) << std::endl
     << ".ob " << boost::join( output_labels(), " " ) << std::endl;

  Cudd_ForeachCube( manager().getManager(), _chi.getNode(), gen, cube, value )
  {
    os << boost::join( boost::irange( vars - n, vars ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i], 1u ); } ), "" )
       << " "
       << boost::join( boost::irange( 0u, m ) | transformed( [cube]( unsigned i ) { return std::string( "01-" ).substr( cube[3u * i + 1u], 1u ); } ), "" )
       << std::endl;
  }

  fb.close();
}


}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
