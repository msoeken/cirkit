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
 * @file readline.hpp
 *
 * @brief Readline interfaces
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string/trim.hpp>

#include <alice/command.hpp>

#if defined USE_READLINE

#include <readline/readline.h>
#include <readline/history.h>

namespace alice
{

class readline
{
public:
  readline( const environment::ptr& env )
    : env( env )
  {
    instance = this;

    for ( const auto& p : env->commands )
    {
      command_names.push_back( p.first );
    }
    rl_attempted_completion_function = readline::readline_completion_s;
  }

  bool read_command_line( const std::string& prefix, std::string& line )
  {
    auto * cline = ::readline( prefix.c_str() );

    /* something went wrong? */
    if ( !cline )
    {
      return false;
    }

    line = cline;
    boost::trim( line );
    free( cline );

    return true;
  }

  void add_to_history( const std::string& line )
  {
    add_history( line.c_str() );
  }

private:
  static char ** readline_completion_s( const char* text, int start, int end )
  {
    if ( start == 0 )
    {
      return rl_completion_matches( text, []( const char* text, int state ) { return instance->command_iterator( text, state ); } );
    }
    else
    {
      return nullptr;
    }
  }

  char * command_iterator( const char * text, int state )
  {
    static std::vector<std::string>::const_iterator it;

    if ( state == 0 )
    {
      it = command_names.begin();
    }

    while ( it != command_names.end() )
    {
      const auto& name = *it++;
      if ( name.find( text ) != std::string::npos )
      {
        char * completion = new char[name.size()];
        strcpy( completion, name.c_str() );
        return completion;
      }
    }

    return nullptr;
  }

private:
  const environment::ptr& env;

  static readline*         instance;
  std::vector<std::string> command_names;
};

readline* readline::instance = nullptr;

}

#elif defined USE_LINENOISE

#include <linenoise.h>

namespace alice
{

class readline
{
public:
  readline( const environment::ptr& env )
    : env( env )
  {
    instance = this;

    for ( const auto& p : env->commands )
    {
      command_names.push_back( p.first );
    }
    linenoiseSetCompletionCallback( readline::readline_completion_s );
  }

  bool read_command_line( const std::string& prefix, std::string& line )
  {
    auto * cline = linenoise( prefix.c_str() );

    /* something went wrong? */
    if ( !cline )
    {
      return false;
    }

    line = cline;
    boost::trim( line );
    free( cline );

    return true;
  }

  void add_to_history( const std::string& line )
  {
    linenoiseHistoryAdd( line.c_str() );
  }

private:
  static void readline_completion_s( const char *buf, linenoiseCompletions *lc )
  {
    for ( const auto& name : instance->command_names )
    {
      if ( name.find( buf ) != std::string::npos )
      {
        linenoiseAddCompletion( lc, name.c_str() );
      }
    }
  }

private:
  const environment::ptr& env;

  static readline*         instance;
  std::vector<std::string> command_names;
};

readline* readline::instance = nullptr;

}

#else

namespace alice
{

class readline
{
public:
  readline( const environment::ptr& env )
    : env( env )
  {
  }

  bool read_command_line( const std::string& prefix, std::string& line )
  {
    std::cout << prefix;
    std::flush(std::cout);
    if( !getline( std::cin, line ) ) {
      return false;
    }

    boost::trim( line );
    return true;
  }

  void add_to_history( const std::string& line )
  {
  }

private:
  const environment::ptr& env;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
