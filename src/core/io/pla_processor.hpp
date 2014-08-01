/* RevKit (www.rekit.org)
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
 * @file pla_processor.hpp
 *
 * @brief Processor which works with the pla_parser
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef PLA_PROCESSOR_HPP
#define PLA_PROCESSOR_HPP

#include <iosfwd>
#include <vector>

namespace cirkit
{

  /**
   * @brief Base class for actions on the pla_parser
   *
   * @since  2.0
   */
  class pla_processor
  {
  public:
      virtual void on_comment( const std::string& comment ) {}
      virtual void on_num_inputs( unsigned num_inputs ) {}
      virtual void on_num_outputs( unsigned num_outputs ) {}
      virtual void on_num_products( unsigned num_products ) {}
      virtual void on_input_labels( const std::vector<std::string>& input_labels ) {}
      virtual void on_output_labels( const std::vector<std::string>& output_labels ) {}
      virtual void on_end() {}
      virtual void on_type( const std::string& type ) {}
      virtual void on_cube( const std::string& in, const std::string& out ) {}
  };
}

#endif /* PLA_PROCESSOR_HPP */

// Local Variables:
// c-basic-offset: 2
// End:
