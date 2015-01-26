/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
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

#include <fstream>
#include <iostream>
#include <vector>

#include <boost/assign/std/vector.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <core/utils/range_utils.hpp>
#include <core/utils/string_utils.hpp>
#include <core/utils/timer.hpp>

#include <classical/aig.hpp>
#include <classical/functions/aig_support.hpp>
#include <classical/functions/simulation_graph.hpp>
#include <classical/utils/simulate_aig.hpp>

using namespace boost::assign;
using boost::format;

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

typedef std::vector<int> vec_int_t;
typedef std::stack<int> stack_int_t;
typedef std::vector<vec_int_t> vec_vec_int_t;

/******************************************************************************
 * Graph                                                                      *
 ******************************************************************************/
struct lad_graph
{
  unsigned nb_vertices;
  vec_int_t vertex_label;
  vec_int_t nb_pred;
  vec_int_t nb_succ;
  vec_vec_int_t adj;
  std::vector<std::vector<char>> edge_direction;
  vec_vec_int_t edge_label;

  /* support */
  vec_int_t support;

  explicit lad_graph( const std::string& filename );
  explicit lad_graph( const aig_graph& aig, unsigned selector, bool verbose = false );
};

lad_graph::lad_graph( const std::string& filename )
{
  std::ifstream in( filename.c_str(), std::ifstream::in );
  std::string line;

  /* Read number of vertices */
  std::getline( in, line );
  nb_vertices = boost::lexical_cast<unsigned>( line );

  /* Prepare data structures */
  vertex_label.resize( nb_vertices );
  edge_label.resize( nb_vertices, vec_int_t( nb_vertices ) );
  nb_pred.resize( nb_vertices );
  nb_succ.resize( nb_vertices );
  edge_direction.resize( nb_vertices, std::vector<char>( nb_vertices ) );
  adj.resize( nb_vertices );

  /* Read vertices */
  unsigned pos = 0u, j, k;
  for ( unsigned i = 0u; i < nb_vertices; ++i )
  {
    std::getline( in, line );
    vec_int_t vline;
    parse_string_list<int>( vline, line );
    pos = 0u;

    vertex_label[i] = vline[pos++];
    nb_succ[i] = vline[pos++];
    for ( j = nb_succ[i]; j > 0u; --j )
    {
      k = vline[pos++];
      edge_label[i][k] = vline[pos++];
      if ( edge_direction[i][k] == 1 ) { assert( false ); }
      else if ( edge_direction[i][k] == 2 )
      {
        edge_direction[k][i] = 3;
        edge_direction[i][k] = 3;
        nb_pred[i]--;
        nb_succ[i]--;
        nb_succ[k]--;
      }
      else
      {
        nb_pred[k]++;
        adj[i] += k;
        adj[k] += i;
        edge_direction[i][k] = 1;
        edge_direction[k][i] = 2;
      }
    }
  }

  /* support, even though not used here */
  support.resize( nb_vertices );

  in.close();
}

