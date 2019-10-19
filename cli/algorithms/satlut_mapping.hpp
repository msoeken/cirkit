/* CirKit: A circuit toolkit
 * Copyright (C) 2017-2019  EPFL
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

#include <alice/alice.hpp>

#include <mockturtle/algorithms/satlut_mapping.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class satlut_mapping_command : public cirkit::cirkit_command<satlut_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>
{
public:
  satlut_mapping_command( environment::ptr& env ) : cirkit::cirkit_command<satlut_mapping_command, aig_t, mig_t, xag_t, xmg_t, klut_t>( env, "Performs k-LUT mapping using SAT", "apply LUT-mapping to {0}" )
  {
    add_option( "-k,--lutsize", ps.cut_enumeration_ps.cut_size, "cut size", true );
    add_option( "--lutcount", ps.cut_enumeration_ps.cut_limit, "number of cuts per node", true );
    add_option( "--conflict_limit", ps.conflict_limit, "conflict limit (0 to disable)", true );
    add_option( "--window_size", window_size, "window size (0 for no windowing)", true );
    add_flag( "--nofun", "do not compute cut functions" );
  }

  template<class Store>
  inline void execute_store()
  {
    if ( window_size > 0 )
    {
      if ( !store<Store>().current()->has_mapping() )
      {
        env->err() << "[e] windowed mapping requires network to be pre-mapped (e.g., with lut_mapping)\n";
      }
      if ( is_set( "nofun" ) )
      {
        mockturtle::satlut_mapping( *( store<Store>().current() ), window_size, ps, &st );
      }
      else
      {
        mockturtle::satlut_mapping<typename Store::element_type, true>( *( store<Store>().current() ), window_size, ps, &st );
      }
      
    }
    else
    {
      if ( is_set( "nofun" ) )
      {
        mockturtle::satlut_mapping( *( store<Store>().current() ), ps, &st );
      }
      else
      {
        mockturtle::satlut_mapping<typename Store::element_type, true>( *( store<Store>().current() ), ps, &st );
      }
    }
  }

  nlohmann::json log() const override
  {
    return {
      {"time_total", mockturtle::to_seconds( st.time_total )}
    };
  }

private:
  mockturtle::satlut_mapping_params ps;
  mockturtle::satlut_mapping_stats st;
  unsigned window_size{32u};
};

ALICE_ADD_COMMAND( satlut_mapping, "Mapping" )

}
