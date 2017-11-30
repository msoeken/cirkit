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

#include "stg_partners.hpp"

namespace cirkit
{

namespace legacy
{

bool stg_partners::has_partners( int index ) const
{
  return _partner_index.find( index ) != _partner_index.end();
}

const std::vector<int>& stg_partners::partners( int index ) const
{
  return _partner_vectors[_partner_index.at( index )];
}

void stg_partners::add_partners( const std::vector<int>& partners )
{
  auto index = _partner_vectors.size();
  _partner_vectors.emplace_back( partners.begin(), partners.end() );

  for ( auto p : partners )
  {
    _partner_index[p] = index;
  }
}

stg_partners find_stg_partners( const gia_graph& gia )
{
  stg_partners partners;

  auto* xors = static_cast<abc::Gia_Man_t*>( gia )->vXors;
  if ( !xors )
  {
    return partners;
  }

  gia.init_lut_refs();

  gia.foreach_lut( [&gia, &xors, &partners]( auto index ) {
    if ( abc::Vec_IntFind( xors, index ) != -1 )
    {
      auto add = true;
      std::vector<int> indexes;

      gia.foreach_lut_fanin( index, [&gia, &add, &indexes]( auto fanin ) {
        if ( add )
        {
          if ( gia.lut_ref_num( fanin ) == 1 )
          {
            indexes.push_back( fanin );
          }
          else
          {
            add = false;
          }
        }
      } );

      if ( add )
      {
        partners.add_partners( indexes );
      }
    }
  } );

  return partners;
}
}
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
