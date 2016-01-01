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
 * @file read_pla_to_cirkit_bdd.hpp
 *
 * @brief Functions to read a pla file and building a cirkit BDD.
 *
 * @author Stefan Frehse
 */

#ifndef READ_PLA_TO_CIRKIT_BDD
#define READ_PLA_TO_CIRKIT_BDD

#include <core/properties.hpp>
#include <classical/dd/bdd.hpp>

#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

namespace cirkit
{

class bdd_function : public boost::noncopyable
{
  using bdd_map_t = std::map<unsigned, bdd>;
  using name_map_t = std::map<unsigned, std::string>;

  public:
    bdd_function (unsigned nvars, unsigned log_max_objs, bool verbose = false );
    explicit bdd_function ( bdd_manager_ptr manager );
    ~bdd_function ();

    bdd_function ( const bdd_manager& ) = delete;
    bdd_function& operator= ( bdd_function const& func ) = delete;

    bdd::const_param_ref lookupInput( unsigned index ) const;
    bdd::const_param_ref lookupOutput( unsigned index ) const;

    void setOutputVar ( unsigned index, const bdd& var );

    bdd_manager_ptr manager() const;

    void setInputName ( unsigned index, const std::string& name);
    void setOutputName ( unsigned index, const std::string& name);

    template<typename Callback>
    void for_each_output ( const Callback& callback ) const {
      for ( auto const& p : m_outputNames) {
        callback ( p.first, p.second );
      }
    }

    void pushInput ( unsigned index );

    inline unsigned num_inputs() const { return m_inputs.size(); }
    inline unsigned num_outputs() const { return m_outputs.size(); }
  
  std::vector<std::string> input_labels() const;
  std::vector<std::string> output_labels() const;
  
  private:
    bdd_manager_ptr m_manager;
    bdd_map_t m_inputs;
    bdd_map_t m_outputs;

    name_map_t m_inputNames;
    name_map_t m_outputNames;
};

using bdd_function_ptr  = std::shared_ptr<bdd_function>;
using bdd_function_cptr = std::shared_ptr<const bdd_function>;

bdd_function_cptr read_pla_into_cirkit_bdd( const boost::filesystem::path &filename,
                                            const properties::ptr& settings = properties::ptr() );

} // namespace cirkit

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
