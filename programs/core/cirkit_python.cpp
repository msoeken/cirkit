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
 * @author Mathias Soeken
 */

#include <cstdarg>
#include <memory>

#include <alice/alice.hpp>
#include <alice/python.hpp>

#include <core/cli/stores.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/cli/commands/testbdd.hpp>
#include <core/utils/bdd_utils.hpp>

#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>

#include <classical/cli/commands/abc.hpp>
#include <classical/cli/commands/bool_complex.hpp>
#include <classical/cli/commands/blif_to_bench.hpp>
#include <classical/cli/commands/comb_approx.hpp>
#include <classical/cli/commands/compress.hpp>
#include <classical/cli/commands/cone.hpp>
#include <classical/cli/commands/contrinv.hpp>
#include <classical/cli/commands/cuts.hpp>
#include <classical/cli/commands/dectest.hpp>
#include <classical/cli/commands/demo.hpp>
#include <classical/cli/commands/depth.hpp>
#include <classical/cli/commands/dsop.hpp>
#include <classical/cli/commands/expr.hpp>
#include <classical/cli/commands/feather.hpp>
#include <classical/cli/commands/gen_npn_circuit.hpp>
#include <classical/cli/commands/gen_trans_arith.hpp>
#include <classical/cli/commands/isop.hpp>
#include <classical/cli/commands/memristor.hpp>
#include <classical/cli/commands/mig_rewrite.hpp>
#include <classical/cli/commands/migfh.hpp>
#include <classical/cli/commands/npn.hpp>
#include <classical/cli/commands/output_noise.hpp>
#include <classical/cli/commands/permmask.hpp>
#include <classical/cli/commands/plim.hpp>
#include <classical/cli/commands/propagate.hpp>
#include <classical/cli/commands/read_sym.hpp>
#include <classical/cli/commands/rename.hpp>
#include <classical/cli/commands/satnpn.hpp>
#include <classical/cli/commands/shuffle.hpp>
#include <classical/cli/commands/simgraph.hpp>
#include <classical/cli/commands/simulate.hpp>
#include <classical/cli/commands/strash.hpp>
#include <classical/cli/commands/support.hpp>
#include <classical/cli/commands/tt.hpp>
#include <classical/cli/commands/unate.hpp>
#include <classical/cli/commands/xmgmerge.hpp>
#include <classical/cli/commands/worstcase.hpp>

#include <Python.h>

using namespace cirkit;
using namespace std::literals;
namespace hana = boost::hana;

using cirkit_store_types = boost::mpl::vector<aig_graph, mig_graph, xmg_graph, simple_fanout_graph_t, tt, bdd_function_t, expression_t::ptr, counterexample_t>;

#define STORE_TYPES cirkit_store_types

#include <addon_commands.hpp> // auto-generated file from CMake

/******************************************************************************
 * Environment API                                                            *
 ******************************************************************************/

using namespace alice;

template<typename Cmd>
static PyObject* python_wrapper( PyObject *self, PyObject *args, PyObject *keywds );

template<class... S>
class cirkit_api_main : public python_api_main<cirkit_api_main<S...>, S...>
{
public:
  cirkit_api_main() : python_api_main<cirkit_api_main<S...>, S...>( this, "cirkit", "CirKit Python API" ) {}

  auto commands() const
  {
    const auto env = this->env;

    return hana::make_map(
      entry<convert_command<S...>>( "convert"s, env ),
      entry<ps_command<S...>>( "ps"s, env ),
      entry<store_command<S...>>( "store"s, env ),

      read_entry<io_aiger_tag_t, S...>( "aiger"s, env ),
      write_entry<io_aiger_tag_t, S...>( "aiger"s, env ),

      entry<cone_command>( "cone"s, env ),
      entry<help_command>( "help"s, env ),
      entry<tt_command>( "tt"s, env )
    );
  }

  template<typename Cmd>
  auto wrapper() const
  {
    return (PyCFunction)python_wrapper<Cmd>;
  }
};

template<typename API, typename T> struct extend;
#define ADD_EXTENDER( cls )            \
  template<typename T, typename... Ts> \
  struct extend<cls<Ts...>, T>         \
  {                                    \
    using type = cls<Ts..., T>;        \
  };
ADD_EXTENDER(cirkit_api_main)
template<typename T>
using extend_t = typename boost::mpl::fold<STORE_TYPES, T, extend<boost::mpl::_1, boost::mpl::_2>>::type;

extend_t<cirkit_api_main<>> api;

/******************************************************************************
 * Python API                                                                 *
 ******************************************************************************/

template<typename Cmd>
static PyObject* python_wrapper( PyObject *self, PyObject *args, PyObject *keywds )
{
  const auto name = hana::first( api.commands()[hana::type_c<Cmd>] );
  const auto it = api.env->commands.find( name );
  if ( it != api.env->commands.end() )
  {
    const auto& cmd = it->second;
    return python_run_command( name, cmd, args, keywds );
  }
  else
  {
    std::cerr << "[e] unknown command" << std::endl;

    Py_RETURN_NONE;
  }
}

PyMODINIT_FUNC PyInit_cirkit( void )
{
  return api.create_module();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
