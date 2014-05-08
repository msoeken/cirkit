/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2013  The RevKit Developers <revkit@informatik.uni-bremen.de>
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

#include "rcbdd_synthesis.hpp"
#include "synthesis_utils_p.hpp"

#include <boost/range/algorithm_ext/push_back.hpp>

namespace revkit
{

enum Direction {
  ChangeLeft = 0,
  ChangeRight
};

struct rcbdd_synthesis_manager
{
  rcbdd_synthesis_manager( const rcbdd& _cf, bool _verbose ) : cf( _cf ), verbose( _verbose )
  {
    f = _cf.chi();
  }

  void set_var(unsigned v)
  {
    _var = v;

    left_f = cf.manager().bddZero();
    right_f = cf.manager().bddZero();
  }

  void compute_cofactors()
  {
    n  = cf.cofactor(f, _var, false, false);
    pp = cf.cofactor(f, _var, true,  false);
    np = cf.cofactor(f, _var, false, true);
    p  = cf.cofactor(f, _var, true,  true);

    nx  = cf.remove_ys(n);
    ppx = cf.remove_ys(pp);
    npx = cf.remove_ys(np);
    px  = cf.remove_ys(p);

    ny  = cf.remove_xs(n);
    ppy = cf.remove_xs(pp);
    npy = cf.remove_xs(np);
    py  = cf.remove_xs(p);
  }

  void apply_gates(const BDD& lf, const BDD& rf)
  {
    left_f ^= lf;
    right_f ^= rf;

    BDD gate_left = cf.create_from_gate(_var, lf);
    BDD gate_right = cf.create_from_gate(_var, cf.move_ys_to_xs(rf));
    f = cf.compose(cf.compose(gate_left, f), gate_right);
  }

  void only_left_gate_shortcut()
  {
    BDD chi_prime = f;

    // Copy y_var to x_var
    chi_prime = chi_prime.ExistAbstract(cf.x(_var)) & cf.x(_var).Xnor(cf.y(_var));

    // Check whether chi' is reversible
    if (mpz_class(chi_prime.CountMinterm(2u * cf.num_vars())) == pow2(cf.num_vars())
        && cf.remove_ys(chi_prime) == cf.manager().bddOne()
        && cf.remove_xs(chi_prime) == cf.manager().bddOne())
    {
      BDD lf = cf.remove_ys(!f & chi_prime).ExistAbstract(cf.x(_var));
      apply_gates(lf, cf.manager().bddZero());
    }
  }

  void resolve_one_cycles()
  {
    // Via F_n
    compute_cofactors();

    //BDD lf = (!nx & ppx) ^ (!px & npx);
    BDD lf = ppx & npx;

    apply_gates(lf, cf.manager().bddZero());

    compute_cofactors();

    BDD rf = ppy & npy;

    apply_gates(cf.manager().bddZero(), rf);

    // // Via F_p
    // compute_cofactors();

    // lf = !px & npx;
    // rf = !py & ppy;

    // apply_gates(lf, rf);
  }

  void resolve_two_cycles()
  {
    // Via F_p
    compute_cofactors();

    BDD _f = ppy & p & npx;
    BDD lf = cf.remove_ys(_f);
    BDD rf = cf.remove_xs(_f);

    apply_gates(lf, rf);

    // Via F_n
    compute_cofactors();

    _f = npy & n & ppx;
    lf = cf.remove_ys(_f);
    rf = cf.remove_xs(_f);

    apply_gates(lf, rf);
  }


