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

#include "aig_support.hpp"

#include <boost/dynamic_bitset.hpp>

#include <classical/functions/simulate_aig.hpp>

namespace cirkit
{

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

class aig_structural_support_simulator : public aig_simulator<boost::dynamic_bitset<>>
{
public:
  aig_structural_support_simulator( unsigned num_inputs ) : num_inputs( num_inputs ) {}

  boost::dynamic_bitset<> get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    boost::dynamic_bitset<> b( num_inputs );
    b.set( pos );
    return b;
  }

  boost::dynamic_bitset<> get_constant() const
  {
    return boost::dynamic_bitset<>( num_inputs );
  }

  boost::dynamic_bitset<> invert( const boost::dynamic_bitset<>& v ) const
  {
    return v;
  }

  boost::dynamic_bitset<> and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const
  {
    return v1 | v2;
  }

private:
  unsigned num_inputs;
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

support_map_t aig_structural_support( const aig_graph& aig, properties::ptr settings, properties::ptr statistics )
{
  return simulate_aig( aig, aig_structural_support_simulator( boost::get_property( aig, boost::graph_name ).inputs.size() ), settings, statistics );
}


boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, unsigned num_pis )
{
  boost::dynamic_bitset<> support( num_pis );

  auto pos = ( po * num_pis ) << 1u;

  for ( auto i = 0u; i < num_pis; ++i )
  {
    if ( !u[pos] || !u[pos + 1] )
    {
      support.set( i );
    }
    pos += 2u;
  }

  return support;
}

boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, const aig_graph_info& info )
{
  return get_functional_support( u, po, info.inputs.size() );
}

boost::dynamic_bitset<> get_functional_support( const boost::dynamic_bitset<>& u, unsigned po, const aig_graph& aig )
{
  return get_functional_support( u, po, aig_info( aig ).inputs.size() );
}

support_map_t aig_functional_support( const aig_graph& aig )
{
  const auto& info = aig_info( aig );
  assert( !info.unateness.empty() );

  support_map_t result;
  for ( auto j = 0u; j < info.outputs.size(); ++j )
  {
    result[info.outputs[j].first] = get_functional_support( info.unateness, j, info.inputs.size() );
  }

  return result;
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
