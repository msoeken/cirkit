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
 * @file simulate.hpp
 *
 * @brief Simulates an AIG
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_SIMULATE_COMMAND_HPP
#define CLI_SIMULATE_COMMAND_HPP

#include <string>
#include <vector>

#include <core/utils/bdd_utils.hpp>
#include <classical/cli/aig_mig_command.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

class simulate_command : public aig_mig_command
{
public:
  simulate_command( const environment::ptr& env );

protected:
  rules_t validity_rules() const;
  bool execute_aig();
  bool execute_mig();

private:
  rule_t one_simulation_method() const;
  rule_t check_pattern_size() const;

  template<typename S>
  void store( const S& element )
  {
    if ( env->has_store<S>() && is_set( "store" ) )
    {
      auto& store = env->store<S>();

      store.extend();
      store.current() = element;
    }
  }

public:
  log_opt_t log() const;

private:
  std::string pattern;
  std::string assignment;

  std::vector<std::string> tts;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
