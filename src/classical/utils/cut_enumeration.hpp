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
 * @file cut_enumeration.hpp
 *
 * @brief Enumerates cuts
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef CUT_ENUMERATION_HPP
#define CUT_ENUMERATION_HPP

#include <map>
#include <stack>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/range/iterator_range.hpp>
#include <core/properties.hpp>
#include <core/utils/bitset_utils.hpp>
#include <core/utils/timer.hpp>
#include <classical/utils/cut_enumeration_traits.hpp>
#include <classical/utils/truth_table_utils.hpp>

using namespace boost::assign;

namespace cirkit
{

/******************************************************************************
 * Cut implementations                                                        *
 ******************************************************************************/

template<typename CutType>
CutType cut_from_index( unsigned index, unsigned n )
{
  assert( false );
}

template<typename CutType>
CutType empty_cut( unsigned n )
{
  assert( false );
}

template<typename CutType>
unsigned cut_size( const CutType& cut )
{
  assert( false );
}

template<typename CutType>
CutType cut_union( const CutType& cut1, const CutType& cut2 )
{
  assert( false );
}

template<typename CutType>
CutType& cut_insert( CutType& cut, unsigned index )
{
  assert( false );
}

template<typename CutType>
void foreach_node_in_cut( const CutType& cut, const std::function<void(unsigned)>&& f )
{
  assert( false );
}

template<typename CutType>
std::ostream& print_cut( std::ostream& os, const CutType& cut )
{
  assert( false );
}

template<typename CutsType>
CutsType cuts_from_index( unsigned index, unsigned n )
{
  return {cut_from_index<typename CutsType::value_type>( index, n )};
}

template<typename CutsType>
CutsType empty_cuts( unsigned n )
{
  return {empty_cut<typename CutsType::value_type>( n )};
}

/* structural_cut */
using structural_cut = std::vector<boost::dynamic_bitset<>>;

template<>
boost::dynamic_bitset<> cut_from_index( unsigned index, unsigned n );

template<>
boost::dynamic_bitset<> empty_cut( unsigned n );

template<>
unsigned cut_size<>( const boost::dynamic_bitset<>& cut );

template<>
boost::dynamic_bitset<> cut_union<>( const boost::dynamic_bitset<>& cut1, const boost::dynamic_bitset<>& cut2 );

template<>
boost::dynamic_bitset<>& cut_insert<>( boost::dynamic_bitset<>& cut, unsigned index );

template<>
void foreach_node_in_cut<>( const boost::dynamic_bitset<>& cut, const std::function<void(unsigned)>&& f );

template<>
std::ostream& print_cut<>( std::ostream& os, const boost::dynamic_bitset<>& cut );

/* std::set */
template<>
std::set<unsigned> cut_from_index( unsigned index, unsigned n );

template<>
std::set<unsigned> empty_cut( unsigned n );

template<>
unsigned cut_size<>( const std::set<unsigned>& cut );

template<>
std::set<unsigned> cut_union<>( const std::set<unsigned>& cut1, const std::set<unsigned>& cut2 );

template<>
std::set<unsigned>& cut_insert<>( std::set<unsigned>& cut, unsigned index );

template<>
void foreach_node_in_cut<>( const std::set<unsigned>& cut, const std::function<void(unsigned)>&& f );

template<>
std::ostream& print_cut<>( std::ostream& os, const std::set<unsigned>& cut );

/* std::vector */
template<>
std::vector<unsigned> cut_from_index( unsigned index, unsigned n );

template<>
std::vector<unsigned> empty_cut( unsigned n );

template<>
unsigned cut_size<>( const std::vector<unsigned>& cut );

template<>
std::vector<unsigned> cut_union<>( const std::vector<unsigned>& cut1, const std::vector<unsigned>& cut2 );

template<>
std::vector<unsigned>& cut_insert<>( std::vector<unsigned>& cut, unsigned index );

template<>
void foreach_node_in_cut<>( const std::vector<unsigned>& cut, const std::function<void(unsigned)>&& f );

template<>
std::ostream& print_cut<>( std::ostream& os, const std::vector<unsigned>& cut );

namespace detail
{

/******************************************************************************
 * Simulation algorithm for cut enumeration                                   *
 ******************************************************************************/

template<typename T, typename CutsType>
class structural_cut_enumeration_simulator : public cut_enumeration_circuit_traits<T>::template simulator<CutsType>
{
public:
  using node_t = typename circuit_traits<T>::node;
  using cut_t  = typename CutsType::value_type;

public:
  structural_cut_enumeration_simulator( const T& circ, unsigned k, bool include_constant, const boost::dynamic_bitset<>& cut_boundary )
    : circ( circ ),
      n( boost::num_vertices( circ ) ),
      include_constant( include_constant ),
      k( k ),
      cut_boundary( cut_boundary )
  {
  }

