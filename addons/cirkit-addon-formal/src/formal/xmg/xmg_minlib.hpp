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
 * @file xmg_minlib.hpp
 *
 * @brief Optimum XMGs from libraries
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef XMG_MINLIB_HPP
#define XMG_MINLIB_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/optional.hpp>

#include <core/properties.hpp>
#include <classical/utils/npn_manager.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <classical/xmg/xmg.hpp>

namespace cirkit
{

class xmg_minlib_manager
{
public:
  xmg_minlib_manager( const properties::ptr& settings = properties::ptr() );
  ~xmg_minlib_manager();

  void load_library_file( const std::string& filename, bool auto_update = false );
  void load_library_string( const std::string& string );
  void write_library_file( const std::string& filename, unsigned minsize = 5u );

  xmg_graph find_xmg( const tt& spec );
  xmg_graph find_xmg_no_npn( const tt& spec );
  xmg_function rewrite_inplace( const tt& spec,
                                xmg_graph& dest,
                                const std::vector<xmg_function>& pi_mapping );

  void add_to_library( const xmg_graph& xmg );
  bool verify();

  void print_statistics( std::ostream& os );

private:
  void load_library( std::istream& in );
  void add_to_library( const std::string& hex, const std::string& expr );
  std::string format_library_entry( const std::string& hex, const std::string& expr );

  std::string find_or_create_xmg( const std::string& hex );

private:
  std::unordered_map<std::string, std::string> library;
  npn_manager                                  npn;
  boost::optional<unsigned>                    timeout;
  bool                                         verbose;

  bool auto_update = false;
  std::ofstream update_out;

public: /* libraries */
  static std::string npn2_s;
  static std::string npn3_s, npn3_sd, npn3_ds;
  static std::string npn4_s, npn4_sd, npn4_ds;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
