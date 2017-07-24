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

#include "stg_map_shannon.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <reversible/functions/add_gates.hpp>
#include <reversible/synthesis/lhrs/stg_map_luts.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

class stg_map_shannon_impl
{
public:
  stg_map_shannon_impl( circuit& circ, const gia_graph& function,
                        const std::vector<unsigned>& line_map,
                        const std::vector<unsigned>& ancillas,
                        const stg_map_shannon_params& params,
                        stg_map_shannon_stats& stats )
    : circ( circ ),
      function( function ),
      line_map( line_map ),
      ancillas( ancillas ),
      params( params ),
      stats( stats )
  {
  }

  void run()
  {
    /* check whether function should be mapped with LUTs based approach */
    const auto num_inputs = function.num_inputs();
    if ( num_inputs <= params.map_luts_params->max_cut_size || /* function is small, use pre-computed approach */
         ( line_map.size() + ancillas.size() == circ.lines() ) ) /* there are no dirty ancillas */
    {
      stg_map_luts( circ, function, line_map, ancillas, *params.map_luts_params, *stats.map_luts_stats );
      return;
    }

    /* decompose at variable var (in the example it's 0) */
    auto var = 0;

    /* compute cofactors */
    const auto gia0 = function.cofactor( var, false );
    const auto gia1 = function.cofactor( var, true );

    const auto dirty = get_dirty_ancilla();

    std::cout << dirty << " " << circ.lines() << std::endl;

    /* line map for co-factored circuits to not contain variable var and map their target to dirty */
    auto sub_line_map = line_map;
    //sub_line_map.erase( sub_line_map.begin() + var ); /* erase control line for var */
    sub_line_map.back() = dirty;                      /* modify target line */

    /* compute circuits for co-factors */
    auto circ0 = get_fast_circuit();
    auto circ1 = get_fast_circuit();

    stg_map_luts( circ0, gia0, sub_line_map, ancillas, *params.map_luts_params, *stats.map_luts_stats );
    stg_map_luts( circ1, gia1, sub_line_map, ancillas, *params.map_luts_params, *stats.map_luts_stats );

    /* put everything together */
    append_circuit_fast( circ0 );
    append_toffoli( circ )( make_var( dirty, true ), make_var( var, false ) )( line_map.back() );
    append_circuit_fast( circ0 );
    append_toffoli( circ )( make_var( dirty, true ), make_var( var, false ) )( line_map.back() );

    append_circuit_fast( circ1 );
    append_toffoli( circ )( make_var( dirty, true ), make_var( var, true ) )( line_map.back() );
    append_circuit_fast( circ1 );
    append_toffoli( circ )( make_var( dirty, true ), make_var( var, true ) )( line_map.back() );
  }

private:
  inline circuit get_fast_circuit() const
  {
    standard_circuit c;
    c.lines = circ.lines();
    return c;
  }

  inline void append_circuit_fast( const circuit& src )
  {
    auto& dest_s = boost::get<standard_circuit>( static_cast<circuit_variant&>( circ ) );
    const auto& src_s = boost::get<standard_circuit>( static_cast<const circuit_variant&>( src ) );

    boost::push_back( dest_s.gates, src_s.gates );
  }

  unsigned get_dirty_ancilla()
  {
    boost::dynamic_bitset<> indexes( circ.lines() );
    std::for_each( line_map.begin(), line_map.end(), [&indexes]( unsigned index ) { indexes.set( index ); } );
    std::for_each( ancillas.begin(), ancillas.end(), [&indexes]( unsigned index ) { indexes.set( index ); } );

    return ( ~indexes ).find_first();
  }

private:
  circuit& circ;
  const gia_graph& function;
  const std::vector<unsigned>& line_map;
  const std::vector<unsigned>& ancillas;
  const stg_map_shannon_params& params;
  stg_map_shannon_stats& stats;
};

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

void stg_map_shannon( circuit& circ, const gia_graph& function,
                      const std::vector<unsigned>& line_map,
                      const std::vector<unsigned>& ancillas,
                      const stg_map_shannon_params& params,
                      stg_map_shannon_stats& stats )
{
  stg_map_shannon_impl impl( circ, function, line_map, ancillas, params, stats );
  impl.run();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
