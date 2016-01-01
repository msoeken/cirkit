/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
