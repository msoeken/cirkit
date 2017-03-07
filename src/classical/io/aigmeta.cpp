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

#include "aigmeta.hpp"

#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

namespace cirkit
{

using namespace boost::assign;
using boost::property_tree::ptree;

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

inline void read_port_def( aigmeta_box& box, const ptree& pt )
{
  aigmeta_port port;

  port.id = pt.get<unsigned>( "bundle_id" );
  port.name = pt.get<std::string>( "port_name" );

  box.ports += port;
}

inline void read_box_def( aigmeta& meta, const ptree& pt )
{
  aigmeta_box box;

  box.id = pt.get<unsigned>( "output_bundle_id" );
  box.oper_type = pt.get<std::string>( "oper_type" );

  auto children = pt.get_child_optional( "port_def" );
  if ( children )
  {
    for ( const auto& port_def : *children )
    {
      read_port_def( box, port_def.second );
    }
  }

  meta.boxes += box;
}

inline void read_bundle_def( aigmeta& meta, const ptree& pt )
{
  using boost::adaptors::transformed;

  aigmeta_bundle bundle;

  bundle.id = pt.get<unsigned>( "bundle_id" );
  bundle.name = pt.get<std::string>( "name" );
  boost::push_back( bundle.literals, pt.get_child( "litx" ) | transformed( []( const ptree::value_type& v ) { return boost::lexical_cast<unsigned>( v.second.data() ); } ) );

  meta.bundles += bundle;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void read_aigmeta( aigmeta& meta, const std::string& filename )
{
  ptree pt;

  read_json( filename, pt );

  meta.dut = pt.get<std::string>( "dut" );

  for ( const auto& box_def : pt.get_child( "box_def" ) )
  {
    read_box_def( meta, box_def.second );
  }

  for ( const auto& bundle_def : pt.get_child( "bundle_def" ) )
  {
    read_bundle_def( meta, bundle_def.second );
  }
}

void write_aigmeta( const aigmeta& meta, const std::string& filename )
{
  using boost::adaptors::transformed;

  std::filebuf fb;
  fb.open( filename.c_str(), std::ios::out );
  std::ostream os( &fb );

  os << "{" << std::endl;
  os << "    \"dut\": \"" << meta.dut << "\"," << std::endl;

  /* boxes */
  os << "    \"box_def\": [" << std::endl;

  bool first = true;
  for ( const auto& box : meta.boxes )
  {
    if ( !first )
    {
      os << "," << std::endl;
    }
    first = false;
    os << "        {" << std::endl
       << "            \"output_bundle_id\": " << box.id << "," << std::endl
       << "            \"oper_type\": \"" << box.oper_type << "\"," << std::endl
       << "            \"port_def\": []" << std::endl
       << "        }";
  }

  os << std::endl
     << "    ]," << std::endl;

  /* bundles */
  os << "    \"bundle_def\": [" << std::endl;

  first = true;
  for ( const auto& bundle : meta.bundles )
  {
    if ( !first )
    {
      os << "," << std::endl;
    }
    first = false;
    os << "        {" << std::endl
       << "            \"name\": \"" << bundle.name << "\"," << std::endl
       << "            \"bundle_id\": " << bundle.id << "," << std::endl
       << "            \"litx\": [ " << boost::join( bundle.literals | transformed( []( unsigned l ) { return boost::lexical_cast<std::string>( l ); } ), ", " ) << " ]" << std::endl
       << "        }";
  }
  os << std::endl
     << "    ]" << std::endl
     << "}" << std::endl;

  fb.close();
}

std::ostream& operator<<( std::ostream& os, const aigmeta& meta )
{
  os << "DUT:      " << meta.dut << std::endl
     << "#Boxes:   " << meta.boxes.size() << std::endl
     << "#Bundles: " << meta.bundles.size() << std::endl;

  return os;
}

void foreach_bundle_with_literal( const aigmeta& meta, unsigned literal, bool exact, const std::function<void(const aigmeta_bundle&, unsigned, unsigned)>& f )
{
  for ( const auto& bundle : meta.bundles )
  {
    for ( auto it = bundle.literals.begin(); it != bundle.literals.end(); ++it )
    {
      if ( *it == literal || ( !exact && *it == literal + 1u ) )
      {
        f( bundle, *it, std::distance( bundle.literals.begin(), it ) );
      }
    }
  }
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
