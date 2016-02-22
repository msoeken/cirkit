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
 * @file abc_run_command.hpp
 *
 * @brief Runs a list of semicolon separated abc commands on an AIG
 *        and return the resulting AIG.
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef ABC_RUN_COMMAND_HPP
#define ABC_RUN_COMMAND_HPP

#include <classical/aig.hpp>

#include <boost/dynamic_bitset.hpp>
#include <boost/optional.hpp>

namespace cirkit
{

aig_graph abc_run_command( const std::string& commands );
aig_graph abc_run_command( const aig_graph& aig, const std::string& commands );
void abc_run_command_no_output( const std::string& commands );
void abc_run_command_no_output( const aig_graph& aig, const std::string& commands );

boost::optional< boost::dynamic_bitset<> > abc_run_command_get_counterexample( const std::string& commands );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
