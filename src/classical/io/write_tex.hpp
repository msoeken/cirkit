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
 * @file write_tex.hpp
 *
 * @brief Write AIG to TEX
 *
 * @author Heinz Riener
 * @since  2.0
 */

#if ADDON_GRAPHVIZ

#ifndef WRITE_TEX_HPP
#define WRITE_TEX_HPP

#include <classical/aig.hpp>

#include <string>

namespace cirkit
{

/**
 * @brief Writes TikZ/TeX code that represents the AIG as circuit.
 *
 * Writes Tikz/TeX code that represents the AIG as circuit.  The
 * layout is done by instantiating a layout algorithm from GraphViz.
 *
 * The produced code is not self-contained. Start from the following
 * example:
 *   \documentclass{standalone}
 *
 *   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *   \usepackage{tikz}
 *   \usetikzlibrary{circuits.logic.US,positioning,arrows,shapes}
 *   \usepackage{circuitikz}
 *
 *   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *   \begin{document}
 *   \input{aig.tex}
 *   \end{document}
 *
 * @since  2.0
 */
void write_tex( const aig_graph& aig, const std::string& layout_algorithm, std::ostream& os, const bool fill_sym_table = false );

}

#endif

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
