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

#include <cmath>
#include <cstdint>
#include <type_traits>

#include <kitty/constructors.hpp>
#include <mockturtle/algorithms/miter.hpp>

namespace alice
{

class miter_command : public alice::command
{
public:
  miter_command( const environment::ptr& env ) : command( env, "Creates miter from two networks" )
  {
    add_option( "-a,--aig", aigs, "AIG index to use in miter" );
    add_option( "--xag", xags, "XAG index to use in miter" );
    add_option( "-m,--mig", migs, "MIG index to use in miter" );
    add_option( "-x,--xmg", migs, "XMG index to use in miter" );
  }

  rules validity_rules() const override
  {
    return {
        {[this]() {
           for ( auto i : aigs )
             if ( i >= store<aig_t>().size() )
               return false;
           return true;
         },
         "invalid index for AIG"},
        {[this]() {
           for ( auto i : xags )
             if ( i >= store<xag_t>().size() )
               return false;
           return true;
         },
         "invalid index for XAG"},
        {[this]() {
           for ( auto i : migs )
             if ( i >= store<mig_t>().size() )
               return false;
           return true;
         },
         "invalid index for MIG"},
        {[this]() {
           for ( auto i : xmgs )
             if ( i >= store<xmg_t>().size() )
               return false;
           return true;
         },
         "invalid index for XMG"},
        {[this]() {
           return aigs.size() + xags.size() + migs.size() + xmgs.size() == 2u;
         },
         "exactly two store elements need to be selected"}};
  }

  void execute() override
  {
    if ( !aigs.empty() )
    {
      if ( aigs.size() == 2u )
      {
        create_miter<aig_t, aig_t>( aigs[0u], aigs[1u] );
      }
      else if ( xags.size() == 1u )
      {
        create_miter<aig_t, xag_t>( aigs[0u], xags[0u] );
      }
      else if ( migs.size() == 1u )
      {
        create_miter<aig_t, mig_t>( aigs[0u], migs[0u] );
      }
      else if ( xmgs.size() == 1u )
      {
        create_miter<aig_t, xmg_t>( aigs[0u], xmgs[0u] );
      }
      else
      {
        env->err() << "[e] something went wrong\n";
      }
    }
    else if ( !xags.empty() )
    {
      if ( xags.size() == 2u )
      {
        create_miter<xag_t, xag_t>( xags[0u], xags[1u] );
      }
      else if ( migs.size() == 1u )
      {
        create_miter<xag_t, mig_t>( xags[0u], migs[0u] );
      }
      else if ( xmgs.size() == 1u )
      {
        create_miter<xag_t, xmg_t>( xags[0u], xmgs[0u] );
      }
      else
      {
        env->err() << "[e] something went wrong\n";
      }
    }
    else if ( !migs.empty() )
    {
      if ( migs.size() == 2u )
      {
        create_miter<mig_t, mig_t>( migs[0u], migs[1u] );
      }
      else if ( xmgs.size() == 1u )
      {
        create_miter<mig_t, xmg_t>( migs[0u], xmgs[0u] );
      }
      else
      {
        env->err() << "[e] something went wrong\n";
      }
    }
    else if ( !xmgs.empty() )
    {
      if ( xmgs.size() == 2u )
      {
        create_miter<xmg_t, xmg_t>( xmgs[0u], xmgs[1u] );
      }
      else
      {
        env->err() << "[e] something went wrong\n";
      }
    }
    else
    {
      env->err() << "[e] something went wrong\n";
    }

    aigs.clear();
    xags.clear();
    migs.clear();
    xmgs.clear();
  }

private:
  template<class Store1, class Store2>
  void create_miter( uint32_t index1, uint32_t index2 )
  {
    const auto& ntk1 = store<Store1>()[index1];
    const auto& ntk2 = store<Store2>()[index2];

    const auto miter_ntk = mockturtle::miter<typename Store2::element_type::base_type>( *ntk1, *ntk2 );
    if ( !miter_ntk )
    {
      env->err() << "[e] could not create miter from two input networks\n";
    }
    else
    {
      store<Store2>().extend();
      typename Store2::element_type view{*miter_ntk};
      store<Store2>().current() = std::make_shared<typename Store2::element_type>( view );

      constexpr auto option = store_info<Store2>::option;
      env->set_default_option( option );
    }
  }

private:
  std::vector<uint32_t> aigs;
  std::vector<uint32_t> xags;
  std::vector<uint32_t> migs;
  std::vector<uint32_t> xmgs;
};

ALICE_ADD_COMMAND( miter, "Verification" );

} // namespace alice
