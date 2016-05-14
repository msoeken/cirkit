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
 * @file dirty.hpp
 *
 * @brief Data-type which can be invalidated
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef UTILS_DIRTY_HPP
#define UTILS_DIRTY_HPP

namespace cirkit
{

template<typename T>
class dirty
{
public:
  T& operator*() { return _m; }
  const T& operator*() const { return _m; }
  inline bool is_dirty() const { return _dirty; }

  inline void make_clean() { _dirty = false; }
  inline void make_dirty() { _dirty = true; }

  /* Fn should return T and takes no arguments */
  template<typename Fn>
  inline void update( Fn&& f )
  {
    if ( _dirty )
    {
      _m = f();
      _dirty = false;
    }
  }

private:
  bool _dirty = true;
  T    _m;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
