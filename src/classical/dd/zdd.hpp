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

/**
 * @file zdd.hpp
 *
 * @brief ZDD package
 *
 * @author Heinz Riener
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef ZDD_HPP
#define ZDD_HPP

#include <cassert>
#include <iostream>
#include <memory>

#include <classical/dd/dd_manager.hpp>

namespace cirkit
{

class zdd_manager;

struct zdd
{
  zdd() : manager( nullptr ), index( 0u ) {}
  zdd( zdd_manager* manager, unsigned index )
    : manager( manager ),
      index( index ) {}
  zdd( const zdd& other ) : manager( other.manager ), index( other.index ) {}

  zdd& operator=( const zdd& other );

  unsigned var() const;
  zdd high() const;
  zdd low() const;

  inline bool is_bot() const { return index == 0u; }
  inline bool is_top() const { return index == 1u; }

  zdd operator-( const zdd& other ) const;
  zdd operator||( const zdd& other ) const;
  zdd operator&&( const zdd& other ) const;
  zdd operator^( const zdd& other ) const;
  zdd operator+( const zdd& other ) const;
  zdd operator*( const zdd& other ) const;
  zdd delta( const zdd& other ) const;
  zdd nonsub( const zdd& other ) const;
  zdd nonsup( const zdd& other ) const;
  zdd minhit() const;

  bool equals( const zdd& other ) const;

  zdd_manager* manager;
  unsigned     index;
};

std::ostream& operator<<( std::ostream& os, const zdd& z );

class zdd_manager : public dd_manager
{
public:
  zdd_manager( unsigned nvars, unsigned log_max_objs, bool verbose = false );
  ~zdd_manager();

  inline zdd zdd_bot()             { return zdd( this, 0u );     }
  inline zdd zdd_top()             { return zdd( this, 1u );     }
  inline zdd zdd_var( unsigned i ) { assert( i < nvars ); return zdd( this, i + 2u ); }

  unsigned zdd_diff( unsigned z1, unsigned z2 );
  unsigned zdd_union( unsigned z1, unsigned z2 );
  unsigned zdd_intersection( unsigned z1, unsigned z2 );
  unsigned zdd_symmetric_difference( unsigned z1, unsigned z2 );
  unsigned zdd_join( unsigned z1, unsigned z2 );
  unsigned zdd_meet( unsigned z1, unsigned z2 );
  unsigned zdd_delta( unsigned z1, unsigned z2 );
  unsigned zdd_nonsub( unsigned z1, unsigned z2 );
  unsigned zdd_nonsup( unsigned z1, unsigned z2 );
  unsigned zdd_minhit( unsigned z );

private:
  unsigned unique_create( unsigned var, unsigned high, unsigned low );

public:
  friend std::ostream& operator<<( std::ostream& os, const zdd_manager& mgr );
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
