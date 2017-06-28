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
 * @file cirkit_command.hpp
 *
 * @brief CirKit command
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CIRKIT_COMMAND_HPP
#define CIRKIT_COMMAND_HPP

#include <alice/command.hpp>

#include <core/properties.hpp>

namespace cirkit
{

using namespace alice;

class cirkit_command : public command
{
public:
  cirkit_command( const environment::ptr& env, const std::string& caption, const std::string& publications = std::string() );

  virtual bool run( const std::vector<std::string>& args );

  log_opt_t log() const;

protected:
  /* pre-defined options */
  inline void be_verbose() { opts.add_options()( "verbose,v", "be verbose" ); }
  inline void be_verbose( bool* v ) { opts.add_options()( "verbose,v", boost::program_options::bool_switch( v ), "be verbose" ); }
  inline bool is_verbose() const { return is_set( "verbose" ); }

  inline void add_new_option( bool with_short = true )
  {
    auto option = std::string( "new" );
    if ( with_short )
    {
      option += ",n";
    }
    opts.add_options()( option.c_str(), "create new store entry" );
  }

  template<typename Store>
  inline void extend_if_new( Store& store )
  {
    if ( store.empty() || is_set( "new" ) )
    {
      store.extend();
    }
  }

  /* get settings with often-used pre-defined options */
  properties::ptr make_settings() const;

  void print_runtime( const std::string& key = "runtime", const std::string& label = std::string() ) const;
  void print_runtime( double runtime, const std::string& label = std::string() ) const;

protected:
  properties::ptr statistics;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
