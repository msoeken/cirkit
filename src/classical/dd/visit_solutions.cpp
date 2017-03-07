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

#include "visit_solutions.hpp"

#include <map>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/logic/tribool_io.hpp>

#include <core/utils/bitset_utils.hpp>

namespace std
{

template<>
struct less<std::vector<boost::tribool>>
{
  bool operator()( const std::vector<boost::tribool>& v1, const std::vector<boost::tribool>& v2 ) const
  {
    assert( v1.size() == v2.size() );

    auto i = 0u;
    while ( i < v1.size() )
    {
      const auto& t1 = v1[i];
      const auto& t2 = v2[i];

      if ( t1 != t2 )
      {
        if ( boost::indeterminate( t1 ) ) { return true; }
        if ( boost::indeterminate( t2 ) ) { return false; }
        return !t1 && t2;
      }

      ++i;
    }

    return true;
  }
};

}

namespace cirkit
{

using boost::adaptors::transformed;
using boost::tribool;

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void visit_solutions_rec( unsigned level, const bdd& n, boost::dynamic_bitset<>& x, const visit_solutions_func& f )
{
  if ( n.index == 0u )
  {
    return;
  }
  if ( n.var() > level )
  {
    x.reset( level ); visit_solutions_rec( level + 1u, n, x, f );
    x.set( level );   visit_solutions_rec( level + 1u, n, x, f );
  }
  else if ( n.index == 1u )
  {
    f( x );
  }
  else
  {
    if ( n.low().index != 0u )
    {
      x.reset( level ); visit_solutions_rec( level + 1u, n.low(), x, f );
    }
    if ( n.high().index != 0u )
    {
      x.set( level );   visit_solutions_rec( level + 1u, n.high(), x, f );
    }
  }
}

void visit_paths_rec( const bdd& n, std::vector<tribool>& x, const visit_paths_func& f )
{
  switch ( n.index )
  {
  case 0u:
    return;
  case 1u:
    f( x );
    break;
  default:
    x[n.var()] = false; visit_paths_rec( n.low(), x, f );
    std::fill( x.begin() + n.var(), x.end(), dontcare );
    x[n.var()] = true;  visit_paths_rec( n.high(), x, f );
  }
}

char to_char( const tribool& t )
{
  return indeterminate( t ) ? '-' : ( static_cast<bool>( t ) ? '1' : '0' );
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void visit_solutions( const bdd& n, const visit_solutions_func& f )
{
  if ( n.index != 0u )
  {
    boost::dynamic_bitset<> x( n.manager->num_vars() );
    visit_solutions_rec( 0u, n, x, f );
  }
}

void visit_paths( const bdd& n, const visit_paths_func& f )
{
  if ( n.index != 0u )
  {
    std::vector<tribool> x( n.manager->num_vars(), dontcare );
    visit_paths_rec( n, x, f );
  }
}

void print_paths( const bdd& n, std::ostream& os )
{
  visit_paths( n, [&]( const std::vector<tribool>& cube ) {
      boost::copy( cube | transformed( to_char ), std::ostream_iterator<char>( os ) );
      os << " 1" << std::endl;
    } );
}

void print_paths( const std::vector<bdd>& ns, std::ostream& os )
{
  std::map<std::vector<tribool>, boost::dynamic_bitset<>> pla;
  auto idx = 0u;

  auto visitor = [&]( const std::vector<tribool>& cube ) {
    auto it = pla.find( cube );
    if ( it == pla.end() )
    {
      pla.insert( {cube, onehot_bitset( ns.size(), idx )} );
    }
    else
    {
      it->second.set( idx );
    }
  };

  for ( idx = 0u; idx < ns.size(); ++idx )
  {
    visit_paths( ns[idx], visitor );
  }

  /* now print */
  for ( const auto& line : pla )
  {
    std::string bs;
    to_string( line.second, bs );
    boost::reverse( bs );

    boost::copy( line.first | transformed( to_char ), std::ostream_iterator<char>( os ) );
    os << " " << bs << std::endl;
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
