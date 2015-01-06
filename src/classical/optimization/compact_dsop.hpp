/* CirKit: A circuit toolkit
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

/**
 * @file compact_dsop.hpp
 *
 * @brief Compact DSOP computation [Bernasconi et al., TCS 53 (2013), 583-608.]
 *
 * @author  Mathias Soeken
 * @version 2.0
 */

#ifndef COMPACT_DSOP_HPP
#define COMPACT_DSOP_HPP

#include <functional>
#include <map>
#include <string>

#include <core/cube.hpp>
#include <core/properties.hpp>

namespace cirkit
{

typedef std::map<cube, int>                                       cube_weight_map_t;
typedef std::function<bool(const cube&, const cube&)>             sort_cube_func_t;
typedef std::function<sort_cube_func_t(const cube_weight_map_t&)> sort_cube_meta_func_t;

class connected_cube_list
{
public:
  connected_cube_list();
  explicit connected_cube_list( const cube_vec_t& new_cubes );

  void add( const cube& c );
  void remove( const cube& c );
  cube_vec_t remove_disjoint_cubes();

  inline void operator+=( const cube& c ) { add( c ); }
  inline void operator-=( const cube& c ) { remove( c ); }

  cube_vec_t connected_cubes( const cube& c ) const;
  void sort( const sort_cube_meta_func_t& sortfunc );

  inline const cube_vec_t& cubes() const { return _cubes; }
  inline const cube_weight_map_t& weights() const { return cube_weights; }
  inline bool empty() const { return _cubes.empty(); }

private:
  typedef std::pair<cube, unsigned>         cube_common_pair_t;
  typedef std::vector<cube_common_pair_t>   cube_common_vec_t;
  typedef std::map<cube, cube_common_vec_t> cube_common_map_t;

  cube_vec_t _cubes;
  cube_common_map_t _connected_cubes;
  cube_weight_map_t cube_weights;
};

typedef std::function<void(const cube&, const cube_vec_t&, connected_cube_list&, cube_vec_t&, const sort_cube_meta_func_t&)> opt_cube_func_t;

sort_cube_func_t sort_by_dimension_first( const cube_weight_map_t& cube_weights );
sort_cube_func_t sort_by_weight_first( const cube_weight_map_t& cube_weights );

void opt_dsop_1( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc );
void opt_dsop_2( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc );
void opt_dsop_3( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc );
void opt_dsop_4( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc );
void opt_dsop_5( const cube& cubeq, const cube_vec_t& q, connected_cube_list& p, cube_vec_t& b, const sort_cube_meta_func_t& sortfunc );

/**
 * @param settings The following settings are possible
 *                 +----------+-----------------------+------------------------------------------------+
 *                 | Name     | Type                  | Default                                        |
 *                 +----------+-----------------------+------------------------------------------------+
 *                 | sortfunc | sort_cube_meta_func_t | sort_cube_meta_func_t(sort_by_dimension_first) |
 *                 | optfunc  | opt_cube_func_t       | opt_cube_func_t(opt_dsop_1)                    |
 *                 | verbose  | bool                  | false                                          |
 *                 +----------+-----------------------+------------------------------------------------+
 */
void compact_dsop( const std::string& destination, const std::string& filename,
                   properties::ptr settings = properties::ptr(), properties::ptr statistics = properties::ptr() );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
