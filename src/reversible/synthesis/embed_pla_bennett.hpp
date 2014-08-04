/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2014  University of Bremen
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
 * @file embed_pla_bennett.hpp
 *
 * @brief Embedding of an irreversible specification given as PLA using Bennett Embedding
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef EMBED_PLA_BENNETT_HPP
#define EMBED_PLA_BENNETT_HPP

#include <string>

#include <reversible/rcbdd.hpp>
#include <reversible/synthesis/synthesis.hpp>

namespace cirkit
{

  bool embed_pla_bennett( rcbdd& cf, const std::string& filename,
                          properties::ptr settings = properties::ptr(),
                          properties::ptr statistics = properties::ptr() );


  pla_embedding_func embed_pla_bennett_func( properties::ptr settings = properties::ptr( new properties() ), properties::ptr statistics = properties::ptr( new properties() ) );

}

#endif /* EMBED_PLA_BENNETT_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
