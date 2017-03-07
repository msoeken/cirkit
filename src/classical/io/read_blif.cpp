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

#include "read_blif.hpp"

#include <unordered_map>

#include <boost/algorithm/string/predicate.hpp>

#include <core/utils/string_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

lut_graph_t read_blif( const std::string& filename, bool store_cubes )
{
  lut_graph_t g;
  auto name = get( boost::vertex_name, g );
  auto type = get( boost::vertex_lut_type, g );
  auto func = get( boost::vertex_lut, g );

  const auto gnd = add_vertex( g );
  name[gnd] = "gnd";
  type[gnd] = lut_type_t::gnd;

  const auto vdd = add_vertex( g );
  name[vdd] = "vdd";
  type[vdd] = lut_type_t::vdd;

  std::unordered_map<std::string, lut_vertex_t> name_to_node;
  std::vector<std::string> faninout;
  std::string outputs;

  enum class pla_type_t { none, on, off };
  auto pla_type = pla_type_t::none;
  tt f( 1u );
  std::string cubes;

  foreach_line_in_file_escape( filename, [&, gnd, vdd, store_cubes]( const std::string& line ) {
      /* empty line or comment */
      if ( line.empty() || line[0u] == '#' ) return true;

      if ( boost::starts_with( line, ".model" ) )
      {
        /* skip */
      }
      else if ( boost::starts_with( line, ".inputs" ) )
      {
        foreach_string( line.substr( 8u ), " ", [&g, &name, &type, &name_to_node]( const std::string& str ) {
            auto v = add_vertex( g );
            name[v] = str;
            type[v] = lut_type_t::pi;
            name_to_node[str] = v;
          } );
      }
      else if ( boost::starts_with( line, ".outputs" ) )
      {
        outputs = line.substr( 9u );
      }
      else if ( boost::starts_with( line, ".names" ) || boost::starts_with( line, ".end" ) )
      {
        /* we need to add a node for the previous .names command */
        if ( !faninout.empty() )
        {
          auto it_out = name_to_node.find( faninout.back() );
          if ( it_out != name_to_node.end() && !func[it_out->second].empty() )
          {
            std::cout << "[w] duplicate node " << faninout.back() << std::endl;
          }
          else if ( faninout.size() == 1u )
          {
            name_to_node[faninout.back()] = f[0] ? vdd : gnd;
          }
          else
          {
            lut_vertex_t v;
            if ( it_out == name_to_node.end() )
            {
              v = add_vertex( g );
              name[v] = faninout.back();
              type[v] = lut_type_t::internal;
              name_to_node[faninout.back()] = v;
            }
            else
            {
              v = it_out->second;
              assert( name[v] == faninout.back() );
              assert( type[v] == lut_type_t::internal );
            }

            func[v] = store_cubes ? cubes : tt_to_hex( f );

            for ( auto i = 0u; i < faninout.size() - 1; ++i )
            {
              lut_vertex_t tgt;
              const auto it = name_to_node.find( faninout[i] );
              if ( it == name_to_node.end() )
              {
                /* precreate node */
                tgt = add_vertex( g );
                name[tgt] = faninout[i];
                type[tgt] = lut_type_t::internal;
                name_to_node[faninout[i]] = tgt;
              }
              else
              {
                tgt = it->second;
              }

              add_edge( v, tgt, g );
            }
          }
        }

        if ( line[1u] == 'e' ) return false;

        faninout.clear();
        split_string( faninout, line.substr( 7u ), " " );

        f = tt( 1u << ( faninout.size() - 1 ) );
        cubes.clear();
        pla_type = pla_type_t::none;
      }
      else if ( faninout.size() == 1 )
      {
        if ( line == "1" )
        {
          if ( store_cubes )
          {
            cubes = line;
          }
          else
          {
            f = tt( 1u, 1u );
          }
        }
      }
      else if ( store_cubes )
      {
        cubes += line + "\n";
      }
      else
      {
        const auto pair = split_string_pair( line, " " );
        const auto p = pair.first;
        assert( p.size() + 1 == faninout.size() );

        switch ( pla_type )
        {
        case pla_type_t::none:
          pla_type = ( pair.second == "1" ) ? pla_type_t::on : pla_type_t::off;
          if ( pla_type == pla_type_t::off )
          {
            f.flip();
          }
          break;
        case pla_type_t::on:   assert( pair.second == "1" ); break;
        case pla_type_t::off:  assert( pair.second == "0" ); break;
        }

        auto cube = ( pla_type == pla_type_t::on ) ? ~tt( 1 << p.size() ) : tt( 1 << p.size() );
        for ( auto i = 0u; i < p.size(); ++i )
        {
          if ( p[i] == '-' ) continue;
          auto v = ( p[i] == '0' ) != ( pla_type == pla_type_t::off ) ? ~tt_nth_var( i ) : tt_nth_var( i );
          if ( p.size() < 6 )
          {
            tt_shrink( v, p.size() );
          }
          else
          {
            tt_align( v, cube );
          }

          if ( pla_type == pla_type_t::on )
          {
            cube &= v;
          }
          else
          {
            cube |= v;
          }
        }

        if ( pla_type == pla_type_t::on )
        {
          f |= cube;
        }
        else
        {
          f &= cube;
        }
      }

      return true;
    } );

  foreach_string( outputs, " ", [&g, &name, &type, &name_to_node]( const std::string& str ) {
      auto v = add_vertex( g );
      name[v] = str;
      type[v] = lut_type_t::po;

      add_edge( v, name_to_node.at( str ), g );
    } );

  return g;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
