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
 * @file commands.hpp
 *
 * @brief Formal commands for CLI
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_FORMAL_COMMANDS_HPP
#define CLI_FORMAL_COMMANDS_HPP

#include <cli/commands/exact_mig.hpp>
#include <cli/commands/exact_xmg.hpp>
#include <cli/commands/xmglut.hpp>
#include <cli/commands/xmgmine.hpp>

#define CIRKIT_FORMAL_Z3_CLI_COMMANDS \
  ALICE_ADD_COMMAND( exact_mig, "Synthesis" ); \
  ALICE_ADD_COMMAND( exact_xmg, "Synthesis" ); \
  ALICE_ADD_COMMAND( xmglut, "Synthesis" );    \
  ALICE_ADD_COMMAND( xmgmine, "Synthesis" );

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
