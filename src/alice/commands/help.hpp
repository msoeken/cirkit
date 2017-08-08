/* alice: A C++ EDA command line interface API
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
#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <alice/command.hpp>

namespace alice
{

using boost::program_options::value;

class help_command : public command
{
public:
  help_command( const environment::ptr& env )  : command( env, "Shows help" )
  {
    opts.add_options()
      ( "detailed,d",                   "show command descriptions" )
      ( "search,s",   value( &search ), "search for commands in help descriptions" )
      ;
  }

protected:
  bool execute()
  {
    if ( is_set( "search" ) )
    {
      search_command();
    }
    else
    {
      print_commands();
    }

    return true;
  }

private:
  void search_command()
  {
    for ( const auto& command : env->commands )
    {
      std::stringstream ss;
      ss << command.second->opts;

      auto text = ss.str();
      std::string::iterator it;
      std::string::size_type pos = 0;

      /* split text */
      std::vector<std::string> lines;
      boost::split( lines, text, boost::is_any_of( "\n" ), boost::algorithm::token_compress_on );

      const auto pred = []( char ch1, char ch2 ) {
        return std::toupper( ch1 ) == std::toupper( ch2 );
      };

      std::string output;
      for ( auto& line : lines )
      {
        auto matched = false;
        while ( ( it = std::search( line.begin() + pos, line.end(), search.begin(), search.end(), pred ) ) != line.end() )
        {
          matched = true;
          pos = std::distance( line.begin(), it );

          std::string found( it, it + search.size() );

          line.replace( pos, search.size(), "\033[1;32m" + found + "\033[0m" );
          pos += 15 + search.size();
        }

        if ( matched )
        {
          output += line + "\n";
        }
      }

      if ( !output.empty() )
      {
        std::cout << "[i] found match in command \033[1;34m" << command.first << "\033[0m" << std::endl << output << std::endl << std::endl;
      }
    }
  }

  void print_commands()
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
  }

private:
  std::string search;
};

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