  structural_cut_enumeration_simulator( const T& circ, unsigned k, bool include_constant )
    : structural_cut_enumeration_simulator( circ, k, include_constant, boost::dynamic_bitset<>( boost::num_vertices( circ ) ) )
  {
  }

  CutsType get_input( const node_t& node, const std::string& name, unsigned pos, const T& circ ) const
  {
    return cuts_from_index<CutsType>( node, n );
  }

  CutsType get_constant() const
  {
    if ( include_constant )
    {
      return cuts_from_index<CutsType>( boost::get_property( circ, boost::graph_name ).constant, n );
    }
    else
    {
      return empty_cuts<CutsType>( n );
    }
  }

  CutsType invert( const CutsType& v ) const { return v; }

  CutsType and_op( const node_t& node, const CutsType& v1, const CutsType& v2 ) const
  {
    CutsType cuts;

    if ( !cut_boundary.test( node ) )
    {
      for ( const auto& c1 : v1 )
      {
        for ( const auto& c2 : v2 )
        {
          auto new_cut = cut_union( c1, c2 );
          if ( cut_size( new_cut ) > k ) continue;
          cuts += new_cut;
        }
      }
    }

    cuts += cut_from_index<cut_t>( node, n );

    return cuts;
  }

  CutsType maj_op( const node_t& node, const CutsType& v1, const CutsType& v2, const CutsType& v3 ) const
  {
    CutsType cuts;

    if ( !cut_boundary.test( node ) )
    {
      for ( const auto& c1 : v1 )
      {
        for ( const auto& c2 : v2 )
        {
          for ( const auto& c3 : v3 )
          {
            auto new_cut = cut_union( c1, cut_union( c2, c3 ) );
            if ( cut_size( new_cut ) > k ) continue;
            cuts += new_cut;
          }
        }
      }
    }

    cuts += cut_from_index<cut_t>( node, n );

    return cuts;
  }

  bool terminate( const node_t& node, const T& circ ) const
  {
    return cut_boundary.test( node );
  }

private:
  const T&                circ;
  unsigned                n;
  bool                    include_constant;
  unsigned                k;
  boost::dynamic_bitset<> cut_boundary;
};

template<typename T, typename CutsType>
class structural_cut_cone_simulator : public cut_enumeration_circuit_traits<T>::template simulator<typename CutsType::value_type>
{
public:
  using node_t = typename circuit_traits<T>::node;
  using cut_t  = typename CutsType::value_type;

public:
  structural_cut_cone_simulator( unsigned n ) : n( n ) {}

  cut_t get_input( const node_t& node, const std::string& name, unsigned pos, const T& circ ) const
  {
    return cut_from_index<cut_t>( node, n );
  }

  cut_t get_constant() const
  {
    return cut_from_index<cut_t>( 0u, n );
  }

  cut_t invert( const cut_t& v ) const { return v; }

  cut_t and_op( const node_t& node, const cut_t& v1, const cut_t& v2 ) const
  {
    auto cut = cut_union( v1, v2 );
    return cut_insert( cut, node );
  }

  cut_t maj_op( const node_t& node, const cut_t& v1, const cut_t& v2, const cut_t& v3 ) const
  {
    auto cut = cut_union( v1, cut_union( v2, v3 ) );
    return cut_insert( cut, node );
  }

private:
  unsigned n;
};

template<typename T>
class structural_cut_depth_simulator : public cut_enumeration_circuit_traits<T>::template simulator<unsigned>
{
public:
  using node_t = typename circuit_traits<T>::node;

public:
  unsigned get_input( const node_t& node, const std::string& name, unsigned pos, const T& circ ) const
  {
    return 0u;
  }

  unsigned get_constant() const
  {
    return 0u;
  }

  unsigned invert( const unsigned& v ) const { return v; }

  unsigned and_op( const node_t& node, const unsigned& v1, const unsigned& v2 ) const
  {
    return std::max( v1, v2 ) + 1u;
  }