lad_graph::lad_graph( const aig_graph& aig, unsigned selector, bool verbose )
{
  /* AIG info */
  const auto& aig_info = boost::get_property( aig, boost::graph_name );
  unsigned n = aig_info.inputs.size();
  unsigned m = aig_info.outputs.size();

  /* Simulate vectors */
  std::vector<unsigned> partition;
  std::vector<boost::dynamic_bitset<>> vectors;
  create_simulation_vectors( vectors, n, selector, &partition );

  /* Read number of vertices */
  nb_vertices = n + vectors.size() + m;

  /* some information */
  if ( verbose )
  {
    std::cout << format( "[i] number of inputs: %d" ) % n << std::endl
              << format( "[i] number of outputs: %d" ) % m << std::endl
              << format( "[i] number of simulation vectors: %d" ) % vectors.size() << std::endl;
  }

  /* Prepare data structures */
  vertex_label.resize( nb_vertices );
  edge_label.resize( nb_vertices, vec_int_t( nb_vertices ) );
  nb_pred.resize( nb_vertices );
  nb_succ.resize( nb_vertices );
  edge_direction.resize( nb_vertices, std::vector<char>( nb_vertices ) );
  adj.resize( nb_vertices );

  std::fill( vertex_label.begin(), vertex_label.begin() + n, 1 );
  std::fill( vertex_label.begin() + n + vectors.size(), vertex_label.end(), 0 );

  /* edges from inputs to simulation vectors */
  for ( unsigned i = 0u; i < vectors.size(); ++i )
  {
    vertex_label[n + i] = 2 + std::min( (int)vectors[i].count(), 3 );
    auto pos = vectors[i].find_first();
    while ( pos != boost::dynamic_bitset<>::npos )
    {
      nb_succ[pos]++;
      nb_pred[n + i]++;
      adj[pos] += n + i;
      adj[n + i] += pos;
      edge_direction[pos][n + i] = 1;
      edge_direction[n + i][pos] = 2;
      edge_label[pos][n + i] = 0;
      pos = vectors[i].find_next( pos );
    }
  }

  /* simulate */
  auto vectors_t = transpose( vectors );
  word_node_assignment_simulator::aig_node_value_map map;
  for ( auto word : index( vectors_t ) )
  {
    map[aig_info.inputs[word.first]] = word.second;
  }

  std::map<aig_function, boost::dynamic_bitset<>> results;
  simulate_aig( aig, word_node_assignment_simulator( map ), results );

  /* create edges */
  for ( unsigned i = 0; i < vectors.size(); ++i )
  {
    for ( unsigned j = 0; j < m; ++j )
    {
      if ( results[aig_info.outputs[j].first][i] )
      {
        nb_succ[n + i]++;
        nb_pred[n + vectors.size() + j]++;
        adj[n + i] += n + vectors.size() + j;
        adj[n + vectors.size() + j] += n + i;
        edge_direction[n + i][n + vectors.size() + j] = 1;
        edge_direction[n + vectors.size() + j][n + i] = 2;
        edge_label[n + i][n + vectors.size() + j] = 1 + std::min( (int)vectors[i].count(), 3 );
      }
    }
  }

  /* support */
  support.resize( nb_vertices );
  auto s = aig_structural_support( aig );
  for ( auto o : index( aig_info.outputs ) )
  {
    support[n + vectors.size() + o.first] = s[o.second.first];
  }
}

std::ostream& operator<<( std::ostream& os, const lad_graph& g )
{
  os << format( "Directed labelled graph with %d vertices" ) % g.nb_vertices << std::endl;

  for ( int i = 0; i < g.nb_vertices; ++i )
  {
    os << format( "Vertex %d has label %d and %d adjacent vertices (%d pred and %d succ): ") % i % g.vertex_label[i] % g.adj[i].size() % g.nb_pred[i] % g.nb_succ[i];
    for ( int k : g.adj[i] )
    {
      os << format( " %d(" ) % k;
      switch ( g.edge_direction[i][k] )
      {
      case 1: os << "succ"; break;
      case 2: os << "pred"; break;
      case 3: os << "succ and pred"; break;
      }
      os << "; label " << g.edge_label[i][k] << ")";
    }
    os << std::endl;
  }
  return os;
}

/******************************************************************************
 * Domains                                                                    *
 ******************************************************************************/
struct lad_domain
{
  vec_int_t nb_val;
  vec_int_t first_val;
  vec_int_t val;
  vec_vec_int_t pos_in_val;
  vec_vec_int_t first_match;
  vec_int_t matching;
  int next_out_to_filter;
  int last_in_to_filter;
  vec_int_t to_filter;
  boost::dynamic_bitset<> marked_to_filter;
  vec_int_t global_matching_p;
  vec_int_t global_matching_t;

  lad_domain( const lad_graph& gp, const lad_graph& gt, bool functional_support_constraints = false );

  inline bool to_filter_empty() const
  {
    return next_out_to_filter < 0;
  }

  inline void reset_to_filter( int size )
  {
    for ( unsigned i = 0u; i < size; ++i ) { marked_to_filter.reset( i ); }
    //std::fill( marked_to_filter.begin(), marked_to_filter.begin() + size, 0 );
    next_out_to_filter = -1;
  }

  inline int next_to_filter( int size )
  {
    int u = to_filter[next_out_to_filter];
    marked_to_filter.reset( u );
    if ( next_out_to_filter == last_in_to_filter )
    {
      next_out_to_filter = -1;
    }
    else if ( next_out_to_filter == size - 1 )
    {
      next_out_to_filter = 0;
    }
    else
    {
      ++next_out_to_filter;
    }
    return u;
  }

