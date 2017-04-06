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
 * @file python.hpp
 *
 * @brief Utility functions for the Python API
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#pragma once

#include <Python.h>

#include <alice/command.hpp>

#include <string>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/hana/at_key.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>

using namespace std::literals;
namespace hana = boost::hana;

namespace alice
{

struct log_var_python_visitor : public boost::static_visitor<PyObject*>
{
public:
  PyObject* operator()( const std::string& s ) const
  {
    const auto value = PyUnicode_FromString( s.c_str() );
    Py_INCREF( value );
    return value;
  }

  PyObject* operator()( int i ) const
  {
    const auto value = PyLong_FromLong( i );
    Py_INCREF( value );
    return value;
  }

  PyObject* operator()( unsigned i ) const
  {
    const auto value = PyLong_FromUnsignedLong( i );
    Py_INCREF( value );
    return value;
  }

  PyObject* operator()( uint64_t i ) const
  {
    const auto value = PyLong_FromUnsignedLong( i );
    Py_INCREF( value );
    return value;
  }

  PyObject* operator()( double d ) const
  {
    const auto value = PyFloat_FromDouble( d );
    Py_INCREF( value );
    return value;
  }

  PyObject* operator()( bool b ) const
  {
    if ( b )
    {
      Py_RETURN_TRUE;
    }
    else
    {
      Py_RETURN_FALSE;
    }
  }

  template<typename T>
  PyObject* operator()( const std::vector<T>& v ) const
  {
    const auto value = PyList_New( 0 );

    for ( const auto& element : v )
    {
      PyList_Append( value, operator()( element ) );
    }

    Py_INCREF( value );
    return value;
  }
};

PyObject* python_run_command( const std::string& name, const std::shared_ptr<command>& cmd, PyObject *args, PyObject *keywds )
{
  std::vector<std::string> pargs = {name};

  /* prepare argument string from Python keywords */
  if ( keywds )
  {
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while ( PyDict_Next( keywds, &pos, &key, &value ) )
    {
      const auto skey = std::string( PyUnicode_AsUTF8( key ) );

      if ( PyBool_Check( value ) )
      {
        if ( value == Py_True )
        {
          pargs.push_back( "--" + skey );
        }
      }
      else
      {
        pargs.push_back( "--" + skey );
        pargs.push_back( std::string( PyUnicode_AsUTF8( value ) ) );
      }
    }
  }

  cmd->run( pargs );

  const auto log = cmd->log();
  const auto dict = PyDict_New();

  if ( log )
  {
    log_var_python_visitor vis;
    for ( const auto& p : *log )
    {
      const auto value = boost::apply_visitor( vis, p.second );
      PyDict_SetItemString( dict, p.first.c_str(), value );
    }
  }

  Py_INCREF( dict );
  return dict;
}

template<class Cmd>
auto entry = []( auto name, const environment::ptr& env ) {
  return hana::make_pair( hana::type_c<Cmd>, hana::make_pair( name, std::make_shared<Cmd>( env ) ) );
};

template<class Tag, class... S>
auto read_entry = []( auto tag, const environment::ptr& env ) {
  return hana::make_pair( hana::type_c<read_io_command<Tag, S...>>, hana::make_pair( "read_"s + tag, std::make_shared<read_io_command<Tag, S...>>( env, tag ) ) );
};

template<class Tag, class... S>
auto write_entry = []( auto tag, const environment::ptr& env ) {
  return hana::make_pair( hana::type_c<write_io_command<Tag, S...>>, hana::make_pair( "write_"s + tag, std::make_shared<write_io_command<Tag, S...>>( env, tag ) ) );
};

template<class T, class... S>
class python_api_main
{
public:
  python_api_main( T* self, const std::string& module_name, const std::string& module_doc )
    : env( std::make_shared<environment>() ),
      module_name( module_name ),
      module_doc( module_doc )
  {
    [](...){}( add_store_helper<S>( env )... );

    hana::for_each( self->commands(), [this, self]( auto const& c ) {
        using Cmd = typename decltype( +hana::first( c ) )::type;

        const auto& name = hana::first( hana::second( c ) );
        const auto cmd = env->commands[name] = hana::second( hana::second( c ) );
        methods.push_back( {name.c_str(), self->template wrapper<Cmd>(), METH_VARARGS | METH_KEYWORDS, cmd->caption().c_str()} );
      } );
  }

  auto create_module()
  {
    static struct PyModuleDef cirkit_module = {
      PyModuleDef_HEAD_INIT,
      module_name.c_str(),
      module_doc.c_str(),
      -1
    };

    auto m = PyModule_Create( &cirkit_module );

    methods.push_back( {nullptr, nullptr, 0, nullptr} );

    PyModule_AddFunctions( m, &methods[0] );

    return m;
  }

public:
  std::shared_ptr<environment> env;

private:
  std::string module_name;
  std::string module_doc;
  std::vector<PyMethodDef> methods;
};

}


// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