  void cycle_step()
  {
    compute_cofactors();

    std::vector<BDD> variables;
    boost::push_back(variables, cf.xs());
    boost::push_back(variables, cf.ys());

    // Pick an arbitrary cube from pp
    BDD cube = pp.PickOneMinterm(variables);
    char change = ChangeLeft;

    BDD lf = cf.manager().bddZero();
    BDD rf = cf.manager().bddZero();
    BDD cube_part;
    char *scube = new char[3u * cf.num_vars()];

    do
    {
      if (change == ChangeLeft)
      {
        // Update left gate
        cube_part = cf.remove_ys(cube).ExistAbstract(cf.x(_var));
        lf |= cube_part;
        //change = CHANGE_RIGHT;
        cube = !cf.x(_var) & cube_part & f;
      }
      else
      {
        cube_part = cf.remove_xs(cube).ExistAbstract(cf.y(_var));
        rf |= cube_part;
        //change = CHANGE_LEFT;
        cube = cf.y(_var) & cube_part & f;
      }

      /* Cube is not part of the function? */
      if (cube == cf.manager().bddZero())
      {
        BDD unused_outputs = !cf.remove_xs(f);
        BDD unused_inputs = !cf.remove_ys(f);
        if (verbose)
        {
          std::cout << "change: " << (unsigned)change << std::endl;
          double uim = unused_inputs.CountMinterm(cf.num_vars());
          double uom = unused_outputs.CountMinterm(cf.num_vars());
          std::cout << uim << " " << uom << std::endl;
          if (uim != uom) exit(0);
        }

        BDD icube, ocube;

        if (change == ChangeLeft)
        {
          icube = !cf.x(_var) & cube_part;

          ocube = cf.manager().bddOne();
          if ((unused_outputs & cf.y(_var)) != cf.manager().bddZero())
          {
            (unused_outputs & cf.y(_var)).PickOneCube(scube);
          }
          else
          {
            unused_outputs.PickOneCube(scube);
          }
          for (unsigned i = 0u; i < cf.num_vars(); ++i)
          {
            ocube &= (scube[3u * i + 1u] == 1) ? cf.y(i) : !cf.y(i);
          }
        }
        else
        {
          ocube = cf.y(_var) & cube_part;

          icube = cf.manager().bddOne();
          if ((unused_inputs & !cf.x(_var)) != cf.manager().bddZero())
          {
            (unused_inputs & !cf.x(_var)).PickOneCube(scube);
          }
          else
          {
            unused_inputs.PickOneCube(scube);
          }
          for (unsigned i = 0u; i < cf.num_vars(); ++i)
          {
            icube &= (scube[3u * i] == 1) ? cf.x(i) : !cf.x(i);
          }
        }

        cube = icube & ocube;

        if (verbose)
        {
          std::cout << "icube: " << std::endl;
          icube.PrintMinterm();
          std::cout << "ocube: " << std::endl;
          ocube.PrintMinterm();
          std::cout << "cube: " << std::endl;
          cube.PrintMinterm();
          std::cout << "f & cube (before): " << std::endl;
          (f & cube).PrintMinterm();
        }

        f |= cube;

        if (verbose)
        {
          std::cout << "f & cube (after): " << std::endl;
          (f & cube).PrintMinterm();
        }

        compute_cofactors();
      }

      change = ((char)1u - change);
    } while((cube & (pp | np)) == cf.manager().bddZero());

    /* Cleanup */
    delete[] scube;

    apply_gates(lf, rf);
  }

  void resolve_k_cycles()
  {
    while (cf.cofactor(f, _var, true, false) != cf.manager().bddZero())
    {
      cycle_step();
    }
  }

