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

#include "lad2.hpp"

#include <algorithm>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <stack>

#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/counting_range.hpp>
#include <boost/timer/timer.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <core/utils/timer.hpp>

#include <classical/aig.hpp>
#include <classical/functions/aig_cone.hpp>
#include <classical/functions/aig_support.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/functions/simulation_graph.hpp>
#include <classical/utils/aig_utils.hpp>

using namespace boost::assign;
using boost::format;
using boost::adaptors::transformed;

namespace cirkit
{

/******************************************************************************
 * Macros                                                                     *
 ******************************************************************************/

/**
 * If LAD_TEX_LOGGER is defined, the algorithm creates a LaTeX document that
 * contains several internal statistics.
 */
//#define LAD_TEX_LOGGER

#ifdef LAD_TEX_LOGGER
#define TL(x) x
#else
#define TL(x)
#endif

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

using vec_int_t         = std::vector<int>;
using stack_int_t       = std::stack<int>;
using vec_vec_int_t     = std::vector<vec_int_t>;
using bitset_pair_vec_t = std::vector<std::pair<boost::dynamic_bitset<>, boost::dynamic_bitset<>>>;
using bitset_vec_t      = std::vector<boost::dynamic_bitset<>>;

/******************************************************************************
 * Graph                                                                      *
 ******************************************************************************/

// std::ostream& operator<<( std::ostream& os, const simulation_graph_wrapper& g )
// {
//   os << format( "Directed labelled graph with %d vertices" ) % g.size() << std::endl;

//   for ( const auto& v : g.vertices() )
//   {
//     os << format( "Vertex %d has label %d and %d adjacent vertices (%d pred and %d succ): ") % v % g.label( v )
//       % g.degree( v ) % g.in_degree( v ) % g.out_degree( v );
//     for ( const auto& v2 : g.adjacent( v ) )
//     {
//       os << format( " %d(" ) % v2;
//       switch ( g.edge_direction( v, v2 ) )
//       {
//       case 1: os << "succ"; break;
//       case 2: os << "pred"; break;
//       case 3: os << "succ and pred"; break;
//       }
//       os << "; label " << g.edge_label( v, v2 ) << ")";
//     }
//     os << std::endl;
//   }
//   return os;
// }

/******************************************************************************
 * Domains                                                                    *
 ******************************************************************************/
inline bool compatible_vertex_labels( int l1, int l2 )
{
  return l1 == l2;
}

inline bool compatible_edge_labels( int l1, int l2 )
{
  return l1 == l2;
}

inline bool is_compatible( int dir_gp, int dir_gt )
{
  /* In our application dir_gt is never 3 */
  // return dir_gp == dir_gt || dir_gt == 3;
  return dir_gp == dir_gt;
}

bool compatible_vertices( int u, int v,
                          const simulation_graph_wrapper& gp, const simulation_graph_wrapper& gt,
                          const boost::optional<unsigned>& simulation_signatures,
                          bool functional_support_constraints )
{
  if ( !compatible_vertex_labels( gp.label( u ), gt.label( v ) ) )
  {
    return false;
  }

  // INDUCED
  if ( gp.in_degree( u ) > gt.in_degree( v ) ) return false;
  if ( gp.out_degree( u ) > gt.out_degree( v ) ) return false;
  //if ( gp.adj[u].size() - gp.nb_pred[u] - gp.nb_succ[u] > gt.adj[v].size() - gt.nb_pred[v] - gt.nb_succ[v] ) return false;
  if ( functional_support_constraints && ( gt.support( v ).count() != gp.support( u ).count() ) ) return false;
  if ( !functional_support_constraints && ( gt.support( v ).count() < gp.support( u ).count() ) ) return false;

  if ( (bool)simulation_signatures )
  {
    if ( !compatible_simulation_signatures( gp, gt, u, v, *simulation_signatures ) )
    {
      return false;
    }
  }

  return true;
}

lad2_domain::lad2_domain( const simulation_graph_wrapper& gp, const simulation_graph_wrapper& gt, const boost::optional<unsigned>& simulation_signatures, bool functional_support_constraints )
  : matching( gp.size(), gt.size() )
{
  /* create */
  global_matching_p.resize( gp.size(), -1 );
  global_matching_t.resize( gt.size(), -1 );
  nb_val.resize( gp.size(), 0 );
  first_val.resize( gp.size() );
  pos_in_val.resize( gp.size(), vec_int_t( gt.size() ) );
  marked_to_filter.resize( gp.size(), true );
  to_filter.resize( gp.size() );

  /* initialize */
  auto val_size = 0;
  unsigned v;
  for ( unsigned u = 0u; u < gp.size(); ++u )
  {
    to_filter[u] = u;
    first_val[u] = val_size;
    for ( v = 0u; v < gt.size(); ++v )
    {
      if ( !compatible_vertices( u, v, gp, gt, simulation_signatures, functional_support_constraints ) ) /* v not in D[u] */
      {
        pos_in_val[u][v] = first_val[u] + gt.size();
      }
      else /* v in D[u] */
      {
        matching.reserve( u, v, gp.degree( u ) );
        val += v;
        nb_val[u]++;
        pos_in_val[u][v] = val_size++;
      }
    }
  }

  matching.resize();
  next_out_to_filter = 0;
  last_in_to_filter = gp.size() - 1;
}

bool lad2_domain::augmenting_path( int u, int nbv )
{
  std::deque<int> fifo;
  vec_int_t pred( nbv );
  int i, v, v2, u2;
  boost::dynamic_bitset<> marked( nbv );
  for ( const auto& v : get( u ) )
  {
    if ( global_matching_t[v] < 0 ) /* v is free => augmenting path found */
    {
      global_matching_p[u] = v;
      global_matching_t[v] = u;
      return true;
    }

    /* v is not free => add to fifo */
    pred[v] = u;
    fifo.push_back( v );
    marked.set( v );
  }

  while ( !fifo.empty() )
  {
    u2 = global_matching_t[fifo.front()];
    fifo.pop_front();

    for ( i = 0u; i < nb_val[u2]; ++i )
    {
      v = val[first_val[u2] + i]; /* v in D(u2) */
      if ( global_matching_t[v] < 0 ) /* v is free => augmenting path found */
      {
        while ( u2 != u )
        {
          v2 = global_matching_p[u2];
          global_matching_p[u2] = v;
          global_matching_t[v] = u2;
          v = v2;
          u2 = pred[v];
        }
        global_matching_p[u] = v;
        global_matching_t[v] = u;
        return true;
      }
      if ( !marked.test( v ) ) /* v is not free and not marked => add it to fifo */
      {
        pred[v] = u2;
        fifo.push_back( v );
        marked.set( v );
      }
    }
  }

  return false;
}

std::ostream& operator<<( std::ostream& os, const lad2_domain& d )
{
  unsigned u;
  int i;
  for ( u = 0u; u < d.nb_val.size(); ++u )
  {
    os << format( "D[%d] = " ) % u;
    for ( i = 0; i < d.nb_val[u]; ++i )
    {
      os << format( "%d " ) % d.val[d.first_val[u] + i];
    }
    os << std::endl;
  }
  return os;
}

void lad2_domain::dump( const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  os << "nbVal: " << any_join( nb_val, " " ) << std::endl
     << "firstVal: " << any_join( first_val, " " ) << std::endl
     << "val: " << any_join( boost::make_iterator_range( val.begin(), val.begin() + first_val.back() + nb_val.back() ), " " ) << std::endl;
  for ( const auto& p : pos_in_val )
  {
    os << "posInVal[]: " << any_join( p, " " ) << std::endl;
  }
  // for ( const auto& p : first_match )
  // {
  //   os << "firstMatch[]: " << any_join( p, " " ) << std::endl;
  // }
  os /*<< "matching: " << any_join( matching, " " ) << std::endl*/
     << "nextOutToFilter: " << next_out_to_filter << std::endl
     << "lastInToFilter: " << last_in_to_filter << std::endl
     << "toFilter: " << any_join( to_filter, " " ) << std::endl;
  os << "markedToFilter:";
  for ( unsigned i = 0; i < marked_to_filter.size(); ++i )
  {
    os << " " << marked_to_filter.test( i );
  }
  os << std::endl
     << "globalMatchingP: " << any_join( global_matching_p, " " ) << std::endl
     << "globalMatchingT: " << any_join( global_matching_t, " " ) << std::endl;

  os.close();
}

/******************************************************************************
 * Manager                                                                    *
 ******************************************************************************/

struct lad2_manager
{
  lad2_manager( const aig_graph& target, const aig_graph& pattern,
                const std::vector<unsigned>& types,
                bool functional, bool support_edges, const boost::optional<unsigned>& simulation_signatures,
                const std::string& texlogname, bool verbose )
    : gp( pattern, types, support_edges, simulation_signatures ),
      gt( target, types, support_edges, simulation_signatures ),
      d( gp, gt, simulation_signatures, functional ),
      verbose( verbose ),
      num( gt.size() ),
      num_inv( gt.size() ),
      nb_comp( gp.size() ),
      first_comp( gp.size() ),
      matched_with_u( gp.size() )
  {
    if ( verbose )
    {
      std::cout << format( "[i] target graph has %d vertices" ) % gt.size() << std::endl
                << format( "[i] pattern graph has %d vertices" ) % gp.size() << std::endl;
    }

#ifdef LAD_TEX_LOGGER
    tl.open( texlogname.c_str(), std::ofstream::out );
    tl << "\\documentclass[9pt]{scrartcl}" << std::endl
       << "\\usepackage[utf8]{inputenc}" << std::endl
       << "\\usepackage[T1]{fontenc}" << std::endl << std::endl
       << "\\usepackage[left=1cm,right=1cm,top=1cm,bottom=1cm]{geometry}" << std::endl
       << "\\usepackage{amsmath}" << std::endl
       << "\\usepackage{amssymb}" << std::endl
       << "\\usepackage{pgfplots}" << std::endl
       << "\\allowdisplaybreaks[1]" << std::endl;

    tl << "\\begin{document}" << std::endl;
    tl << "\\subsubsection*{Input sizes}" << std::endl;

    tl << format( "\\[ \\begin{aligned} \\#X_T &= %d \\\\ \\#S_T &= %d \\\\ \\#Y_T &= %d \\end{aligned}" ) % gt.num_inputs() % gt.num_vectors() % gt.num_outputs()
       << std::endl << "   \\qquad" << std::endl
       << format( "   \\begin{aligned} \\#X_P &= %d \\\\ \\#S_P &= %d \\\\ \\#Y_P &= %d \\end{aligned} \\]" ) % gp.num_inputs() % gp.num_vectors() % gp.num_outputs()
       << std::endl;

    tl << "\\subsubsection*{Initial domain}" << std::endl;
    list_with_names( tl );
    list_target_image( tl );
#endif
  }

