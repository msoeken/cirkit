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
 * @file convert.hpp
 *
 * @brief Convert something into something else
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <alice/command.hpp>

namespace po = boost::program_options;

namespace alice
{

template<typename D, typename S>
int add_combination_helper_inner( po::options_description& opts )
{
  if ( store_can_convert<S, D>() )
  {
    constexpr auto source_option = store_info<S>::option;
    constexpr auto dest_option   = store_info<D>::option;

    constexpr auto source_name   = store_info<S>::name;
    constexpr auto dest_name     = store_info<D>::name;

    opts.add_options()
      ( ( boost::format( "%s_to_%s" ) % source_option % dest_option ).str().c_str(),
        ( boost::format( "convert %s to %s" ) % source_name % dest_name ).str().c_str() )
      ;
  }
  return 0;
};

template<typename D, class... S>
int add_combination_helper( po::options_description& opts )
{
  [](...){}( add_combination_helper_inner<D, S>( opts )... );
  return 0;
};

template<typename D, class S>
int convert_helper_inner( const environment::ptr& env, const command& cmd )
{
  if ( store_can_convert<S, D>() )
  {
    constexpr auto source_option = store_info<S>::option;
    constexpr auto dest_option   = store_info<D>::option;

    if ( cmd.is_set( ( boost::format( "%s_to_%s" ) % source_option % dest_option ).str().c_str() ) )
    {
      constexpr auto source_name = store_info<S>::name;
      const auto& source_store = env->store<S>();

      if ( source_store.current_index() == -1 )
      {
        std::cout << boost::format( "[w] there is no %s to convert from" ) % source_name << std::endl;
        return 0;
      }

      auto& dest_store = env->store<D>();
      dest_store.extend();
      dest_store.current() = store_convert<S, D>( source_store.current() );
    }
  }
  return 0;
}

template<typename D, class... S>
int convert_helper( const environment::ptr& env, const command& cmd )
{
  [](...){}( convert_helper_inner<D, S>( env, cmd )... );
  return 0;
}

template<class... S>
class convert_command : public command
{
public:
  convert_command( const environment::ptr& env )
    : command( env, "Convert something into something else" )
  {
    [](...){}( add_combination_helper<S, S...>( opts )... );
  }

protected:
  bool execute()
  {
    [](...){}( convert_helper<S, S...>( env, *this )... );
    return true;
  }
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
