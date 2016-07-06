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

#include "spec_representation.hpp"

#include <classical/mig/mig_simulate.hpp>
#include <classical/mig/mig_utils.hpp>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

struct is_explicit_visitor : public boost::static_visitor<bool>
{
  bool operator()( const tt& spec ) const
  {
    return true;
  }

  bool operator()( const mig_graph& spec ) const
  {
    return false;
  }
};

struct num_vars_visitor : public boost::static_visitor<unsigned>
{
  unsigned operator()( const tt& spec ) const
  {
    return tt_num_vars( spec );
  }

  unsigned operator()( const mig_graph& spec ) const
  {
    return mig_info( spec ).inputs.size();
  }
};

struct support_visitor : public boost::static_visitor<boost::dynamic_bitset<>>
{
  boost::dynamic_bitset<> operator()( const tt& spec ) const
  {
    return tt_support( spec );
  }

  boost::dynamic_bitset<> operator()( const mig_graph& spec ) const
  {
    return ~boost::dynamic_bitset<>( mig_info( spec ).inputs.size() );
  }
};

struct symmetric_variables_visitor : public boost::static_visitor<std::vector<std::pair<unsigned, unsigned>>>
{
  std::vector<std::pair<unsigned, unsigned>> operator()( const tt& spec ) const
  {
    std::vector<std::pair<unsigned, unsigned>> pairs;

    const auto n = tt_num_vars( spec );

    for ( auto j = 1u; j < n; ++j )
    {
      for ( auto i = 0u; i < j; ++i )
      {
        if ( tt_permute( spec, i, j ) == spec )
        {
          pairs.push_back( {i, j} );
        }
      }
    }

    return pairs;
  }

  std::vector<std::pair<unsigned, unsigned>> operator()( const mig_graph& spec ) const
  {
    return {};
  }
};

struct is_trivial_visitor : public boost::static_visitor<boost::optional<std::pair<unsigned, bool>>>
{
  boost::optional<std::pair<unsigned, bool>> operator()( const tt& spec ) const
  {
    /* terminal cases */
    if ( ( ~spec ).none() || spec.none() )
    {
      return std::make_pair( 0u, spec.test( 0u ) );
    }

    /* single variable */
    tt spec_copy = spec;
    tt_extend( spec_copy, 6u );

    const auto nvars = tt_num_vars( spec_copy );

    if ( nvars == 6u )
    {
      for ( auto i = 0u; i < 6u; ++i )
      {
        if ( spec_copy == tt_store::i()( i ) || ~spec_copy == tt_store::i()( i ) )
        {
          return std::make_pair( i + 1u, spec_copy.test( 0u ) );
        }
      }
    }
    else
    {
      const auto ttn = tt_nth_var( nvars - 1 );
      if ( spec_copy == ttn || ~spec_copy == ttn )
      {
        return std::make_pair( nvars, spec_copy.test( 0u ) );
      }
    }

    return boost::none;
  }

  boost::optional<std::pair<unsigned, bool>> operator()( const mig_graph& spec ) const
  {
    /* let's be pessimistic for now */
    return boost::none;
  }
};

struct is_normal_visitor : public boost::static_visitor<bool>
{
  bool operator()( const tt& spec ) const
  {
    return !spec[0];
  }

  bool operator()( const mig_graph& spec ) const
  {
    mig_simple_assignment_simulator::mig_name_value_map m;
    const auto r = simulate_mig( spec, mig_simple_assignment_simulator( m ) );

    return !r.at( mig_info( spec ).outputs[0u].first );
  }
};

struct invert_visitor : public boost::static_visitor<void>
{
  void operator()( tt& spec ) const
  {
    spec.flip();
  }

  void operator()( mig_graph& spec ) const
  {
    mig_info( spec ).outputs[0].first.complemented ^= 1;
  }
};

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

spec_representation::spec_representation( const spec_t& spec )
  : spec( spec )
{
}

bool spec_representation::is_explicit() const
{
  return boost::apply_visitor( is_explicit_visitor(), spec );
}

unsigned spec_representation::num_vars() const
{
  return boost::apply_visitor( num_vars_visitor(), spec );
}

boost::dynamic_bitset<> spec_representation::support() const
{
  return boost::apply_visitor( support_visitor(), spec );
}

std::vector<std::pair<unsigned, unsigned>> spec_representation::symmetric_variables() const
{
  return boost::apply_visitor( symmetric_variables_visitor(), spec );
}

boost::optional<std::pair<unsigned, bool>> spec_representation::is_trivial() const
{
  return boost::apply_visitor( is_trivial_visitor(), spec );
}

bool spec_representation::is_normal() const
{
  return boost::apply_visitor( is_normal_visitor(), spec );
}

void spec_representation::invert()
{
  boost::apply_visitor( invert_visitor(), spec );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
