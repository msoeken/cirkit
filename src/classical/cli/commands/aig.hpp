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

/**
 * @file aig.hpp
 *
 * @brief Creates an AIG from some other structure
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_AIG_COMMAND_HPP
#define CLI_AIG_COMMAND_HPP

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <core/cli/command.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/utils/aig_utils.hpp>

using namespace boost::program_options;

namespace cirkit
{

template<typename S>
int add_aig_option_helper( program_options& opts )
{
  constexpr auto option   = store_info<S>::option;
  constexpr auto mnemonic = store_info<S>::mnemonic;
  constexpr auto name     = store_info<S>::name;

  if ( store_can_convert<S, aig_graph>() )
  {
    opts.add_options()
      ( ( boost::format( "%s,%s" ) % option % mnemonic ).str().c_str(), boost::str( boost::format( "Create AIG from %s" ) % name ).c_str() )
      ;
  }
  return 0;
}

template<typename S>
int aig_convert_helper( const program_options& opts, const environment::ptr& env )
{
  constexpr auto option = store_info<S>::option;

  if ( opts.is_set( option ) )
  {
    env->store<aig_graph>().current() = store_convert<S, aig_graph>( env->store<S>().current() );
  }
  return 0;
}

template<class... S>
class aig_command : public command
{
public:
  aig_command( const environment::ptr& env )
    : command( env, "Creates an AIG from some other structure" )
  {
    [](...){}( add_aig_option_helper<S>( opts )... );

    opts.add_options()
      //      ( "mig,m",                  "Create AIG from MIG" )
      //      ( "tt,t",                   "Create AIG from truth table" )
      ( "new,n",                  "Add a new entry to the store; if not set, the current entry is overriden" )
      ( "name",   value( &name ), "Override default name" )
      ;
    be_verbose();
  }

protected:
  //rules_t validity_rules() const;
  bool execute()
  {
    auto& aigs = env->store<aig_graph>();

    if ( aigs.empty() || opts.is_set( "new" ))
    {
      aigs.extend();
    }

    [](...){}( aig_convert_helper<S>( opts, env )... );

    if ( opts.is_set( "name" ) )
    {
      aig_info( aigs.current() ).model_name = name;
    }

    return true;
  }

private:
  std::string name;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
