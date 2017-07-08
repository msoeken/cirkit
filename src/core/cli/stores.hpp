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
 * @file stores.hpp
 *
 * @brief Meta-data for stores
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_CORE_STORES_HPP
#define CLI_CORE_STORES_HPP

#include <string>

#include <alice/command.hpp>

#include <core/properties.hpp>
#include <core/utils/bdd_utils.hpp>

namespace alice
{

using namespace cirkit;

struct io_pla_tag_t {};

/******************************************************************************
 * bdd_function_t                                                             *
 ******************************************************************************/

template<>
struct store_info<bdd_function_t>
{
  static constexpr const char* key         = "bdds";
  static constexpr const char* option      = "bdd";
  static constexpr const char* mnemonic    = "b";
  static constexpr const char* name        = "BDD";
  static constexpr const char* name_plural = "BDDs";
};

template<>
std::string store_entry_to_string<bdd_function_t>( const bdd_function_t& bdd );

template<>
void print_store_entry<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd );

template<>
struct show_store_entry<bdd_function_t>
{
  show_store_entry( command& cmd );

  bool operator()( bdd_function_t& bdd, const std::string& dotname, const command& cmd );

  command::log_opt_t log() const;
};

template<>
void print_store_entry_statistics<bdd_function_t>( std::ostream& os, const bdd_function_t& bdd );

template<>
command::log_opt_t log_store_entry_statistics<bdd_function_t>( const bdd_function_t& bdd );

template<>
inline bool store_can_read_io_type<bdd_function_t, io_pla_tag_t>( command& cmd ) { return true; }

template<>
bdd_function_t store_read_io_type<bdd_function_t, io_pla_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<bdd_function_t, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<bdd_function_t, io_pla_tag_t>( const bdd_function_t& bdd, const std::string& filename, const command& cmd );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
