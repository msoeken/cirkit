/* alice: A C++ EDA command line interface API
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file current.hpp
 *
 * @brief Switches current data structure
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <alice/command.hpp>

using namespace boost::program_options;

namespace alice
{

template<typename S>
int set_current_index_helper( const command& cmd, const environment::ptr& env, unsigned index )
{
  constexpr auto option = store_info<S>::option;

  if ( cmd.is_set( option ) && index < env->store<S>().size() )
  {
    env->store<S>().set_current_index( index );
  }
  return 0;
}

template<class... S>
class current_command : public command
{
public:
  current_command( const environment::ptr& env )
    : command( env, "Switches current data structure" )
  {
    add_positional_option( "index" );
    opts.add_options()
      ( "index,i", value( &index ), "new index" )
      ;

    [](...){}( add_option_helper<S>( opts )... );
  }

protected:
  rules_t validity_rules() const
  {
    rules_t rules;

    rules.push_back( {[this]() { return exactly_one_true_helper( { is_set( store_info<S>::option )... } ); }, "exactly one store needs to be specified" } );

    return rules;
  }

  bool execute()
  {
    [](...){}( set_current_index_helper<S>( *this, env, index )... );

    return true;
  }

private:
  unsigned index;
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
