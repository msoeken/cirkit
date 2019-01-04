#include <alice/alice.hpp>

#include <mockturtle/properties/migcost.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/views/depth_view.hpp>

#include "../utils/cirkit_command.hpp"

namespace alice
{

class migcost_command : public cirkit::cirkit_command<migcost_command, aig_t, mig_t>
{
public:
  migcost_command( environment::ptr& env ) : cirkit::cirkit_command<migcost_command, aig_t, mig_t>( env, "Performs k-LUT mapping", "apply LUT-mapping to {0}" )
  {
    add_flag( "--silent", silent, "produce no output" );
  }

  template<class Store>
  inline void execute_store()
  {
    const auto& ntk = *store<Store>().current();
    num_gates = ntk.num_gates();
    num_inv = mockturtle::num_inverters( ntk );

    mockturtle::depth_view depth_ntk{ntk};
    depth = depth_ntk.depth();

    mockturtle::depth_view depth_ntk2{ntk, true};
    depth_mixed = depth_ntk2.depth();
    std::tie( depth_maj, depth_inv ) = split_critical_path( depth_ntk2 );

    num_dangling = mockturtle::num_dangling_inputs( ntk );

    qca_area = num_gates * 0.0012 + num_inv * 0.004;
    qca_delay = depth_maj * 0.004 + depth_inv * 0.014;
    qca_energy = num_gates * 2.94 + num_inv * 9.8;

    stmg_area = std::max( num_gates * 0.0036, num_inv * 0.06 );
    stmg_delay = depth_maj * 1.5 + depth_inv * 0.0262701;
    stmg_energy = 700 * num_dangling + 3.98495 * num_inv;

    if ( !silent )
    {
      env->out() << fmt::format( "[i] Gates             = {}\n"
                                 "[i] Inverters         = {}\n"
                                 "[i] Depth (def.)      = {}\n"
                                 "[i] Depth mixed       = {}\n"
                                 "[i] Depth mixed (MAJ) = {}\n"
                                 "[i] Depth mixed (INV) = {}\n"
                                 "[i] Dangling inputs   = {}\n",
                                 num_gates, num_inv, depth, depth_mixed, depth_maj, depth_inv, num_dangling );

      env->out() << fmt::format( "[i] QCA (area)        = {:.2f} um^2\n"
                                 "[i] QCA (delay)       = {:.2f} ns\n"
                                 "[i] QCA (energy)      = {:.2f} E-21 J\n",
                                 qca_area, qca_delay, qca_energy );

      env->out() << fmt::format( "[i] STMG (area)       = {:.2f} um^2\n"
                                 "[i] STMG (delay)      = {:.2f} ns\n"
                                 "[i] STMG (energy)     = {:.2f} E-21 J\n",
                                 stmg_area, stmg_delay, stmg_energy );
    }
  }

  nlohmann::json log() const override
  {
    return {
        {"num_gates", num_gates},
        {"num_inverters", num_inv},
        {"depth", depth},
        {"depth_mixed", depth_mixed},
        {"depth_maj", depth_maj},
        {"depth_inv", depth_inv},
        {"num_dangling", num_dangling},
        {"qca_area", qca_area},
        {"qca_delay", qca_delay},
        {"qca_energy", qca_energy},
        {"stmg_area", stmg_area},
        {"stmg_delay", stmg_delay},
        {"stmg_energy", stmg_energy}};
  }

private:
  template<class Ntk>
  std::pair<unsigned, unsigned> split_critical_path( Ntk const& ntk )
  {
    using namespace mockturtle;

    unsigned num_maj{0}, num_inv{0};

    node<Ntk> cp_node;
    ntk.foreach_po( [&]( auto const& f ) {
      auto level = ntk.level( ntk.get_node( f ) );
      if ( ntk.is_complemented( f ) )
      {
        level++;
      }

      if ( level == ntk.depth() )
      {
        if ( ntk.is_complemented( f ) )
        {
          ++num_inv;
        }
        cp_node = ntk.get_node( f );
        return false;
      }

      return true;
    } );

    while ( !ntk.is_constant( cp_node ) && !ntk.is_pi( cp_node ) )
    {
      num_maj++;

      ntk.foreach_fanin( cp_node, [&]( auto const& f ) {
        auto level = ntk.level( ntk.get_node( f ) );
        if ( ntk.is_complemented( f ) )
        {
          level++;
        }

        if ( level + 1 == ntk.level( cp_node ) )
        {
          if ( ntk.is_complemented( f ) )
          {
            num_inv++;
          }
          cp_node = ntk.get_node( f );
          return false;
        }
        return true;
      } );
    }

    return {num_maj, num_inv};
  }

private:
  unsigned num_gates{0u}, num_inv{0u}, depth{0u}, depth_mixed{0u}, depth_maj{0u}, depth_inv{0u}, num_dangling{0u};
  double qca_area, qca_delay, qca_energy;
  double stmg_area, stmg_delay, stmg_energy;
  bool silent{false};
};

ALICE_ADD_COMMAND( migcost, "Various" )

} // namespace alice
