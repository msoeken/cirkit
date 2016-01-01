/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
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

/**
 * @file bdd.hpp
 *
 * @brief BDD package
 *
 * @author Mathias Soeken
 * @since  2.3
 */
#ifndef BDD_HPP
#define BDD_HPP

#include <classical/dd/dd_manager.hpp>
#include <classical/dd/bdd_fwd.hpp>

#include <boost/call_traits.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <cassert>
#include <iostream>
#include <map>
#include <memory>

namespace cirkit
{

class bdd_manager;

struct bdd
{
  using const_param_ref = boost::call_traits < bdd >::const_reference;

  bdd() : manager( nullptr ), index( 0u ) {}
  bdd( bdd_manager* manager, unsigned index )
    : manager( manager ),
      index( index ) {}
  bdd( const bdd& other ) : manager( other.manager ), index( other.index ) {}

  bdd& operator=( const bdd& other );

  unsigned var() const;
  bdd high() const;
  bdd low() const;

  inline bool is_bot() const { return index == 0u; }
  inline bool is_top() const { return index == 1u; }

  bdd operator&&( const bdd& other ) const;
  bdd operator||( const bdd& other ) const;
  bdd operator^( const bdd& other ) const;
  bdd operator!() const;
  bdd cof0( unsigned v ) const;
  bdd cof1( unsigned v ) const;
  bdd exists( const bdd& other ) const;
  bdd constrain( const bdd& other ) const;
  bdd restrict( const bdd& other ) const;
  bdd round_down( unsigned level ) const;
  bdd round_up( unsigned level ) const;
  bdd round( unsigned level ) const;

  bool equals( const bdd& other ) const;

  bdd_manager* manager;
  unsigned     index;
};

std::ostream& operator<< ( std::ostream& stream, bdd::const_param_ref bdd );

class bdd_manager : public dd_manager
{
public:
  bdd_manager( unsigned nvars, unsigned log_max_objs, bool verbose = false );
  ~bdd_manager();

  bdd_manager ( const bdd_manager& other ) = delete;
  bdd_manager& operator= ( const bdd_manager& other ) = delete;

  inline bdd bdd_bot()                { return bdd( this, 0u );     }
  inline bdd bdd_top()                { return bdd( this, 1u );     }
  inline bdd bdd_var( unsigned i )    { assert( i < nvars ); return bdd( this, i + 2u ); }
  inline bdd operator[]( unsigned i ) { assert( i < nvars ); return bdd( this, i + 2u ); }

  unsigned bdd_and( unsigned f, unsigned g );
  unsigned bdd_or( unsigned f, unsigned g );
  unsigned bdd_xor( unsigned f, unsigned g );
  unsigned bdd_not( unsigned f );
  unsigned bdd_cof0( unsigned f, unsigned v );
  unsigned bdd_cof1( unsigned f, unsigned v );
  unsigned bdd_exists( unsigned f, unsigned g );
  unsigned bdd_constrain( unsigned f, unsigned g );
  unsigned bdd_restrict( unsigned f, unsigned g );
  unsigned bdd_round_down( unsigned f, unsigned level );
  unsigned bdd_round_up( unsigned f, unsigned level );
  unsigned bdd_round( unsigned f, unsigned level );

  unsigned unique_create( unsigned var, unsigned high, unsigned low );

  static bdd_manager_ptr create( unsigned nvars, unsigned log_max_objs, bool verbose = false );

private:
  unsigned bdd_round_to( unsigned f, unsigned level, unsigned cop, unsigned to );
  unsigned bdd_round_to( unsigned f, unsigned level, unsigned cop, unsigned to, const std::map<unsigned, boost::multiprecision::uint256_t>& count_map );

public:
  friend std::ostream& operator<<( std::ostream& os, const bdd_manager& mgr );
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