  ~lad2_manager()
  {
#ifdef LAD_TEX_LOGGER
    /* domain size graph */
    const double sec = 1000000000.0L;

    std::vector<double>      branch_points;
    std::vector<std::string> branch_labels;
    for ( const auto& t : branch_at_time )
    {
      branch_points += std::get<0>( t ) / sec;
      branch_labels += std::string();
    }

    tl << "\\subsubsection*{Domain size graph}" << std::endl;
    tl << "\\begin{tikzpicture}" << std::endl
       << format( "  \\begin{axis}[x tick label style={/pgf/number format/fixed},extra x ticks={%s},extra x tick labels={%s},extra x tick style={grid=major,tick label style={rotate=90,font=\\footnotesize,anchor=east}},legend entries={$D_X$,$D_S$,$D_Y$}]" )
          % any_join( branch_points, "," )
          % boost::join( branch_labels, "," ) << std::endl;
    tl << "    \\addplot coordinates {";
    for ( auto i = 0u; i < size_at_time.size(); ++i )
    {
      if ( i == 0u || i == size_at_time.size() - 1u ||
           std::get<1>( size_at_time[i] ) != std::get<1>( size_at_time[i + 1] ) ||
           std::get<1>( size_at_time[i] ) != std::get<1>( size_at_time[i - 1] ) )
      {
        tl << format( " (%.2f, %d)" ) % ( std::get<0>( size_at_time[i] ) / sec ) % std::get<1>( size_at_time[i] );
      }
    }
    tl << " };" << std::endl;
    tl << "    \\addplot coordinates {";
    for ( auto i = 0u; i < size_at_time.size(); ++i )
    {
      if ( i == 0u || i == size_at_time.size() - 1u ||
           std::get<2>( size_at_time[i] ) != std::get<2>( size_at_time[i + 1] ) ||
           std::get<2>( size_at_time[i] ) != std::get<2>( size_at_time[i - 1] ) )
      {
        tl << format( " (%.2f, %d)" ) % ( std::get<0>( size_at_time[i] ) / sec ) % std::get<2>( size_at_time[i] );
      }
    }
    tl << " };" << std::endl;
    tl << "    \\addplot coordinates {";
    for ( auto i = 0u; i < size_at_time.size(); ++i )
    {
      if ( i == 0u || i == size_at_time.size() - 1u ||
           std::get<3>( size_at_time[i] ) != std::get<3>( size_at_time[i + 1] ) ||
           std::get<3>( size_at_time[i] ) != std::get<3>( size_at_time[i - 1] ) )
      {
        tl << format( " (%.2f, %d)" ) % ( std::get<0>( size_at_time[i] ) / sec ) % std::get<3>( size_at_time[i] );
      }
    }
    tl << " };" << std::endl;
    tl << "  \\end{axis}" << std::endl
       << "\\end{tikzpicture}" << std::endl << std::endl;

    tl << "\\end{document}" << std::endl;
    tl.close();
#endif
  }