  unsigned maj_op( const node_t& node, const unsigned& v1, const unsigned& v2, const unsigned& v3 ) const
  {
    return std::max( v1, std::max( v2, v3 ) ) + 1u;
  }
};

template<typename T>
inline void copy_outputs_from_info( const T& circ, std::vector<typename circuit_traits<T>::node>& nodes )
{
  for ( const auto& o : boost::get_property( circ, boost::graph_name ).outputs )
  {
    nodes += o.first.node;
  }
}

}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

template<typename T, typename CutsType = structural_cut>
std::map<typename circuit_traits<T>::node, CutsType> structural_cut_enumeration( const T& circ, unsigned k,
                                                                                const properties::ptr& settings = properties::ptr(),
                                                                                const properties::ptr& statistics = properties::ptr() )
{
  using node_t = typename circuit_traits<T>::node;

  /* settings */
  const auto boundary         = get( settings, "boundary",         boost::dynamic_bitset<>( boost::num_vertices( circ ) ) );
        auto start_nodes      = get( settings, "start_nodes",      std::vector<node_t>() );
  const auto include_constant = get( settings, "include_constant", false );

  /* timer */
  properties_timer t( statistics );

  std::map<node_t, CutsType> cut_map;

  /* nodes */
  if ( start_nodes.empty() )
  {
    detail::copy_outputs_from_info( circ, start_nodes );
  }

  /* simulation */
  typename circuit_traits<T>::node_color_map colors;
  detail::structural_cut_enumeration_simulator<T, CutsType> simulator( circ, k, include_constant, boundary );

  for ( const auto& node : start_nodes )
  {
    detail::cut_enumeration_circuit_traits<T>::simulate_node( circ, node, simulator, colors, cut_map );
  }

  return cut_map;
}

template<typename T, typename CutsType = structural_cut>
tt simulate_cut_tt( const typename circuit_traits<T>::node& node,
                    const typename CutsType::value_type& cut,
                    const T& circ,
                    const properties::ptr& settings = properties::ptr(),
                    const properties::ptr& statistics = properties::ptr() )
{
  /* timer */
  properties_timer t( statistics );

  /* simulation */
  std::map<typename circuit_traits<T>::node, tt> assignment;
  auto i = 0u;

  foreach_node_in_cut( cut, [&]( unsigned node ) {
      if ( node == 0u ) { return; }

      assignment.insert( {node, tt_nth_var( i++ )} );
    } );

  typename detail::cut_enumeration_circuit_traits<T>::tt_simulator tt_sim;
  typename detail::cut_enumeration_circuit_traits<T>::template partial_node_assignment_simulator<tt> sim( tt_sim, assignment, tt_const0() );

  return detail::cut_enumeration_circuit_traits<T>::template simulate_node<tt>( circ, node, sim );
}

template<typename T, typename CutsType = structural_cut>
typename CutsType::value_type cut_cone( const typename circuit_traits<T>::node& node, const typename CutsType::value_type& cut, const T& circ )
{
  using cut_t = typename CutsType::value_type;

  std::map<typename circuit_traits<T>::node, cut_t> assignment;
  const auto n = boost::num_vertices( circ );

  foreach_node_in_cut( cut, [&]( unsigned node ) {
      assignment.insert( {node, cut_from_index<cut_t>( node, n )} );
    } );

  detail::structural_cut_cone_simulator<T, CutsType> count_sim( n );
  typename detail::cut_enumeration_circuit_traits<T>::template partial_node_assignment_simulator<cut_t> sim( count_sim, assignment, empty_cut<cut_t>( n ) );

  return detail::cut_enumeration_circuit_traits<T>::template simulate_node<cut_t>( circ, node, sim );
}

template<typename T, typename CutsType = structural_cut>
unsigned cut_cone_size( const typename circuit_traits<T>::node& node, const typename CutsType::value_type& cut, const T& circ )
{
  return cut_size( cut_cone<T, CutsType>( node, cut, circ ) );
}

template<typename T, typename CutsType = structural_cut>
unsigned cut_cone_depth( const typename circuit_traits<T>::node& node, const typename CutsType::value_type& cut, const T& circ )
{
  std::map<typename circuit_traits<T>::node, unsigned> assignment;

  foreach_node_in_cut( cut, [&]( unsigned node ) {
      assignment.insert( {node, 0u} );
    } );

  detail::structural_cut_depth_simulator<T> depth_sim;
  typename detail::cut_enumeration_circuit_traits<T>::template partial_node_assignment_simulator<unsigned> sim( depth_sim, assignment, 0u );

  return detail::cut_enumeration_circuit_traits<T>::template simulate_node<unsigned>( circ, node, sim );
}

// template<typename T, typename CutType = structural_cut>
// std::map<typename circuit_traits<T>::node, unsigned> cut_cone_children_depth( const typename circuit_traits<T>::node& node, const typename CutType::value_type& cut, const T& circ )
// {
//   std::map<typename circuit_traits<T>::node, unsigned> depths;
//   std::stack<std::pair<typename circuit_traits<T>::node, unsigned>> stack;
//   stack.push( {node, 0u} );

//   while( !stack.empty() )
//   {
//     auto top = stack.top(); stack.pop();

//     if ( cut[top.first] )
//     {
//       depths.insert( {top.first, top.second} );
//     }
//     else
//     {
//       for ( const auto& child : boost::make_iterator_range( boost::adjacent_vertices( top.first, circ ) ) )
//       {
//         stack.push( {child, top.second + 1u} );
//       }
//     }
//   }
// }

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
