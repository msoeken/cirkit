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
 * @file aig_mig_command.hpp
 *
 * @brief For commands that support AIGs _or_ MIGs
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef AIG_MIG_COMMAND_HPP
#define AIG_MIG_COMMAND_HPP

#include <string>

#include <boost/format.hpp>

#include <core/cli/cirkit_command.hpp>

#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/mig/mig.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

class aig_mig_command : public cirkit_command
{
public:
  aig_mig_command( const environment::ptr& env, const std::string& caption, const std::string& opts_desc )
    : cirkit_command( env, caption )
  {
    if ( env->has_store<aig_graph>() )
    {
      opts.add_options()
        ( "aig,a", boost::str( boost::format( opts_desc ) % "AIG" ).c_str() )
        ;
    }
    if ( env->has_store<mig_graph>() )
    {
      opts.add_options()
        ( "mig,m", boost::str( boost::format( opts_desc ) % "MIG" ).c_str() )
        ;
    }
  }

protected:
  inline bool aig_selected() const
  {
    return env->has_store<aig_graph>() && is_set( "aig" );
  }

  inline bool mig_selected() const
  {
    return env->has_store<mig_graph>() && is_set( "mig" );
  }

  inline rule_t one_data_structure_selected() const
  {
    const auto assertion = [&]() {
      auto total = 0u;

      if ( aig_selected() ) { ++total; }
      if ( mig_selected() ) { ++total; }

      return total == 1u;
    };

    return {assertion, "exactly one circuit type needs to be selected"};
  }

  inline rule_t maybe_has_aig() const
  {
    const auto assertion = [&]() {
      return !( aig_selected() ) || env->store<aig_graph>().current_index() >= 0;
    };

    return {assertion, "no current AIG available"};
  }

  inline rule_t maybe_has_mig() const
  {
    const auto assertion = [&]() {
      return !( mig_selected() ) || env->store<mig_graph>().current_index() >= 0;
    };

    return {assertion, "no current MIG available"};
  }

  virtual rules_t validity_rules() const
  {
    return {one_data_structure_selected(), maybe_has_aig(), maybe_has_mig()};
  }

  inline aig_graph& aig()
  {
    return env->store<aig_graph>().current();
  }

  inline const aig_graph& aig() const
  {
    return env->store<aig_graph>().current();
  }

  inline aig_graph_info& aig_info()
  {
    return cirkit::aig_info( aig() );
  }

  inline const aig_graph_info& aig_info() const
  {
    return cirkit::aig_info( aig() );
  }

  inline mig_graph& mig()
  {
    return env->store<mig_graph>().current();
  }

  inline const mig_graph& mig() const
  {
    return env->store<mig_graph>().current();
  }

  inline mig_graph_info& mig_info()
  {
    return cirkit::mig_info( mig() );
  }

  inline const mig_graph_info& mig_info() const
  {
    return cirkit::mig_info( mig() );
  }

  virtual bool execute()
  {
    if ( !before() )
    {
      return false;
    }

    if ( aig_selected() )
    {
      if ( !execute_aig() )
      {
        return false;
      }
    }
    else if ( mig_selected() )
    {
      if ( !execute_mig() )
      {
        return false;
      }
    }

    return after();
  }

  virtual bool before()      { return true; };
  virtual bool execute_aig() { return true; };
  virtual bool execute_mig() { return true; };
  virtual bool after()       { return true; };
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
