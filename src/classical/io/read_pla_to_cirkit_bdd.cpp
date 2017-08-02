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
#include "read_pla_to_cirkit_bdd.hpp"

#include <core/io/pla_processor.hpp>
#include <core/io/pla_parser.hpp>

#include <classical/dd/bdd.hpp>

#include <boost/format.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/timer.hpp>

namespace cirkit
{

namespace
{

class from_bdd_pla_processor : public pla_processor
{
public:
  explicit from_bdd_pla_processor( unsigned log_max_objs, bool verbose )
    : m_log_max_objs( log_max_objs  ),
      m_verbose( verbose )
    {}

    void on_comment(const std::string &comment)
    {
    }

    void on_num_inputs(unsigned num_inputs)
    {
      m_inputs = num_inputs;
      m_timer.restart();
    }

    void on_num_outputs(unsigned num_outputs) final
    {
      m_outputs = num_outputs;
    }

    void on_num_products(unsigned num_products) final
    {
    }

    void on_input_labels(const std::vector<std::string> &input_labels) final
    {
      m_inputNames = input_labels;
    }

    void on_output_labels(const std::vector<std::string> &output_labels) final
    {
      m_outputNames = output_labels;
    }

    void on_end() final
    {
      if ( true /* m_verbose */ )
      {
        std::cout
          << boost::format ("[i] took %.2f seconds to reading pla-file into BDD.") % m_timer.elapsed()
          << std::endl;
      }
    }

    void on_type(const std::string &type)
    {
    }

    void setPortNames()
    {
      assert( m_inputNames.size() <= m_inputs );
      assert( m_outputNames.size() <= m_outputs );

      for ( auto i = 0u; i < m_inputNames.size(); ++i )
      {
        m_function->setInputName ( i, m_inputNames.at ( i ) );
      }

      for ( auto i = 0u; i < m_outputNames.size(); ++i )
      {
        m_function->setOutputName ( i, m_outputNames.at ( i ) );
      }
    }

    void initializeInputPorts()
    {
      for ( auto i = 0u; i < m_inputs; ++i )
      {
        m_function->pushInput ( i );
      }
    }

    void initializeOutputPorts(bdd_manager_ptr manager)
    {
      auto falseNode = manager->bdd_bot ();
      std::cout << falseNode << std::endl;
      for ( auto i = 0u; i < m_outputs; ++i )
      {
        m_function->setOutputVar ( i, falseNode );
      }
    }

    void initializeFunction () {
      assert ( m_inputs != 0 );
      assert ( m_outputs != 0 );

      try {
        auto function = new bdd_function (
          m_inputs, m_log_max_objs, m_verbose
        );
        m_function.reset ( function );

        initializeInputPorts();
        initializeOutputPorts( m_function->manager() );
        setPortNames();
      }
      catch ( std::exception const& e )
      {
        std::cerr << "[e] unable to create bdd_function: " << e.what() << std::endl;
        throw;
      }
    }

    void on_cube(const std::string &in, const std::string &out) final
    {
      if ( !m_function ) {
        initializeFunction ();
      }

      auto term = getBddCube ( in );

      addToOutputBdds ( term, out );
    }

    bdd_function_cptr function() const {
      return m_function;
    }

  private:
    void addToOutputBdds ( const bdd& term, const std::string& out ) {

      auto relevantValue = [] ( const char& val )
      {
        return val == '1';
      };

      for ( auto i = 0u; i < out.size(); ++i )
      {
        auto const& value = out.at( i );
        if ( relevantValue ( value ) ) {
          auto output = m_function->lookupOutput ( i );
//          std::cout << "Output: " << i  << " is relevant: " << output << std::endl;

          m_function->setOutputVar ( i, output || term );
        }
      }
    }


