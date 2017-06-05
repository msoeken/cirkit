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

#include <memory>

#include <alice/alice.hpp>
#include <alice/python.hpp>

#include <core/cli/stores.hpp>
#include <core/cli/commands/bdd.hpp>
#include <core/utils/bdd_utils.hpp>
#include <classical/aig.hpp>
#include <classical/cli/stores.hpp>
#include <classical/cli/commands/expr.hpp>
#include <classical/cli/commands/abc.hpp>
#include <classical/cli/commands/cone.hpp>
#include <classical/cli/commands/exorcism.hpp>
#include <classical/cli/commands/print_io.hpp>
#include <classical/cli/commands/simulate.hpp>
#include <classical/cli/commands/tt.hpp>
#include <classical/utils/expression_parser.hpp>
#include <classical/utils/truth_table_utils.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/cli/stores.hpp>
#include <reversible/cli/commands/adding_lines.hpp>
#include <reversible/cli/commands/cbs.hpp>
#include <reversible/cli/commands/dbs.hpp>
#include <reversible/cli/commands/dxs.hpp>
#include <reversible/cli/commands/embed.hpp>
#include <reversible/cli/commands/enumerate.hpp>
#include <reversible/cli/commands/esopbs.hpp>
#include <reversible/cli/commands/exs.hpp>
#include <reversible/cli/commands/filter.hpp>
#include <reversible/cli/commands/gen_reciprocal.hpp>
#include <reversible/cli/commands/hdbs.hpp>
#include <reversible/cli/commands/is_identity.hpp>
#include <reversible/cli/commands/lhrs.hpp>
#include <reversible/cli/commands/maslov234.hpp>
#include <reversible/cli/commands/mitm.hpp>
#include <reversible/cli/commands/nct.hpp>
#include <reversible/cli/commands/pos.hpp>
#include <reversible/cli/commands/qbs.hpp>
#include <reversible/cli/commands/random_circuit.hpp>
#include <reversible/cli/commands/rec.hpp>
#include <reversible/cli/commands/required_lines.hpp>
#include <reversible/cli/commands/revsim.hpp>
#include <reversible/cli/commands/revsimp.hpp>
#include <reversible/cli/commands/rms.hpp>
#include <reversible/cli/commands/stg4.hpp>
#include <reversible/cli/commands/tbs.hpp>
#include <reversible/cli/commands/tof.hpp>
#include <reversible/cli/commands/tpar.hpp>
#include <reversible/cli/commands/unique_names.hpp>

using namespace cirkit;
using namespace std::literals;
namespace hana = boost::hana;

#define STORES circuit, binary_truth_table, tt, expression_t::ptr, bdd_function_t, rcbdd, aig_graph, xmg_graph

/******************************************************************************
 * Environment API                                                            *
 ******************************************************************************/

using namespace alice;

template<typename Cmd>
static PyObject* python_wrapper( PyObject *self, PyObject *args, PyObject *keywds );

template<class... S>
class revkit_api_main : public python_api_main<revkit_api_main<S...>, S...>
{
public:
  revkit_api_main() : python_api_main<revkit_api_main<S...>, S...>( this, "revkit", "RevKit Python API" ) {}

  auto commands() const
  {
    const auto env = this->env;

    return hana::make_map(
      entry<alias_command>( "alias"s, env ),
      entry<convert_command<S...>>( "convert"s, env ),
      entry<current_command<S...>>( "current"s, env ),
      entry<help_command>( "help"s, env ),
      entry<print_command<S...>>( "print"s, env ),
      entry<ps_command<S...>>( "ps"s, env ),
      entry<set_command>( "set"s, env ),
      entry<show_command<S...>>( "show"s, env ),
      entry<store_command<S...>>( "store"s, env ),

      read_entry<io_aiger_tag_t, S...>( "aiger"s, env ),
      read_entry<io_bench_tag_t, S...>( "bench"s, env ),
      read_entry<io_pla_tag_t, S...>( "pla"s, env ),
      read_entry<io_qc_tag_t, S...>( "qc"s, env ),
      read_entry<io_real_tag_t, S...>( "real"s, env ),
      read_entry<io_spec_tag_t, S...>( "spec"s, env ),
      read_entry<io_verilog_tag_t, S...>( "verilog"s, env ),
      write_entry<io_aiger_tag_t, S...>( "aiger"s, env ),
      write_entry<io_liquid_tag_t, S...>( "liquid"s, env ),
      write_entry<io_pla_tag_t, S...>( "pla"s, env ),
      write_entry<io_qc_tag_t, S...>( "qc"s, env ),
      write_entry<io_qcode_tag_t, S...>( "qcode"s, env ),
      write_entry<io_qpic_tag_t, S...>( "qpic"s, env ),
      write_entry<io_quipper_tag_t, S...>( "quipper"s, env ),
      write_entry<io_real_tag_t, S...>( "real"s, env ),
      write_entry<io_spec_tag_t, S...>( "spec"s, env ),
      write_entry<io_tikz_tag_t, S...>( "tikz"s, env ),
      write_entry<io_verilog_tag_t, S...>( "verilog"s, env ),

      entry<embed_command>( "embed"s, env ),
      entry<required_lines_command>( "required_lines"s, env ),

      entry<cbs_command>( "cbs"s, env ),
      entry<dbs_command>( "dbs"s, env ),
      entry<dxs_command>( "dxs"s, env ),
      entry<esopbs_command>( "esopbs"s, env ),
      entry<exs_command>( "exs"s, env ),
      entry<hdbs_command>( "hdbs"s, env ),
      entry<lhrs_command>( "lhrs"s, env ),
      entry<qbs_command>( "qbs"s, env ),
      entry<rms_command>( "rms"s, env ),
      entry<tbs_command>( "tbs"s, env ),

      entry<adding_lines_command>( "adding_lines"s, env ),
      entry<exorcism_command>( "exorcism"s, env ),
      entry<revsimp_command>( "revsimp"s, env ),
      entry<tpar_command>( "tpar"s, env ),

      entry<filter_command>( "filter"s, env ),
      entry<maslov234_command>( "mitm"s, env ),
      entry<mitm_command>( "mitm"s, env ),
      entry<nct_command>( "nct"s, env ),
      entry<pos_command>( "pos"s, env ),
      entry<stg4_command>( "stg4"s, env ),
      entry<tof_command>( "tof"s, env ),
      entry<unique_names_command>( "unique_names"s, env ),

      entry<cone_command>( "cone"s, env ),

      entry<is_identity_command>( "is_identity"s, env ),
      entry<rec_command>( "rec"s, env ),
      entry<revsim_command>( "revsim"s, env ),
      entry<simulate_command>( "simulate"s, env ),

      entry<gen_reciprocal_command>( "gen_reciprocal"s, env ),

      entry<abc_command>( "abc"s, env ),
      entry<bdd_command>( "bdd"s, env ),
      entry<enumerate_command>( "enumerate"s, env ),
      entry<expr_command>( "expr"s, env ),
      entry<print_io_command>( "print_io"s, env ),
      entry<random_circuit_command>( "random_circuit"s, env ),
      entry<tt_command>( "tt"s, env )
    );
  }

  template<typename Cmd>
  auto wrapper() const
  {
    return (PyCFunction)python_wrapper<Cmd>;
  }
};

revkit_api_main<STORES> api;

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

PyMODINIT_FUNC PyInit_revkit( void )
{
  return api.create_module();
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
