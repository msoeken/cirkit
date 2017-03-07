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
  static std::string npn2_s_mig;
  static std::string npn3_s_mig;
  static std::string npn4_s_mig;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