  inline void add_to_filter( int u, int size )
  {
    if ( marked_to_filter.test( u ) ) return;
    marked_to_filter.set( u );
    if ( next_out_to_filter < 0 )
    {
      last_in_to_filter = 0;
      next_out_to_filter = 0;
    }
    else if ( last_in_to_filter == size - 1 )
    {
      last_in_to_filter = 0;
    }
    else
    {
      ++last_in_to_filter;
    }
    to_filter[last_in_to_filter] = u;
  }

  inline bool is_in_domain( int u, int v ) const
  {
    return pos_in_val[u][v] < first_val[u] + nb_val[u];
  }

  bool augmenting_path( int u, int nbv );
  bool remove_all_values_but_one( int u, int v, const lad_graph& gp, const lad_graph& gt );
  bool remove_value( int u, int v, const lad_graph& gp, const lad_graph& gt );
  bool match_vertices( stack_int_t& to_be_matched, const lad_graph& gp, const lad_graph& gt );
  inline bool match_vertex( int u, const lad_graph& gp, const lad_graph& gt )
  {
    stack_int_t to_be_matched;
    to_be_matched.push( u );
    return match_vertices( to_be_matched, gp, gt );
  }

  void dump( const std::string& filename );
};

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
                          const lad_graph& gp, const lad_graph& gt, bool functional_support_constraints )
{
  if ( !compatible_vertex_labels( gp.vertex_label[u], gt.vertex_label[v] ) )
  {
    return false;
  }

  // INDUCED
  if ( gp.nb_pred[u] > gt.nb_pred[v] ) return false;
  if ( gp.nb_succ[u] > gt.nb_succ[v] ) return false;
  if ( gp.adj[u].size() - gp.nb_pred[u] - gp.nb_succ[u] > gt.adj[v].size() - gt.nb_pred[v] - gt.nb_succ[v] ) return false;
  //if ( gp.adj[u].size() > gt.adj[v].size() ) return false;
  //if ( gp.nb_pred[u] > gt.adj[v].size() ) return false;
  //if ( gp.nb_succ[u] > gt.adj[v].size() - gt.nb_pred[v] ) return false;
  //if ( gp.adj[u].size() - gp.nb_pred[u] - gp.nb_succ[u] > gt.adj[v].size() - gt.nb_pred[v] - gt.nb_succ[v] ) return false;
  if ( functional_support_constraints && ( gt.support[v] != gp.support[u] ) ) return false;
  if ( !functional_support_constraints && ( gt.support[v] < gp.support[u] ) ) return false;
  return true;
}

lad_domain::lad_domain( const lad_graph& gp, const lad_graph& gt, bool functional_support_constraints )
{
  /* create */
  global_matching_p.resize( gp.nb_vertices, -1 );
  global_matching_t.resize( gt.nb_vertices, -1 );
  nb_val.resize( gp.nb_vertices, 0 );
  first_val.resize( gp.nb_vertices );
  pos_in_val.resize( gp.nb_vertices, vec_int_t( gt.nb_vertices ) );
  first_match.resize( gp.nb_vertices, vec_int_t( gt.nb_vertices ) );
  marked_to_filter.resize( gp.nb_vertices, true );
  to_filter.resize( gp.nb_vertices );

  /* initialize */
  int matching_size = 0;
  int val_size = 0;
  unsigned v;
  for ( unsigned u = 0u; u < gp.nb_vertices; ++u )
  {
    to_filter[u] = u;
    first_val[u] = val_size;
    for ( v = 0u; v < gt.nb_vertices; ++v )
    {
      if ( !compatible_vertices( u, v, gp, gt, functional_support_constraints ) ) /* v not in D[u] */
      {
        pos_in_val[u][v] = first_val[u] + gt.nb_vertices;
      }
      else /* v in D[u] */
      {
        first_match[u][v] = matching_size;
        matching_size += gp.adj[u].size();
        val += v;
        nb_val[u]++;
        pos_in_val[u][v] = val_size++;
      }
    }
  }

  matching.resize( matching_size, -1 );
  next_out_to_filter = 0;
  last_in_to_filter = gp.nb_vertices - 1;
}

