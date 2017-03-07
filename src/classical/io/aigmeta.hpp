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
 * @file aigmeta.hpp
 *
 * @brief AIG metadata files
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef AIGMETA_HPP
#define AIGMETA_HPP

#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace cirkit
{

struct aigmeta_port
{
  unsigned id;
  std::string name;
};

struct aigmeta_box
{
  unsigned id;
  std::string oper_type;
  std::vector<aigmeta_port> ports;
};

struct aigmeta_bundle
{
  unsigned id;
  std::string name;
  std::list<unsigned> literals;
};

struct aigmeta
{
  std::string dut;
  std::vector<aigmeta_box> boxes;
  std::vector<aigmeta_bundle> bundles;
};

void read_aigmeta( aigmeta& meta, const std::string& filename );
void write_aigmeta( const aigmeta& meta, const std::string& filename );

/**
 * @brief Iterates through bundles that contain literal
 *
 * @param meta    AIG meta-data
 * @param literal Literal to look for
 * @param exact   If false, then also literal + 1 is a valid match
 * @param f       Functor (first param is found bundle,
 *                second param is the literal that was found,
 *                and third param is the position of that literal in the bundle)
 */
void foreach_bundle_with_literal( const aigmeta& meta, unsigned literal, bool exact, const std::function<void(const aigmeta_bundle&, unsigned, unsigned)>& f );

std::ostream& operator<<( std::ostream& os, const aigmeta& meta );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
