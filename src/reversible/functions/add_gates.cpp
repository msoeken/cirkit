/* RevKit (www.revkit.org)
 * Copyright (C) 2009-2015  University of Bremen
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

#include "add_gates.hpp"

#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

#include "../target_tags.hpp"

using namespace boost::assign;

namespace cirkit
{

  ////////////////////////////// class target_line_adder
  target_line_adder::target_line_adder( gate* gate ) : g( gate )
  {
  }

  gate& target_line_adder::operator()( unsigned l1 )
  {
    g->add_target( l1 );
    return *g;
  }

  gate& target_line_adder::operator()( unsigned l1, unsigned l2 )
  {
    g->add_target( l1 );
    g->add_target( l2 );
    return *g;
  }

  ////////////////////////////// class control_line_adder
  control_line_adder::control_line_adder( gate& gate ) : g( &gate )
  {
  }

  target_line_adder control_line_adder::operator()( boost::optional<variable> l1, boost::optional<variable> l2, boost::optional<variable> l3, boost::optional<variable> l4, boost::optional<variable> l5, boost::optional<variable> l6, boost::optional<variable> l7, boost::optional<variable> l8, boost::optional<variable> l9 )
  {
    if ( l1 ) g->add_control( *l1 );
    if ( l2 ) g->add_control( *l2 );
    if ( l3 ) g->add_control( *l3 );
    if ( l4 ) g->add_control( *l4 );
    if ( l5 ) g->add_control( *l5 );
    if ( l6 ) g->add_control( *l6 );
    if ( l7 ) g->add_control( *l7 );
    if ( l8 ) g->add_control( *l8 );
    if ( l9 ) g->add_control( *l9 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( boost::optional<unsigned> l1, boost::optional<unsigned> l2, boost::optional<unsigned> l3, boost::optional<unsigned> l4, boost::optional<unsigned> l5, boost::optional<unsigned> l6, boost::optional<unsigned> l7, boost::optional<unsigned> l8, boost::optional<unsigned> l9 )
  {
    if ( l1 ) g->add_control( make_var( *l1 ) );
    if ( l2 ) g->add_control( make_var( *l2 ) );
    if ( l3 ) g->add_control( make_var( *l3 ) );
    if ( l4 ) g->add_control( make_var( *l4 ) );
    if ( l5 ) g->add_control( make_var( *l5 ) );
    if ( l6 ) g->add_control( make_var( *l6 ) );
    if ( l7 ) g->add_control( make_var( *l7 ) );
    if ( l8 ) g->add_control( make_var( *l8 ) );
    if ( l9 ) g->add_control( make_var( *l9 ) );
    return target_line_adder( g );
  }

  ////////////////////////////// create_ functions

  gate& create_toffoli( gate& g, const gate::control_container& controls, unsigned target )
  {
    boost::for_each( controls, [&g](variable c) { g.add_control( c ); } );

    g.add_target( target );
    g.set_type( toffoli_tag() );

    return g;
  }

  gate& create_toffoli( gate& g, const std::vector<unsigned>& controls, unsigned target )
  {
    boost::for_each( controls, [&g](unsigned c) { g.add_control( make_var( c ) ); } );

    g.add_target( target );
    g.set_type( toffoli_tag() );

    return g;
  }

  gate& create_fredkin( gate& g, const gate::control_container& controls, unsigned target1, unsigned target2 )
  {
    boost::for_each( controls, [&g](variable c) { g.add_control( c ); } );

    g.add_target( target1 );
    g.add_target( target2 );
    g.set_type( fredkin_tag() );

    return g;
  }

  gate& create_peres( gate& g, variable control, unsigned target1, unsigned target2 )
  {
    g.add_control( control );
    g.add_target( target1 );
    g.add_target( target2 );

    peres_tag tag;
    g.set_type( tag );

    return g;
  }

  gate& create_cnot( gate& g, variable control, unsigned target )
  {
    g.add_control( control );
    g.add_target( target );
    g.set_type( toffoli_tag() );

    return g;
  }

  gate& create_not( gate& g, unsigned target )
  {
    g.add_target( target );
    g.set_type( toffoli_tag() );
    return g;
  }

  gate& create_module( gate& g, const circuit& circ, const std::string& name, const gate::control_container& controls, const gate::target_container& targets )
  {
    typedef std::map<std::string, std::shared_ptr<circuit> > map_t;
    const map_t& modules = circ.modules();
    map_t::const_iterator it = modules.find( name );
    assert( it != modules.end() );

    boost::for_each( controls, [&g](variable c) { g.add_control( c ); } );
    boost::for_each( targets, [&g](unsigned t) { g.add_target( t ); } );

    module_tag module;
    module.reference = it->second;
    module.name = name;

    g.set_type( module );
    return g;
  }

  ////////////////////////////// append_ functions

  gate& append_toffoli( circuit& circ, const gate::control_container& controls, unsigned target )
  {
    return create_toffoli( circ.append_gate(), controls, target );
  }

  gate& append_toffoli( circuit& circ, const std::vector<unsigned>& controls, unsigned target )
  {
    return create_toffoli( circ.append_gate(), controls, target );
  }

  gate& append_fredkin( circuit& circ, const gate::control_container& controls, unsigned target1, unsigned target2 )
  {
    return create_fredkin( circ.append_gate(), controls, target1, target2 );
  }

  gate& append_peres( circuit& circ, variable control, unsigned target1, unsigned target2 )
  {
    return create_peres( circ.append_gate(), control, target1, target2 );
  }

  gate& append_cnot( circuit& circ, variable control, unsigned target )
  {
    return create_cnot( circ.append_gate(), control, target );
  }

  gate& append_cnot( circuit& circ, unsigned control, unsigned target )
  {
    return create_cnot( circ.append_gate(), make_var( control ), target );
  }

  gate& append_not( circuit& circ, unsigned target )
  {
    return create_not( circ.append_gate(), target );
  }

  gate& append_module( circuit& circ, const std::string& module_name, const gate::control_container& controls, const gate::target_container& targets )
  {
    return create_module( circ.append_gate(), circ, module_name, controls, targets );
  }

  control_line_adder append_gate( circuit& circ, const boost::any& tag )
  {
    gate& g = circ.append_gate();
    g.set_type( tag );
    return control_line_adder( g );
  }

  control_line_adder append_toffoli( circuit& circ )
  {
    return append_gate( circ, toffoli_tag() );
  }

  control_line_adder append_fredkin( circuit& circ )
  {
    return append_gate( circ, fredkin_tag() );
  }

  ////////////////////////////// prepend_ functions

  gate& prepend_toffoli( circuit& circ, const gate::control_container& controls, unsigned target )
  {
    return create_toffoli( circ.prepend_gate(), controls, target );
  }

  gate& prepend_toffoli( circuit& circ, const std::vector<unsigned>& controls, unsigned target )
  {
    return create_toffoli( circ.prepend_gate(), controls, target );
  }

  gate& prepend_fredkin( circuit& circ, const gate::control_container& controls, unsigned target1, unsigned target2 )
  {
    return create_fredkin( circ.prepend_gate(), controls, target1, target2 );
  }

  gate& prepend_peres( circuit& circ, variable control, unsigned target1, unsigned target2 )
  {
    return create_peres( circ.prepend_gate(), control, target1, target2 );
  }

  gate& prepend_cnot( circuit& circ, variable control, unsigned target )
  {
    return create_cnot( circ.prepend_gate(), control, target );
  }

  gate& prepend_cnot( circuit& circ, unsigned control, unsigned target )
  {
    return create_cnot( circ.prepend_gate(), make_var( control ), target );
  }

  gate& prepend_not( circuit& circ, unsigned target )
  {
    return create_not( circ.prepend_gate(), target );
  }

  gate& prepend_module( circuit& circ, const std::string& module_name, const gate::control_container& controls, const gate::target_container& targets )
  {
    return create_module( circ.prepend_gate(), circ, module_name, controls, targets );
  }

  control_line_adder prepend_gate( circuit& circ, const boost::any& tag )
  {
    gate& g = circ.prepend_gate();
    g.set_type( tag );
    return control_line_adder( g );
  }

  control_line_adder prepend_toffoli( circuit& circ )
  {
    return prepend_gate( circ, toffoli_tag() );
  }

  control_line_adder prepend_fredkin( circuit& circ )
  {
    return prepend_gate( circ, fredkin_tag() );
  }

  ////////////////////////////// insert_ functions

  gate& insert_toffoli( circuit& circ, unsigned n, const gate::control_container& controls, unsigned target )
  {
    return create_toffoli( circ.insert_gate( n ), controls, target );
  }

  gate& insert_toffoli( circuit& circ, unsigned n, const std::vector<unsigned>& controls, unsigned target )
  {
    return create_toffoli( circ.insert_gate( n ), controls, target );
  }

  gate& insert_fredkin( circuit& circ, unsigned n, const gate::control_container& controls, unsigned target1, unsigned target2 )
  {
    return create_fredkin( circ.insert_gate( n ), controls, target1, target2 );
  }

  gate& insert_peres( circuit& circ, unsigned n, variable control, unsigned target1, unsigned target2 )
  {
    return create_peres( circ.insert_gate( n ), control, target1, target2 );
  }

  gate& insert_cnot( circuit& circ, unsigned n, variable control, unsigned target )
  {
    return create_cnot( circ.insert_gate( n ), control, target );
  }

  gate& insert_cnot( circuit& circ, unsigned n, unsigned control, unsigned target )
  {
    return create_cnot( circ.insert_gate( n ), make_var( control ), target );
  }

  gate& insert_not( circuit& circ, unsigned n, unsigned target )
  {
    return create_not( circ.insert_gate( n ), target );
  }

  gate& insert_module( circuit& circ, unsigned n, const std::string& module_name, const gate::control_container& controls, const std::vector<unsigned>& targets )
  {
    return create_module( circ.insert_gate( n ), circ, module_name, controls, targets );
  }

  control_line_adder insert_gate( circuit& circ, unsigned n, const boost::any& tag )
  {
    gate& g = circ.insert_gate( n );
    g.set_type( tag );
    return control_line_adder( g );
  }

  control_line_adder insert_toffoli( circuit& circ, unsigned n )
  {
    return insert_gate( circ, n, toffoli_tag() );
  }

  control_line_adder insert_fredkin( circuit& circ, unsigned n )
  {
    return insert_gate( circ, n, fredkin_tag() );
  }

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