    bdd getBddCube ( const std::string& cube )
    {
      auto manager = m_function->manager ();

      assert ( cube.size() == m_inputs );
      bdd term = manager->bdd_top ();

      for ( auto i = 0u; i < cube.size(); ++i ) {
        auto const& val = cube.at( i );

        if ( val != '-') {
          auto input = m_function->lookupInput( i );

          if ( val == '0')
          {
            term = !input && term;
          }
          else
          {
            term = input && term;
          }
        }
      }

      if ( m_verbose )
      {
        std::cout << "[i] term result:" << term << std::endl;
      }
      return term;
    }

private:
  unsigned         m_log_max_objs;
  bool             m_verbose;
  bdd_function_ptr m_function;
  unsigned         m_inputs;
  unsigned         m_outputs;

  boost::timer     m_timer;

  std::vector<std::string> m_inputNames;
  std::vector<std::string> m_outputNames;
};

} // anonymous namespace


bdd_function::bdd_function(
      unsigned nvars
    , unsigned log_max_objs
    , bool verbose
) : m_manager ( bdd_manager::create ( nvars, log_max_objs, verbose ) )
{
}

bdd_function::bdd_function(bdd_manager_ptr manager)
  : m_manager ( manager )
{
}

bdd_function::~bdd_function()
{
}

void bdd_function::pushInput ( unsigned index ) {
  assert ( m_outputs.empty() );
  auto bdd = m_manager->bdd_var( index );
  m_inputs.emplace ( index, bdd );
}

bdd::const_param_ref bdd_function::lookupInput(unsigned index) const
{
  auto iter = m_inputs.find ( index );
  assert ( iter != m_inputs.end() );
  return iter->second;
}

bdd::const_param_ref bdd_function::lookupOutput(unsigned index) const
{
  auto iter = m_outputs.find ( index );
  assert ( iter != m_outputs.end() );
  return iter->second;
}

void bdd_function::setOutputVar(unsigned index, const bdd &var)
{
  auto iter = m_outputs.find ( index );
  if ( iter != m_outputs.end() ) {
    iter->second = var;
  }
  else {
    m_outputs.emplace ( index, var );
  }
}

bdd_manager_ptr bdd_function::manager() const
{
  return m_manager;
}

void bdd_function::setInputName(unsigned index, const std::string &name)
{
  m_inputNames.emplace ( index, name );
}

void bdd_function::setOutputName(unsigned index, const std::string &name)
{
  m_outputNames.emplace ( index, name );
}

bdd_function_cptr read_pla_into_cirkit_bdd_job( boost::filesystem::ifstream& stream, unsigned log_max_objs, bool verbose )
{
  assert ( stream );

  from_bdd_pla_processor processor ( log_max_objs, verbose );
  try {
    pla_parser ( stream, processor );
  } catch ( std::exception const& e ) {
    std::cerr << "[e] unable to parse PLA file: " << e.what() << std::endl;
    return bdd_function_ptr ();
  }

  return processor.function();
}

bdd_function_cptr read_pla_into_cirkit_bdd( const boost::filesystem::path &filename,
                                            const properties::ptr& settings )
{
  assert ( !filename.empty() );

  auto log_max_objs = get( settings, "log_max_objs", 24u );
  auto verbose      = get( settings, "verbose",      false );

  boost::filesystem::ifstream stream( filename );
  if ( !stream ) {
    std::cerr << "[e] unable to open file " << filename << std::endl;
    return bdd_function_ptr();
  }

  return read_pla_into_cirkit_bdd_job( stream, log_max_objs, verbose );
}

std::vector<std::string> bdd_function::input_labels() const { 
  std::vector<std::string> input_vars;
  for (auto const &itr: m_inputNames)
    input_vars.push_back(itr.second);
  return input_vars; 
}
std::vector<std::string> bdd_function::output_labels() const{ 
  std::vector<std::string> output_vars;
  for (auto const &itr: m_outputNames)
    output_vars.push_back(itr.second);
  return output_vars; 
}

} // namespace cirkit

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
