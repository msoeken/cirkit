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
 * @file help.hpp
 *
 * @brief Shows help
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <algorithm>
#include <iostream>

#include <boost/format.hpp>

#include <alice/command.hpp>

namespace alice
{

class help_command : public command
{
public:
  help_command( const environment::ptr& env )  : command( env, "Shows help" )
  {
    opts.add_options()
      ( "detailed,d", "show command descriptions" )
      ;
  }

protected:
  bool execute()
  {
    for ( auto& p : env->categories )
    {
      std::cout << p.first << " commands:" << std::endl;

      std::sort( p.second.begin(), p.second.end() );

      if ( is_set( "detailed" ) )
      {
        for ( const auto& name : p.second )
        {
          std::cout << boost::format( " %-17s : %s" ) % name % env->commands[name]->caption() << std::endl;
        }
        std::cout << std::endl;
      }
      else
      {
        auto counter = 0;
        std::cout << " ";

        for ( const auto& name : p.second )
        {
          if ( counter > 0 && ( counter % 4 == 0 ) )
          {
            std::cout << std::endl << " ";
          }
          std::cout << boost::format( "%-17s" ) % name;
          ++counter;
        }
        std::cout << std::endl << std::endl;
      }
    }

    return true;
  }
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
