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
* @author Stefan Frehse
*/

#if defined(HAS_METASMT)

#include <metaSMT/support/default_visitation_unrolling_limit.hpp>
#include <metaSMT/DirectSolver_Context.hpp>
#include <metaSMT/backend/CVC4.hpp>
#include <metaSMT/backend/MiniSAT.hpp>
#include <metaSMT/backend/PicoSAT.hpp>
#include <metaSMT/backend/Lingeling.hpp>
#include <metaSMT/backend/Z3_Backend.hpp>
#include <metaSMT/backend/Boolector.hpp>
#include <metaSMT/backend/SAT_Aiger.hpp>
#include <metaSMT/BitBlast.hpp>
#include <metaSMT/frontend/Logic.hpp>
#include <metaSMT/support/run_algorithm.hpp>

#include <core/properties.hpp>

#include <reversible/circuit.hpp>
#include <reversible/utils/reversible_program_options.hpp>
#include <reversible/io/read_realization.hpp>

#include <reversible/verification/formal_equivalence_check.hpp>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/timer.hpp>

#include <algorithm>

#include <cstdlib>

using namespace cirkit;
using namespace metaSMT;

class EquivalenceCheckMain
{
  typedef boost::mpl::vector <
      DirectSolver_Context < BitBlast < SAT_Aiger < solver::MiniSAT>>>
    , DirectSolver_Context < BitBlast < SAT_Aiger < solver::PicoSAT>>>
    , DirectSolver_Context < BitBlast < SAT_Aiger < solver::Lingeling>>>
    , DirectSolver_Context < solver::Boolector >
    , DirectSolver_Context < solver::Z3_Backend >
    , DirectSolver_Context < solver::CVC4 >
  > SolverVec;

  public:
    EquivalenceCheckMain () {
    }

    int run ( int argc, char *argv[] ) {
      reversible_program_options opts;

      using boost::filesystem::path;
      using boost::program_options::value;

      opts.add_options()
              ( "spec", value<path> ( &m_spec_filename )->required(), "Filename to specification")
              ( "impl", value<path> ( &m_impl_filename )->required(), "Filename to implementation");

      opts.parse( argc, argv );

      if ( !opts.good() ) {
        std::cerr << opts << std::endl;
        return EXIT_FAILURE;
      }

      boost::timer timer;
      auto spec = read_circuit ( m_spec_filename );
      auto impl = read_circuit ( m_impl_filename );

      if ( !checkable ( spec ) || !checkable ( impl ) ) {
        std::cout << "[w]: Circuits are not checkable." << std::endl;
        return EXIT_FAILURE;
      }

      std::cout <<
                   boost::format ("[i] Both files successfully read: %.2f seconds")
                   % timer.elapsed()
                << std::endl;

      return check ( spec, impl );
    }

    bool checkable ( circuit const& circ ) const {

      auto const& constants = circ.constants();
      auto has_constants = std::any_of (
              constants.begin()
            , constants.end()
            , [] ( constant c ) { return c; } );

      auto const& garbage = circ.garbage();
      auto has_garbage = std::any_of (
              garbage.begin()
            , garbage.end()
            , [] ( bool val ) { return val; });

      return !has_constants && !has_garbage;
    }

    bool check ( circuit const& spec, circuit const& impl ) {

      boost::timer timer;
      metaSMT::run_algorithm< SolverVec, formal_equivalence_check> (
              2
            , spec
            , impl
      );

      std::cout << boost::format ("[i]: Runtime. %.2f seconds") % timer.elapsed() << std::endl;

      return EXIT_SUCCESS;
    }

    circuit read_circuit ( boost::filesystem::path const& filename ) const
    {
      assert ( !filename.empty() );

      boost::filesystem::ifstream instream ( filename );

      circuit circ;
      if ( !read_realization( circ, instream ) ) {
        throw std::runtime_error (
              boost::str ( boost::format ("Error while reading file '%s'.")
              % filename ) ) ;
      }
      return circ;
    }

  private:
    boost::filesystem::path m_spec_filename;
    boost::filesystem::path m_impl_filename;
};

int main ( int argc, char *argv[] ) {
  try {
    return EquivalenceCheckMain().run ( argc, argv );
  } catch ( std::exception const& e ) {
    std::cerr << "[e] " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}

#else
#include <iostream>
int main(int argc, char *argv[])
{
  std::cerr << "[w] metaSMT has not been activated for this addon." << std::endl;
  return EXIT_FAILURE;
}
#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
