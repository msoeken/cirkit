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
 * @file gen_trans_arith.hpp
 *
 * @brief Generate transparent arithmetic circuits
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_GEN_TRANS_ARITH_COMMAND_HPP
#define CLI_GEN_TRANS_ARITH_COMMAND_HPP

#include <string>
#include <vector>

#include <core/cli/cirkit_command.hpp>

namespace cirkit
{

class gen_trans_arith_command : public cirkit_command
{
public:
  gen_trans_arith_command( const environment::ptr& env );

protected:
  rules_t validity_rules() const;
  bool execute();

public:
  log_opt_t log() const;

private:
  std::string filename;
  unsigned seed;
  unsigned bitwidth           = 8u;
  unsigned min_words          = 3u;
  unsigned max_words          = 10u;
  unsigned max_fanout         = 4u;
  unsigned max_rounds         = 4u;
  std::string operators       = "+ - * /";
  std::string mux_types       = "MO";
  unsigned mux_prob           = 40u;
  unsigned new_ctrl_prob      = 30u;
  std::string word_pattern    = "w%d";
  std::string control_pattern = "c%d";
  std::string module_name     = "trans_arith";
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
