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

#include <lscli/command.hpp>

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
inline bool store_can_read_io_type<bdd_function_t, io_pla_tag_t>( command& cmd ) { return true; }

template<>
bdd_function_t store_read_io_type<bdd_function_t, io_pla_tag_t>( const std::string& filename, const command& cmd );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