  inline bool are_compatible_edges( int u, int u2, int v, int v2 )
  {
    return ( compatible_edge_labels( gp.edge_label( u, u2 ), gt.edge_label( v, v2 ) ) &&
             is_compatible( gp.edge_direction( u, u2 ), gt.edge_direction( v, v2 ) ) );
  }

  bool remove_all_values_but_one( int u, int v );
  bool remove_value( int u, int v );
  bool match_vertices( stack_int_t& to_be_matched );
  inline bool match_vertex( int u )
  {
    stack_int_t to_be_matched;
    to_be_matched.push( u );
    return match_vertices( to_be_matched );
  }
  bool ensure_gac_all_diff();
  bool check_lad( int u, int v );
  bool filter();
  bool solve( unsigned& nb_sol, std::vector<unsigned>& mapping );
  bool start_lad( std::vector<unsigned>& mapping );

  void list_target_image( std::ostream& os );
  std::tuple<unsigned, unsigned, unsigned> target_image_size();
  void list_with_names( std::ostream& os, bool only_inputs = true );

  simulation_graph_wrapper gp;
  simulation_graph_wrapper gt;
  lad2_domain d;
  bool verbose;

  /* for check_lad */
  vec_int_t num;
  vec_int_t num_inv;
  vec_int_t nb_comp;
  vec_int_t first_comp;
  vec_int_t comp;
  vec_int_t matched_with_u;

  /* special settings */
  domain_hook_t on_before_first_branch;
  domain_hook_t on_filter;

  /* statistics */
  boost::timer::cpu_timer timer;
#ifdef LAD_TEX_LOGGER /* because of comma in type */
  std::vector<std::tuple<boost::timer::nanosecond_type, unsigned, unsigned, unsigned>> size_at_time;
  std::vector<std::tuple<boost::timer::nanosecond_type, unsigned, unsigned>> branch_at_time;
#endif
  unsigned num_branches = 0u;

  /* TeX logger */
  TL( std::ofstream tl; );
};

/******************************************************************************
 * All Different Calculation                                                  *
 ******************************************************************************/
enum marker_t
{
  white, gray, black, to_be_deleted, deleted
};

inline void add_to_delete( int u, vec_int_t& list, int* nb, vec_int_t& marked )
{
  if ( marked[u] < to_be_deleted )
  {
    list[(*nb)++] = u;
    marked[u] = to_be_deleted;
  }
}

inline void erase_and_replace( vec_int_t& v, unsigned i )
{
  v[i] = v.back();
  v.pop_back();
}

bool update_matching( int size_of_u, int size_of_v, const vec_int_t& degree, const vec_int_t& first_adj, const vec_int_t& adj, vec_int_t& matched_with_u )
{
  if ( size_of_u > size_of_v ) return false;

  static vec_int_t matched_with_v, nb_pred, nb_succ, list_v, list_u, list_dv, list_du, marked, marked_v, marked_u, unmatched, pos_in_unmatched;
  static vec_vec_int_t pred, succ;

  matched_with_v.resize( size_of_v );
  boost::fill( matched_with_v, -1 );
  nb_pred.resize( size_of_v );
  pred.clear();
  pred.resize( size_of_v, vec_int_t( size_of_u ) );
  nb_succ.resize( size_of_u );
  succ.clear();
  succ.resize( size_of_u, vec_int_t( size_of_v ) );
  list_v.resize( size_of_v );
  list_u.resize( size_of_u );
  list_dv.resize( size_of_v );
  list_du.resize( size_of_u );

  int nbv, nbu, nbdv, nbdu;
  int i, j, k, u, v;
  bool stop;

  marked_v.resize( size_of_v );
  marked_u.resize( size_of_u );
  unmatched.clear();
  pos_in_unmatched.resize( size_of_u );

  for ( u = 0; u < size_of_u; ++u )
  {
    if ( matched_with_u[u] >= 0 )
    {
      matched_with_v[matched_with_u[u]] = u;
    }
    else
    {
      pos_in_unmatched[u] = unmatched.size();
      unmatched += u;
    }
  }

  j = 0;
  while ( j < static_cast<int>( unmatched.size() ) )
  {
    u = unmatched[j];
    for ( i = first_adj[u]; ( ( i < first_adj[u] + degree[u] ) && ( matched_with_v[adj[i]] >= 0 ) ); ++i );
    if ( i == first_adj[u] + degree[u] )
    {
      ++j;
    }
    else
    {
      v = adj[i];
      matched_with_u[u] = v;
      matched_with_v[v] = u;
      erase_and_replace( unmatched, j );
      pos_in_unmatched[unmatched[j]] = j;
    }
  }

  while ( !unmatched.empty() )
  {
    boost::fill( marked_u, white );
    boost::fill( nb_succ, 0 );
    boost::fill( marked_v, white );
    boost::fill( nb_pred, 0 );

    nbv = 0;
    for ( unsigned u : unmatched )
    {
      marked_u[u] = black;
      for ( i = first_adj[u]; i < first_adj[u] + degree[u]; ++i )
      {
        v = adj[i];
        pred[v][nb_pred[v]++] = u;
        succ[u][nb_succ[u]++] = v;
        if ( marked_v[v] == white )
        {
          marked_v[v] = gray;
          list_v[nbv++] = v;
        }
      }
    }
    stop = false;
    while ( !stop && nbv > 0 )
    {
      nbu = 0;
      for ( i = 0; i < nbv; ++i )
      {
        v = list_v[i];
        marked_v[v] = black;
        u = matched_with_v[v];
        if ( marked_u[u] == white )
        {
          marked_u[u] = gray;
          list_u[nbu++] = u;
        }
      }
      nbv = 0;
      for ( j = 0; j < nbu; ++j )
      {
        u = list_u[j];
        marked_u[u] = black;
        for ( i = first_adj[u]; i < first_adj[u] + degree[u]; ++i )
        {
          v = adj[i];
          if ( marked_v[v] != black )
          {
            pred[v][nb_pred[v]++] = u;
            succ[u][nb_succ[u]++] = v;
            if ( marked_v[v] == white )
            {
              marked_v[v] = gray;
              list_v[nbv++] = v;
            }
            if ( matched_with_v[v] == -1 )
            {
              stop = true;
            }
          }
        }
      }
    }

    if ( nbv == 0 ) { return false; }

    for ( k = 0; k < nbv; ++k )
    {
      v = list_v[k];
      if ( matched_with_v[v] == -1 && nb_pred[v] > 0 )
      {
        stack_int_t path;
        path.push( v );
        nbdv = nbdu = 0;
        add_to_delete( v, list_dv, &nbdv, marked_v );
        do
        {
          u = pred[v][0];
          path.push( u );
          add_to_delete( u, list_du, &nbdu, marked_u );
          if ( matched_with_u[u] != -1 )
          {
            v = matched_with_u[u];
            path.push( v );
            add_to_delete( v, list_dv, &nbdv, marked_v );
          }
        } while ( matched_with_u[u] != -1 );

        while ( nbdv > 0 || nbdu > 0 )
        {
          while ( nbdv > 0 )
          {
            v = list_dv[--nbdv];
            marked_v[v] = deleted;
            u = matched_with_v[v];
            if ( u != -1 )
            {
              add_to_delete( u, list_du, &nbdu, marked_u );
            }
            for ( i = 0; i < nb_pred[v]; ++i )
            {
              u = pred[v][i];
              j = 0; while ( j < nb_succ[u] && v != succ[u][j] ) { ++j; }
              succ[u][j] = succ[u][--nb_succ[u]];
              if ( nb_succ[u] == 0 )
              {
                add_to_delete( u, list_du, &nbdu, marked_u );
              }
            }
          }
          while ( nbdu > 0 )
          {
            u = list_du[--nbdu];
            marked_u[u] = deleted;
            v = matched_with_u[u];
            if ( v != -1 )
            {
              add_to_delete( v, list_dv, &nbdv, marked_v );
            }
            j = 0;
            for ( i = 0; i < nb_succ[u]; ++i )
            {
              v = succ[u][i];
              j = 0; while ( j < nb_pred[v] && u != pred[v][j] ) { ++j; }
              pred[v][j] = pred[v][--nb_pred[v]];
              if ( nb_pred[v] == 0 )
              {
                add_to_delete( v, list_dv, &nbdv, marked_v );
              }
            }
          }
        }

        u = path.top();
        i = pos_in_unmatched[u];
        erase_and_replace( unmatched, i );
        pos_in_unmatched[unmatched[i]] = i;
        while ( path.size() > 1 )
        {
          u = path.top(); path.pop();
          v = path.top(); path.pop();
          matched_with_u[u] = v;
          matched_with_v[v] = u;
        }
      }
    }
  }

  return true;
}

void lad2_dfs( int nbu, int nbv, int u, std::vector<char>& marked, const vec_int_t& nb_succ, const vec_vec_int_t& succ, const vec_int_t& matched_with_u, vec_int_t& order, int* nb )
{
  marked[u] = 1;
  int v = matched_with_u[u];
  for ( int i = 0; i < nb_succ[v]; ++i )
  {
    if ( !marked[succ[v][i]] )
    {
      lad2_dfs( nbu, nbv, succ[v][i], marked, nb_succ, succ, matched_with_u, order, nb );
    }
  }
  order[*nb] = u; (*nb)--;
}

void lad2_scc( int nbu, int nbv, vec_int_t& numv, vec_int_t& numu,
               const vec_int_t& nb_succ, const vec_vec_int_t& succ,
               const vec_int_t& nb_pred, const vec_vec_int_t& pred,
               const vec_int_t& matched_with_u, const vec_int_t& matched_with_v )
{
  vec_int_t order( nbu );
  std::vector<char> marked( nbu );
  vec_int_t fifo( nbv );
  int u, v, i, j, k, nb_scc, nb;

  nb = nbu - 1;
  for ( u = 0; u < nbu; ++u )
  {
    if ( !marked[u] )
    {
      lad2_dfs( nbu, nbv, u, marked, nb_succ, succ, matched_with_u, order, &nb );
    }
  }

  nb_scc = 0;
  boost::fill( numu, -1 );
  boost::fill( numv, -1 );
  for ( i = 0; i < nbu; ++i )
  {
    u = order[i];
    v = matched_with_u[u];
    if ( numv[v] == -1 )
    {
      ++nb_scc;
      k = 1;
      fifo[0] = v;
      numv[v] = nb_scc;
      while ( k > 0 )
      {
        v = fifo[--k];
        u = matched_with_v[v];
        if ( u != -1 )
        {
          numu[u] = nb_scc;
          for ( j = 0; j < nb_pred[u]; ++j )
          {
            v = pred[u][j];
            if ( numv[v] == -1 )
            {
              numv[v] = nb_scc;
              fifo[k++] = v;
            }
          }
        }
      }
    }
  }
}

bool lad2_manager::remove_all_values_but_one( int u, int v )
{
  for ( const auto& j : gp.adjacent( u ) )
  {
    d.add_to_filter( j, gp.size() );
  }

  int old_pos = d.pos_in_val[u][v];
  int new_pos = d.first_val[u];
  d.val[old_pos] = d.val[new_pos];
  d.val[new_pos] = v;
  d.pos_in_val[u][d.val[new_pos]] = new_pos;
  d.pos_in_val[u][d.val[old_pos]] = old_pos;
  d.nb_val[u] = 1;
  if ( d.global_matching_p[u] != v )
  {
    d.global_matching_t[d.global_matching_p[u]] = -1;
    d.global_matching_p[u] = -1;
    return d.augmenting_path( u, gt.size() );
  }
  return true;
}

bool lad2_manager::remove_value( int u, int v )
{
  for ( const auto& j : gp.adjacent( u ) )
  {
    d.add_to_filter( j, gp.size() );
  }
  int old_pos = d.pos_in_val[u][v];
  d.nb_val[u]--;
  int new_pos = d.first_val[u] + d.nb_val[u];
  d.val[old_pos] = d.val[new_pos];
  d.val[new_pos] = v;
  d.pos_in_val[u][d.val[old_pos]] = old_pos;
  d.pos_in_val[u][d.val[new_pos]] = new_pos;
  if ( d.global_matching_p[u] == v )
  {
    d.global_matching_p[u] = -1;
    d.global_matching_t[v] = -1;
    return d.augmenting_path( u, gt.size() );
  }
  return true;
}

bool lad2_manager::match_vertices( stack_int_t& to_be_matched )
{
  int j, u, v, u2, old_nb_val;
  while ( !to_be_matched.empty() )
  {
    u = to_be_matched.top();
    to_be_matched.pop();
    v = d.val[d.first_val[u]];
    for ( u2 = 0; u2 < static_cast<int>( gp.size() ); ++u2 )
    {
      if ( u != u2 )
      {
        old_nb_val = d.nb_val[u2];
        if ( d.is_in_domain( u2, v ) && !remove_value( u2, v ) ) { return false; }
        if ( gp.edge_direction( u, u2 ) != 0 )
        {
          j = d.first_val[u2];
          while ( j < d.first_val[u2] + d.nb_val[u2] )
          {
            if ( compatible_edge_labels( gp.edge_label( u, u2 ), gt.edge_label( v, d.val[j] ) ) && is_compatible( gp.edge_direction( u, u2 ), gt.edge_direction( v, d.val[j] ) ) ) { ++j; }
            else if ( !remove_value( u2, d.val[j] ) ) { return false; }
          }
        }
        /* INDUCED */
        else // (u,u2) is not an edge => remove neighbors of v from D[u2]
        {
					j = d.first_val[u2];
					while ( j < d.first_val[u2] + d.nb_val[u2] )
          {
						if ( gt.edge_direction( v, d.val[j] ) == 0) { ++j; }
						else if ( !remove_value( u2, d.val[j] ) ) { return false; }
					}
				}

        if ( d.nb_val[u2] == 0 ) { return false; }
        if ( d.nb_val[u2] == 1 && old_nb_val > 1 )
        {
          to_be_matched.push( u2 );
        }
      }
    }
  }

  return true;
}

bool lad2_manager::ensure_gac_all_diff()
{
  vec_int_t nb_pred( gp.size() );
  vec_vec_int_t pred( gp.size(), vec_int_t( gt.size() ) );
  vec_int_t nb_succ( gt.size() );
  vec_vec_int_t succ( gt.size(), vec_int_t( gp.size() ) );
  int u, v, i, w, old_nb_val;
  vec_int_t numv( gt.size() ), numu( gp.size() );
  stack_int_t to_match;
  std::vector<std::vector<char>> used( gp.size(), std::vector<char>( gt.size() ) );
  for ( u = 0; u < static_cast<int>( gp.size() ); ++u )
  {
    for ( i = 0; i < d.nb_val[u]; ++i )
    {
      v = d.val[d.first_val[u] + i];
      used[u][v] = 0;
      if ( v != d.global_matching_p[u] )
      {
        pred[u][nb_pred[u]++] = v;
        succ[v][nb_succ[v]++] = u;
      }
    }
  }

  vec_int_t list( gt.size() );
  int nb = 0;
  for ( v = 0; v < static_cast<int>( gt.size() ); ++v )
  {
    if ( d.global_matching_t[v] < 0 )
    {
      list[nb++] = v;
      numv[v] = 1;
    }
  }
	while ( nb > 0 )
  {
		v = list[--nb];
		for ( i = 0; i < nb_succ[v]; ++i)
    {
			u = succ[v][i];
			used[u][v] = 1;
			if (numu[u] == 0)
      {
				numu[u] = 1;
				w = d.global_matching_p[u];
				used[u][w] = 1;
				if ( numv[w] == 0 )
        {
					list[nb++] = w;
					numv[w] = 1;
				}
			}
		}
	}

  lad2_scc( gp.size(), gt.size(), numv, numu,
            nb_succ, succ, nb_pred, pred, d.global_matching_p, d.global_matching_t );

  for ( u = 0; u < static_cast<int>( gp.size() ); ++u )
  {
    old_nb_val = d.nb_val[u];
		for ( i = 0; i < d.nb_val[u]; ++i)
    {
      v = d.val[d.first_val[u] + i];
      if ( !used[u][v] && numv[v] != numu[u] && d.global_matching_p[u] != v )
      {
        if ( !remove_value( u, v ) )
        {
          return false;
        }
      }
    }
    if ( d.nb_val[u] == 0 ) { return false; }
    if ( old_nb_val > 1 && d.nb_val[u] == 1 )
    {
      to_match.push( u );
    }
	}
	return match_vertices( to_match );
}

/******************************************************************************
 * LAD                                                                        *
 ******************************************************************************/
bool lad2_manager::check_lad( int u, int v )
{
  auto is_valid = [this]( int u2, int v2 ) { return v2 != -1 && this->d.is_in_domain( u2, v2 ); };

  /* Case where deg( u ) = 1 */
  if ( gp.degree( u ) == 1u )
  {
    auto u2 = *boost::adjacent_vertices( u, gp.sim_graph() ).first;
    auto v2 = d.matching( u, v );
    if ( is_valid( u2, v2 ) ) { return true; }

    for ( const auto& v2 : d.get( u2 ) )
    {
      if ( are_compatible_edges( u, u2, v, v2 ) )
      {
        d.matching( u, v ) = v2;
        return true;
      }
    }

    return false;
  }

  /* Case where deg( u ) > 1 */
  /*
   * It is first check whether all edges match. If that is the
   * case, true is returned.
   */
  auto nb_matched = 0u;
  auto idx = 0u;
  for ( const auto& u2 : gp.adjacent( u ) )
  {
    auto v2 = d.matching( u, v, idx );
    if ( is_valid( u2, v2 ) ) { ++nb_matched; }
    ++idx;
  }
  if ( nb_matched == gp.degree( u ) ) { return true; }

  comp.resize( gp.degree( u ) * gt.size() );

  boost::fill( num, -1 );

  int nb_num = 0;
  int pos_in_comp = 0;
  idx = 0u;
  for ( const auto& u2 : gp.adjacent( u ) )
  {
    nb_comp[idx] = 0;
    first_comp[idx] = pos_in_comp;
    if ( d.nb_val[u2] > static_cast<int>( gt.degree( v ) ) )
    {
      for ( const auto& v2 : d.get( u2 ) )
      {
        if ( are_compatible_edges( u, u2, v, v2 ) )
        {
          if ( num[v2] < 0 )
          {
            num[v2] = nb_num;
            num_inv[nb_num++] = v2;
          }
          comp[pos_in_comp++] = num[v2];
          nb_comp[idx]++;
        }
      }
    }
    else
    {
      for ( const auto& v2 : gt.adjacent( v ) )
      {
        if ( d.is_in_domain( u2, v2 ) && are_compatible_edges( u, u2, v, v2 ) )
        {
          if ( num[v2] < 0 )
          {
            num[v2] = nb_num;
            num_inv[nb_num++] = v2;
          }
          comp[pos_in_comp++] = num[v2];
          nb_comp[idx]++;
        }
      }
    }
    if ( nb_comp[idx] == 0 ) { return false; }
    auto v2 = d.matching( u, v, idx );
    if ( v2 != -1 && d.is_in_domain( u2, v2 ) )
    {
      matched_with_u[idx] = num[v2];
    }
    else
    {
      matched_with_u[idx] = -1;
    }
    ++idx;
  }

  if ( !update_matching( gp.degree( u ), nb_num, nb_comp, first_comp, comp, matched_with_u ) )
  {
    return false;
  }
  for ( auto i = 0u; i < gp.degree( u ); ++i )
  {
    d.matching( u, v, i ) = num_inv[matched_with_u[i]];
  }
  return true;
}

bool lad2_manager::filter()
{
  int u, v, i, old_nb_val;
  while ( !d.to_filter_empty() )
  {
    while ( !d.to_filter_empty() )
    {
      u = d.next_to_filter( gp.size() );
      old_nb_val = d.nb_val[u];
      i = d.first_val[u];
      while ( i < d.first_val[u] + d.nb_val[u] )
      {
        v = d.val[i];
        if ( check_lad( u, v ) ) { ++i; }
        else if ( !remove_value( u, v ) ) { return false; }
      }
      /* If D_u just became singleton; then try to match it */
      if ( d.nb_val[u] == 1 && old_nb_val > 1 && !match_vertex( u ) )
      {
        return false;
      }
      /* If D_u is empty */
      if ( d.nb_val[u] == 0 ) { return false; }

      TL( const auto image_size = target_image_size(); )
      TL( size_at_time += std::make_tuple( timer.elapsed().wall, std::get<0>( image_size ), std::get<1>( image_size ), std::get<2>( image_size ) ); );

      if ( (bool)on_filter )
      {
        /* We better ignore the return value of the hook for now */
        (*on_filter)( d );
      }
    }
    if ( !ensure_gac_all_diff() ) { return false; }
  }
  return true;
}

bool lad2_manager::solve( unsigned& nb_sol, std::vector<unsigned>& mapping )
{
  int v, min_dom, i;
  vec_int_t nb_val( gp.size() );
  vec_int_t global_matching( gp.size() );

  if ( !filter() )
  {
    if ( verbose )
    {
      std::cout << "[i] reset to filter" << std::endl;
    }
    d.reset_to_filter( gp.size() );
    return true;
  }

  /**
   * min_dom = argmin { #D_u | u ∈ V_P such that #D_u > 1 }
   *
   * otherwise, min_dom = -1
   */
  min_dom = -1;
  for ( const auto& u : gp.vertices() )
  {
    nb_val[u] = d.nb_val[u];
    if ( nb_val[u] > 1 && ( min_dom < 0 || nb_val[u] < nb_val[min_dom] ) )
    {
      min_dom = u;
    }
    global_matching[u] = d.global_matching_p[u];
  }

  if ( min_dom == -1 )
  {
    ++nb_sol;
    mapping.resize( gp.size() );
    if ( verbose )
    {
      std::cout << format( "Solution %d:" ) % nb_sol;
    }
    for ( const auto& u : gp.vertices() )
    {
      mapping[u] = d.val[d.first_val[u]];
      if ( verbose )
      {
        std::cout << format( " %d=%d" ) % u % d.val[d.first_val[u]];
      }
    }
    TL( tl << "\\subsubsection*{Final domain}" << std::endl; );
    TL( list_with_names( tl ); );
    d.reset_to_filter( gp.size() );
    return true;
  }

  vec_int_t val( d.nb_val[min_dom] );
  boost::copy( d.get( min_dom ), val.begin() );

  if ( num_branches == 0u )
  {
    TL( tl << "\\subsubsection*{Before first branch}" << std::endl; );
    TL( list_with_names( tl ); );
    TL( list_target_image( tl ); );

    if ( (bool)on_before_first_branch )
    {
      if ( (*on_before_first_branch)( d ) )
      {
        return false;
      }
    }
  }

  for ( i = 0; i < nb_val[min_dom] && nb_sol == 0; ++i )
  {
    v = val[i];
    num_branches++;
    if ( verbose )
    {
      std::cout << format( "Branch on %d=%d\n" ) % min_dom % v << std::endl;
    }
    TL( branch_at_time += std::make_tuple( timer.elapsed().wall, min_dom, v ); );
    if ( !remove_all_values_but_one( min_dom, v ) || !match_vertex( min_dom ) )
    {
      if ( verbose )
      {
        std::cout << "[i] inconsistency detected while matching" << std::endl;
      }
      d.reset_to_filter( gp.size() );
    }
    else if ( !solve( nb_sol, mapping ) )
    {
      if ( verbose )
      {
        std::cout << "[i] timeout" << std::endl;
      }
      return false;
    }
    if ( verbose )
    {
      std::cout << format( "End of branch %d=%d" ) % min_dom % v << std::endl;
    }
    boost::fill( d.global_matching_t, -1 );
    // TODO use boost::copy
    for ( const auto& u : gp.vertices() )
    {
      d.nb_val[u] = nb_val[u];
      d.global_matching_p[u] = global_matching[u];
      d.global_matching_t[global_matching[u]] = u;
    }
  }
  return true;
}

bool lad2_manager::start_lad( std::vector<unsigned>& mapping )
{
  if ( !update_matching( gp.size(), gt.size(), d.nb_val, d.first_val, d.val, d.global_matching_p ) )
  {
    return false;
  }

  if ( !ensure_gac_all_diff() )
  {
    return false;
  }

  int u;
  stack_int_t to_match;
  for ( u = 0; u < static_cast<int>( gp.size() ); ++u )
  {
    d.global_matching_t[d.global_matching_p[u]] = u;
    if ( d.nb_val[u] == 1 )
    {
      to_match.push( u );
    }
  }
  if ( !match_vertices( to_match ) )
  {
    return false;
  }

  unsigned nb_sol = 0u;
  solve( nb_sol, mapping );
  return nb_sol > 0u;
}

void lad2_manager::list_target_image( std::ostream& os )
{
  using namespace std::placeholders;
  using boost::adaptors::transformed;

  std::unordered_set<unsigned> img;
  std::vector<unsigned> vimg;

  for ( auto u : boost::counting_range( 0u, gp.num_inputs() ) )
  {
    for ( const auto& v : d.get( u ) )
    {
      img.insert( v );
    }
  }

  boost::push_back( vimg, img );
  boost::sort( vimg );
  os << "\\[ D_X = \\{" << any_join( vimg | transformed( std::bind( &simulation_graph_wrapper::name, gt, _1 ) ), ", " ) << " \\} \\]" << std::endl;
}

std::tuple<unsigned, unsigned, unsigned> lad2_manager::target_image_size()
{
  const std::vector<unsigned> sizes = { gp.num_inputs(), gp.num_vectors(), gp.num_outputs() };
  std::vector<std::unordered_set<unsigned>> img( 3u );

  auto offset = 0u;
  for ( auto i = 0u; i < 3u; ++i )
  {
    for ( auto u : boost::counting_range( 0u, sizes[i] ) )
    {
      for ( const auto& v : d.get( offset + u ) )
      {
        img[i].insert( v );
      }
    }
    offset += sizes[i];
  }

  return std::make_tuple( img[0u].size(), img[1u].size(), img[2u].size() );
}

void lad2_manager::list_with_names( std::ostream& os, bool only_inputs )
{
  os << "\\begin{align*}" << std::endl;
  for ( const auto& u : gp.vertices() )
  {
    if ( only_inputs && u == gp.num_inputs() ) { break; }
    os << format( "  D_{%s} &= \\left\\{ \\begin{array}{l}" ) % gp.name( u ) << std::endl << "    ";
    auto index = 0u;
    for ( const auto& v : d.get( u ) )
    {
      os << format( "%s, " ) % gt.name( v );
      if ( ++index % 10 == 0u )
      {
        os << " \\\\" << std::endl << "    ";
      }
    }
    os << " \\end{array}\\right\\} \\\\" << std::endl;;
  }
  os << "\\end{align*}" << std::endl;
}

bool directed_lad2_from_aig( std::vector<unsigned>& mapping, const aig_graph& target, const aig_graph& pattern, const std::vector<unsigned>& types,
                             const properties::ptr& settings, const properties::ptr& statistics )
{
  /* Settings */
  const auto functional               = get( settings, "functional",               false );
  const auto support_edges            = get( settings, "support_edges",            false );
  const auto simulation_signatures    = get( settings, "simulation_signatures",    boost::optional<unsigned>() );
  const auto on_filter                = get( settings, "on_filter",                domain_hook_t() );
  const auto on_before_first_branch   = get( settings, "on_before_first_branch",   domain_hook_t() );
  const auto texlogname               = get( settings, "texlogname",               std::string( "/tmp/log.tex" ) );

  /* Timer */
  properties_timer t( statistics );

  lad2_manager mgr( target, pattern, types, functional, support_edges, simulation_signatures, texlogname, false /*verbose*/ );
  mgr.on_before_first_branch = on_before_first_branch;
  mgr.on_filter              = on_filter;
  auto result = mgr.start_lad( mapping );

  set( statistics, "num_branches", mgr.num_branches );
  set( statistics, "pattern_vertices", mgr.gp.size() );
  set( statistics, "target_vertices", mgr.gt.size() );

  return result;
}

aig_graph shrink_block( const aig_graph& block, const aig_graph& component, const std::vector<unsigned>& types,
                        const properties::ptr& settings,
                        const properties::ptr& statistics )
{
  /* settings */
  const auto functional               = get( settings, "functional",            false );
  const auto support_edges            = get( settings, "support_edges",         false );
  const auto simulation_signatures    = get( settings, "simulation_signatures", boost::optional<unsigned>() );
  const auto verbose                  = get( settings, "verbose",               false );

  /* timing */
  properties_timer t( statistics );

  /* create graphs */
  simulation_graph_wrapper gp( component, types, support_edges, simulation_signatures );
  simulation_graph_wrapper gt( block, types, support_edges, simulation_signatures );

  /* build domain (only inputs and outputs) */
  std::unordered_set<unsigned> in_domain;

  for ( auto u = gp.num_inputs() + gp.num_vectors(); u < gp.size(); ++u )
  {
    for ( auto v = gt.num_inputs() + gt.num_vectors(); v < gt.size(); ++v )
    {
      if ( compatible_vertices( u, v, gp, gt, simulation_signatures, functional ) )
      {
        in_domain.insert( v );
      }
    }
  }

  /* find unmapped outputs */
  std::vector<std::string> names;
  for ( auto v = gt.num_inputs() + gt.num_vectors(); v < gt.size(); ++v )
  {
    if ( boost::find( in_domain, v ) == in_domain.end() )
    {
      if ( verbose )
      {
        std::cout << "[i] output " << gt.name( v ) << " cannot be mapped to" << std::endl;
      }
    }
    else
    {
      names += gt.name( v );
    }
  }

  return aig_cone( block, names );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