bool lad_domain::augmenting_path( int u, int nbv )
{
  vec_int_t fifo( nbv );
  vec_int_t pred( nbv );
  int next_in = 0;
  int next_out = 0;
  int i, j, v, v2, u2;
  std::vector<char> marked( nbv );
  for ( i = 0u; i < nb_val[u]; ++i )
  {
    v = val[first_val[u] + i]; /* v in D(u) */
    if ( global_matching_t[v] < 0 ) /* v is free => augmenting path found */
    {
      global_matching_p[u] = v;
      global_matching_t[v] = u;
      return true;
    }

    /* v is not free => add to fifo */
    pred[v] = u;
    fifo[next_in++] = v;
    marked[v] = 1;
  }

  while ( next_out < next_in )
  {
    u2 = global_matching_t[fifo[next_out++]];
    for ( i = 0u; i < nb_val[u2]; ++i )
    {
      v = val[first_val[u2] + i]; /* v in D(u2) */
      if ( global_matching_t[v] < 0 ) /* v is free => augmenting path found */
      {
        j = 0;
        while ( u2 != u )
        {
          if ( j > 100 ) assert( false ); ++j;
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
      if ( !marked[v] ) /* v is not free and not marked => add it to fifo */
      {
        pred[v] = u2;
        fifo[next_in++] = v;
        marked[v] = 1;
      }
    }
  }

  return false;
}

bool lad_domain::remove_all_values_but_one( int u, int v, const lad_graph& gp, const lad_graph& gt )
{
  for ( int j : gp.adj[u] )
  {
    add_to_filter( j, gp.nb_vertices );
  }

  int old_pos = pos_in_val[u][v];
  int new_pos = first_val[u];
  val[old_pos] = val[new_pos];
  val[new_pos] = v;
  pos_in_val[u][val[new_pos]] = new_pos;
  pos_in_val[u][val[old_pos]] = old_pos;
  nb_val[u] = 1;
  if ( global_matching_p[u] != v )
  {
    global_matching_t[global_matching_p[u]] = -1;
    global_matching_p[u] = -1;
    return augmenting_path( u, gt.nb_vertices );
  }
  return true;
}

bool lad_domain::remove_value( int u, int v, const lad_graph& gp, const lad_graph& gt )
{
  for ( int j : gp.adj[u] )
  {
    add_to_filter( j, gp.nb_vertices );
  }
  int old_pos = pos_in_val[u][v];
  nb_val[u]--;
  int new_pos = first_val[u] + nb_val[u];
  val[old_pos] = val[new_pos];
  val[new_pos] = v;
  pos_in_val[u][val[old_pos]] = old_pos;
  pos_in_val[u][val[new_pos]] = new_pos;
  if ( global_matching_p[u] == v )
  {
    global_matching_p[u] = -1;
    global_matching_t[v] = -1;
    return augmenting_path( u, gt.nb_vertices );
  }
  return true;
}

bool lad_domain::match_vertices( stack_int_t& to_be_matched, const lad_graph& gp, const lad_graph& gt )
{
  int j, u, v, u2, old_nb_val;
  while ( !to_be_matched.empty() )
  {
    u = to_be_matched.top();
    to_be_matched.pop();
    v = val[first_val[u]];
    for ( u2 = 0; u2 < gp.nb_vertices; ++u2 )
    {
      if ( u != u2 )
      {
        old_nb_val = nb_val[u2];
        if ( is_in_domain( u2, v ) && !remove_value( u2, v, gp, gt ) ) { return false; }
        if ( gp.edge_direction[u][u2] != 0 )
        {
          j = first_val[u2];
          while ( j < first_val[u2] + nb_val[u2] )
          {
            if ( compatible_edge_labels( gp.edge_label[u][u2], gt.edge_label[v][val[j]] ) && is_compatible ( gp.edge_direction[u][u2], gt.edge_direction[v][val[j]] ) ) { ++j; }
            else if ( !remove_value( u2, val[j], gp, gt ) ) { return false; }
          }
        }
        /* INDUCED */
        else // (u,u2) is not an edge => remove neighbors of v from D[u2]
        {
					j = first_val[u2];
					while ( j < first_val[u2] + nb_val[u2] )
          {
						if ( gt.edge_direction[v][val[j]] == 0) { ++j; }
						else if ( !remove_value( u2, val[j], gp, gt ) ) { return false; }
					}
				}

        if ( nb_val[u2] == 0 ) { return false; }
        if ( nb_val[u2] == 1 && old_nb_val > 1 )
        {
          to_be_matched.push( u2 );
        }
      }
    }
  }

  return true;
}

std::ostream& operator<<( std::ostream& os, const lad_domain& d )
{
  int u, i;
  for ( u = 0; u < d.nb_val.size(); ++u )
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

void lad_domain::dump( const std::string& filename )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  os << "nbVal: " << any_join( nb_val, " " ) << std::endl
     << "firstVal: " << any_join( first_val, " " ) << std::endl
     << "val: " << any_join( boost::make_iterator_range( val.begin(), val.begin() + first_val.back() + nb_val.back() ), " " ) << std::endl;
  for ( const auto& p : pos_in_val )
  {
    os << "posInVal[]: " << any_join( p, " " ) << std::endl;
  }
  for ( const auto& p : first_match )
  {
    os << "firstMatch[]: " << any_join( p, " " ) << std::endl;
  }
  os << "matching: " << any_join( matching, " " ) << std::endl
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

struct lad_manager
{
  lad_manager( lad_domain& d, const lad_graph& gp, const lad_graph& gt )
    : d( d ), gp( gp ), gt( gt ),
      num( gt.nb_vertices ),
      num_inv( gt.nb_vertices ),
      nb_comp( gp.nb_vertices ),
      first_comp( gp.nb_vertices ),
      matched_with_u( gp.nb_vertices ) {}

  lad_domain& d;
  const lad_graph& gp;
  const lad_graph& gt;

  /* for check_lad */
  vec_int_t num;
  vec_int_t num_inv;
  vec_int_t nb_comp;
  vec_int_t first_comp;
  vec_int_t comp;
  vec_int_t matched_with_u;
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

bool update_matching( int size_of_u, int size_of_v, const vec_int_t& degree, const vec_int_t& first_adj, const vec_int_t& adj, vec_int_t& matched_with_u, lad_manager& mgr )
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
  while ( j < unmatched.size() )
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

void dfs( int nbu, int nbv, int u, std::vector<char>& marked, const vec_int_t& nb_succ, const vec_vec_int_t& succ, const vec_int_t& matched_with_u, vec_int_t& order, int* nb )
{
  marked[u] = 1;
  int v = matched_with_u[u];
  for ( int i = 0; i < nb_succ[v]; ++i )
  {
    if ( !marked[succ[v][i]] )
    {
      dfs( nbu, nbv, succ[v][i], marked, nb_succ, succ, matched_with_u, order, nb );
    }
  }
  order[*nb] = u; (*nb)--;
}

void scc( int nbu, int nbv, vec_int_t& numv, vec_int_t& numu,
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
      dfs( nbu, nbv, u, marked, nb_succ, succ, matched_with_u, order, &nb );
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

bool ensure_gac_all_diff( lad_manager& mgr )
{
  vec_int_t nb_pred( mgr.gp.nb_vertices );
  vec_vec_int_t pred( mgr.gp.nb_vertices, vec_int_t( mgr.gt.nb_vertices ) );
  vec_int_t nb_succ( mgr.gt.nb_vertices );
  vec_vec_int_t succ( mgr.gt.nb_vertices, vec_int_t( mgr.gp.nb_vertices ) );
	int u, v, i, w, old_nb_val;
  vec_int_t numv( mgr.gt.nb_vertices ), numu( mgr.gp.nb_vertices );
  stack_int_t to_match;
  std::vector<std::vector<char>> used( mgr.gp.nb_vertices, std::vector<char>( mgr.gt.nb_vertices ) );
  for ( u = 0; u < mgr.gp.nb_vertices; ++u )
  {
    for ( i = 0; i < mgr.d.nb_val[u]; ++i )
    {
      v = mgr.d.val[mgr.d.first_val[u] + i];
      used[u][v] = 0;
      if ( v != mgr.d.global_matching_p[u] )
      {
        pred[u][nb_pred[u]++] = v;
        succ[v][nb_succ[v]++] = u;
      }
    }
  }

  vec_int_t list( mgr.gt.nb_vertices );
  int nb = 0;
  for ( v = 0; v < mgr.gt.nb_vertices; ++v )
  {
    if ( mgr.d.global_matching_t[v] < 0 )
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
				w = mgr.d.global_matching_p[u];
				used[u][w] = 1;
				if ( numv[w] == 0 )
        {
					list[nb++] = w;
					numv[w] = 1;
				}
			}
		}
	}

  scc( mgr.gp.nb_vertices, mgr.gt.nb_vertices, numv, numu,
       nb_succ, succ, nb_pred, pred, mgr.d.global_matching_p, mgr.d.global_matching_t );

	for ( u = 0; u < mgr.gp.nb_vertices; ++u )
  {
    old_nb_val = mgr.d.nb_val[u];
		for ( i = 0; i < mgr.d.nb_val[u]; ++i)
    {
      v = mgr.d.val[mgr.d.first_val[u] + i];
      if ( !used[u][v] && numv[v] != numu[u] && mgr.d.global_matching_p[u] != v )
      {
        if ( !mgr.d.remove_value( u, v, mgr.gp, mgr.gt ) )
        {
          return false;
        }
      }
    }
    if ( mgr.d.nb_val[u] == 0 ) { return false; }
    if ( old_nb_val > 1 && mgr.d.nb_val[u] == 1 )
    {
      to_match.push( u );
    }
	}
	return mgr.d.match_vertices( to_match, mgr.gp, mgr.gt );
}

/******************************************************************************
 * LAD                                                                        *
 ******************************************************************************/
bool check_lad( int u, int v, lad_domain& d, const lad_graph& gp, const lad_graph& gt, lad_manager& mgr )
{
  int u2, v2, i, j;
  int nb_matched = 0;

  if ( gp.adj[u].size() == 1u )
  {
    u2 = gp.adj[u][0];
    v2 = d.matching[d.first_match[u][v]];
    if ( v2 != -1 && d.is_in_domain( u2, v2 ) ) { return true; }

    for ( i = d.first_val[u2]; i < d.first_val[u2] + d.nb_val[u2]; ++i )
    {
      if ( compatible_edge_labels( gp.edge_label[u][u2], gt.edge_label[v][d.val[i]] ) &&
           is_compatible( gp.edge_direction[u][u2], gt.edge_direction[v][d.val[i]] ) )
      {
        d.matching[d.first_match[u][v]] = d.val[i];
        return true;
      }
    }

    return false;
  }

  for ( i = 0; i < gp.adj[u].size(); ++i )
  {
    u2 = gp.adj[u][i];
    v2 = d.matching[d.first_match[u][v] + i];
    if ( v2 != -1  && d.is_in_domain( u2, v2 ) ) { ++nb_matched; }
  }
  if ( nb_matched == gp.adj[u].size() ) { return true; }

  mgr.comp.resize( gp.adj[u].size() * gt.nb_vertices );

  boost::fill( mgr.num, -1 );

  int nb_num = 0;
  int pos_in_comp = 0;
  for ( i = 0; i < gp.adj[u].size(); ++i )
  {
    u2 = gp.adj[u][i];
    mgr.nb_comp[i] = 0;
    mgr.first_comp[i] = pos_in_comp;
    if ( d.nb_val[u2] > gt.adj[v].size() )
    {
      for ( j = d.first_val[u2]; j < d.first_val[u2] + d.nb_val[u2]; ++j )
      {
        v2 = d.val[j];
        if ( compatible_edge_labels( gp.edge_label[u][u2], gt.edge_label[v][v2] ) &&
             is_compatible( gp.edge_direction[u][u2], gt.edge_direction[v][v2] ) )
        {
          if ( mgr.num[v2] < 0 )
          {
            mgr.num[v2] = nb_num;
            mgr.num_inv[nb_num++] = v2;
          }
          mgr.comp[pos_in_comp++] = mgr.num[v2];
          mgr.nb_comp[i]++;
        }
      }
    }
    else
    {
      for ( int v2 : gt.adj[v] )
      {
        if ( d.is_in_domain( u2, v2 ) &&
             compatible_edge_labels( gp.edge_label[u][u2], gt.edge_label[v][v2] ) &&
             is_compatible( gp.edge_direction[u][u2], gt.edge_direction[v][v2] ) )
        {
          if ( mgr.num[v2] < 0 )
          {
            mgr.num[v2] = nb_num;
            mgr.num_inv[nb_num++] = v2;
          }
          mgr.comp[pos_in_comp++] = mgr.num[v2];
          mgr.nb_comp[i]++;
        }
      }
    }
    if ( mgr.nb_comp[i] == 0 ) { return false; }
    v2 = d.matching[d.first_match[u][v] + i];
    if ( v2 != -1 && d.is_in_domain( u2, v2 ) )
    {
      mgr.matched_with_u[i] = mgr.num[v2];
    }
    else
    {
      mgr.matched_with_u[i] = -1;
    }
  }

  if ( !update_matching( gp.adj[u].size(), nb_num, mgr.nb_comp, mgr.first_comp, mgr.comp, mgr.matched_with_u, mgr ) )
  {
    return false;
  }
  for ( i = 0; i < gp.adj[u].size(); ++i )
  {
    d.matching[d.first_match[u][v] + i] = mgr.num_inv[mgr.matched_with_u[i]];
  }
  return true;
}

bool filter( lad_manager& mgr )
{
  int u, v, i, old_nb_val;
  while ( !mgr.d.to_filter_empty() )
  {
    while ( !mgr.d.to_filter_empty() )
    {
      u = mgr.d.next_to_filter( mgr.gp.nb_vertices );
      old_nb_val = mgr.d.nb_val[u];
      i = mgr.d.first_val[u];
      while ( i < mgr.d.first_val[u] + mgr.d.nb_val[u] )
      {
        v = mgr.d.val[i];
        if ( check_lad( u, v, mgr.d, mgr.gp, mgr.gt, mgr ) ) { ++i; }
        else if ( !mgr.d.remove_value( u, v, mgr.gp, mgr.gt ) ) { return false; }
      }
      if ( mgr.d.nb_val[u] == 1 && old_nb_val > 1 && !mgr.d.match_vertex( u, mgr.gp, mgr.gt ) )
      {
        return false;
      }
      if ( mgr.d.nb_val[u] == 0 ) { return false; }
    }
    if ( !ensure_gac_all_diff( mgr ) ) { return false; }
  }
  return true;
}

bool solve( lad_manager& mgr, unsigned& nb_sol, std::vector<unsigned>& mapping, bool verbose )
{
  int u, v, min_dom, i;
  vec_int_t nb_val( mgr.gp.nb_vertices );
  vec_int_t global_matching( mgr.gp.nb_vertices );

  if ( !filter( mgr ) )
  {
    if ( verbose )
    {
      std::cout << "[i] reset to filter" << std::endl;
    }
    mgr.d.reset_to_filter( mgr.gp.nb_vertices );
    return true;
  }

  min_dom = -1;
  for ( u = 0; u < mgr.gp.nb_vertices; ++u )
  {
    nb_val[u] = mgr.d.nb_val[u];
    if ( nb_val[u] > 1 && ( min_dom < 0 || nb_val[u] < nb_val[min_dom] ) )
    {
      min_dom = u;
    }
    global_matching[u] = mgr.d.global_matching_p[u];
  }


  if ( min_dom == -1 )
  {
    ++nb_sol;
    mapping.resize( mgr.gp.nb_vertices );
    std::cout << format( "Solution %d:" ) % nb_sol;
    for ( u = 0; u < mgr.gp.nb_vertices; ++u )
    {
      mapping[u] = mgr.d.val[mgr.d.first_val[u]];
      std::cout << format( " %d=%d" ) % u % mgr.d.val[mgr.d.first_val[u]];
    }
    std::cout << std::endl;
    mgr.d.reset_to_filter( mgr.gp.nb_vertices );
    return true;
  }

  vec_int_t val( mgr.d.nb_val[min_dom] );
  for ( i = 0; i < mgr.d.nb_val[min_dom]; ++i )
  {
    // TODO use boost::copy
    val[i] = mgr.d.val[mgr.d.first_val[min_dom] + i];
  }

  for ( i = 0; i < nb_val[min_dom] && nb_sol == 0; ++i )
  {
    v = val[i];
    if ( verbose )
    {
      std::cout << format( "Branch on %d=%d\n" ) % min_dom % v << std::endl;
    }
    if ( !mgr.d.remove_all_values_but_one( min_dom, v, mgr.gp, mgr.gt ) || !mgr.d.match_vertex( min_dom, mgr.gp, mgr.gt ) )
    {
      if ( verbose )
      {
        std::cout << "[i] inconsistency detected while matching" << std::endl;
      }
      mgr.d.reset_to_filter( mgr.gp.nb_vertices );
    }
    else if ( !solve( mgr, nb_sol, mapping, verbose ) )
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
    boost::fill( mgr.d.global_matching_t, -1 );
    // TODO use boost::copy
    for ( u = 0; u < mgr.gp.nb_vertices; ++u )
    {
      mgr.d.nb_val[u] = nb_val[u];
      mgr.d.global_matching_p[u] = global_matching[u];
      mgr.d.global_matching_t[global_matching[u]] = u;
    }
  }
  return true;
}

bool start_lad( lad_manager& mgr, std::vector<unsigned>& mapping, bool verbose )
{
  if ( !update_matching( mgr.gp.nb_vertices, mgr.gt.nb_vertices, mgr.d.nb_val, mgr.d.first_val, mgr.d.val, mgr.d.global_matching_p, mgr ) )
  {
    return false;
  }

  if ( !ensure_gac_all_diff( mgr ) )
  {
    return false;
  }

  int u;
  stack_int_t to_match;
  for ( u = 0; u < mgr.gp.nb_vertices; ++u )
  {
    mgr.d.global_matching_t[mgr.d.global_matching_p[u]] = u;
    if ( mgr.d.nb_val[u] == 1 )
    {
      to_match.push( u );
    }
  }
  if ( !mgr.d.match_vertices( to_match, mgr.gp, mgr.gt ) )
  {
    return false;
  }

  unsigned nb_sol = 0u;
  solve( mgr, nb_sol, mapping, verbose );
  return nb_sol > 0u;
}

bool directed_lad( std::vector<unsigned>& mapping, const std::string& target, const std::string& pattern,
                   properties::ptr settings, properties::ptr statistics )
{
  /* Settings */
  bool verbose = get( settings, "verbose", false );

  /* Timer */
  timer<properties_timer> t;

  if ( statistics )
  {
    properties_timer rt( statistics );
    t.start( rt );
  }

  lad_graph gp( pattern );
  lad_graph gt( target );
  lad_domain d( gp, gt );

  if ( verbose )
  {
    std::cout << "Pattern graph:" << std::endl << gp
              << "Target graph:" << std::endl << gt << d;
  }

  if ( statistics )
  {
    statistics->set( "pattern_vertices", (unsigned)gp.nb_vertices );
    statistics->set( "target_vertices", (unsigned)gt.nb_vertices );
  }

  lad_manager mgr( d, gp, gt );
  return start_lad( mgr, mapping, verbose );
}

bool directed_lad_from_aig( std::vector<unsigned>& mapping, const aig_graph& target, const aig_graph& pattern, unsigned selector,
                            properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() )
{
  /* Settings */
  bool functional = get( settings, "functional", false );
  bool verbose    = get( settings, "verbose",    false );

  /* Timer */
  timer<properties_timer> t;

  if ( statistics )
  {
    properties_timer rt( statistics );
    t.start( rt );
  }

  lad_graph gp( pattern, selector, verbose );
  lad_graph gt( target, selector, verbose );
  lad_domain d( gp, gt, functional );

  if ( verbose )
  {
    //std::cout << "Pattern graph:" << std::endl << gp
    //          << "Target graph:" << std::endl << gt << d;
    std::cout << format( "[i] target graph has %d vertices" ) % gt.nb_vertices << std::endl
              << format( "[i] pattern graph has %d vertices" ) % gp.nb_vertices << std::endl;
  }

  if ( statistics )
  {
    statistics->set( "pattern_vertices", (unsigned)gp.nb_vertices );
    statistics->set( "target_vertices", (unsigned)gt.nb_vertices );
  }

  lad_manager mgr( d, gp, gt );
  return start_lad( mgr, mapping, /*verbose*/ false );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