  void default_synthesis()
  {
    for (unsigned var = 0; var < cf.num_vars(); ++var)
    {
      if ( verbose )
      {
        std::cout << "Adjust variable " << var << std::endl;
      }
      set_var(var);
      only_left_gate_shortcut();
      resolve_one_cycles();
      resolve_two_cycles();
      resolve_k_cycles();

      if (verbose)
      {
        std::cout << "Target: " << _var << std::endl << " - left control function:" << std::endl;
        left_f.PrintMinterm();
        std::cout << " - right control function:" << std::endl;
        right_f.PrintMinterm();
      }

      //create_toffoli_gates_with_exorcism(left_f, 0u);
      //create_toffoli_gates_with_exorcism(right_f, 1u);
    }
  }
  /*
  void heuristic_swap() //Get chi
  {
    std::vector<unsigned> list_lines;
    for (unsigned var = 0; var < cf.num_vars(); ++var)
    {
      list_lines.push_back(var);
    }

    BDD lf_c, lr_c;

    while (!list_lines.empty())
    {
      unsigned min_cost = UINT_MAX;
      unsigned best_line = 0u;

      for (unsigned i = 0u; i < list_lines.size(); ++i)
      {
        BDD oldchi = f; //make a copy of chi
        unsigned old_control_lines = total_control_lines;
        unsigned old_toffoli_gates = total_toffoli_gates;
        std::cout << " - total_toffoli_gates" << total_toffoli_gates << std::endl;
        set_var(list_lines[i]);

        std::cout << "set_var(var): " << _var << std::endl;
        only_left_gate_shortcut();
        resolve_one_cycles();
        resolve_two_cycles();
        resolve_k_cycles();

        if (verbose)
        {
          std::cout << "Target: " << _var << std::endl << " - left control function:" << std::endl;
          left_f.PrintMinterm();
          std::cout << " - right control function:" << std::endl;
          right_f.PrintMinterm();
        }

        create_toffoli_gates_with_exorcism(left_f, 0u);
        create_toffoli_gates_with_exorcism(right_f, 1u);


        // Determine cost and save in new_cost
        unsigned new_cost = total_toffoli_gates - old_toffoli_gates;

        if ( verbose )
        {
          std::cout << "h1: Lines:    " << mgr.num_vars() << std::endl;
          std::cout << "h1: Gates:    " << new_cost << std::endl;
          std::cout << "Controls:     " << total_control_lines << std::endl;
        }

        if (new_cost < min_cost)
        {
          best_line = list_lines[i];
          min_cost = new_cost;
          if ( verbose )
          {
            std::cout << "--------------  Min cost:  " << min_cost << "   -------------- "  << std::endl;
          }
        }

        f = oldchi;
        total_toffoli_gates = old_toffoli_gates;
        total_control_lines = old_control_lines;
      }

      set_var(best_line);
      // Synthesis with best_line
      only_left_gate_shortcut();
      resolve_one_cycles();
      resolve_two_cycles();
      resolve_k_cycles();

      create_toffoli_gates_with_exorcism(left_f, 0u);
      create_toffoli_gates_with_exorcism(right_f, 1u);

      list_lines.erase(std::remove(list_lines.begin(),list_lines.end(),best_line));

      if ( verbose )
      {
        std::cout << "--------------  Best Lines:  " << best_line << "   -------------- "  << std::endl;
      }
    }
  }

  void heuristic_hamming()
  {
    std::vector<unsigned> list_lines;
    for (unsigned var = 0; var < cf.num_vars(); ++var)
    {
      list_lines.push_back(var);
    }

    BDD lf_c, lr_c;

    while (!list_lines.empty())
    {
      double min_cost = DBL_MAX;
      unsigned best_line = 0u;

      for (unsigned i = 0u; i < list_lines.size(); ++i)
      {
        // Determine costs and save in new_cost
        double new_cost = cf.cofactor(f, list_lines[i], false, true).CountMinterm(2 * cf.num_vars());


        if (new_cost < min_cost)
        {
          best_line = list_lines[i];
          min_cost = new_cost;
          if ( verbose )
          {
            std::cout << "--------------  Min cost:  " << min_cost << "   -------------- "  << std::endl;
          }
        }
      }

      set_var(best_line);
      // Synthesis with best_line
      only_left_gate_shortcut();
      resolve_one_cycles();
      resolve_two_cycles();
      resolve_k_cycles();

      create_toffoli_gates_with_exorcism(left_f, 0u);
      create_toffoli_gates_with_exorcism(right_f, 1u);

      list_lines.erase(std::remove(list_lines.begin(),list_lines.end(),best_line));

      if ( verbose )
      {
        std::cout << "--------------  Best Lines:  " << best_line << "   -------------- "  << std::endl;
      }
    }
  }
  */

  const rcbdd& cf;
  bool verbose;
  BDD f;
  BDD left_f, right_f;
  unsigned _var;
  BDD n, pp, np, p;
  BDD nx, ppx, npx, px;
  BDD ny,  ppy, npy, py;
};

bool rcbdd_synthesis( circuit& circ, const rcbdd& cf, properties::ptr settings, properties::ptr statistics )
{
  /* Settings */
  bool verbose = get<bool>( settings, "verbose", false );

  rcbdd_synthesis_manager mgr( cf, verbose );
  mgr.default_synthesis();

  return false;
}

}

// Local Variables:
// c-basic-offset: 2
// End:
