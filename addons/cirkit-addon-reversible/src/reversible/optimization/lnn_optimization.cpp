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

#include "lnn_optimization.hpp"

#include <boost/format.hpp>

#include <core/functor.hpp>
#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/add_gates.hpp>

#include <stack>
#include <climits>

namespace cirkit
{

/**
 * optimizationmode
 */
  enum lnn_optimization_mode {
    LNN_OPTIMIZATION_NONE,
    LNN_OPTIMIZATION_NAIVE,
    LNN_OPTIMIZATION_LOCAL_REORDER,
    LNN_OPTIMIZATION_GLOBAL_REORDER,
  };

  /**
     calculates the nearest neighbor cost between two lines, where line1 != line2
  */
  inline unsigned nnc_func(unsigned u1, unsigned u2){
    return u1 < u2 ? u2-u1-1 : u1-u2-1;
  }

/**
 * @brief Function providing the mapping of a specific new allocated line to its index
 *
 * @param src Source line which should be looked up
 * @param allocation Array representing the lines and their current position
 * @param length Length of the allocation array
 *
 * @return The line index of the source line in the allocation
 *
 */
  unsigned lookup(unsigned src, unsigned allocation[], unsigned length)
  {
    for(unsigned index = 0; index < length; index++)
      if(allocation[index] == src)
        return index;
    return 0;
  }

/**
 * @brief Function providing the mapping of two specific new allocated lines to their indices
 *
 * @param src1 First source line which should be looked up
 * @param src2 Second source line which should be looked up
 * @param allocation Array representing the lines and their current position
 * @param length Length of the allocation array
 *
 * @return the indices of two source lines in the allocation
 *
 */
  std::pair<unsigned,unsigned> lookup2(unsigned src1, unsigned src2, unsigned allocation[], unsigned length)
  {
    unsigned i1 = 0;
    unsigned i2 = 0;
    for(unsigned index = 0; index < length; index++){
        if(allocation[index] == src1)
    i1 = index;
        if(allocation[index] == src2)
    i2 = index;
    }
    return std::make_pair(i1,i2);
  }

  /**
   * @brief Function calculating the nearest neighbor cost (nnc) of a gate wrt. a line permutation (allocation)
   *
   * @param g Specific gate for which the nnc should be calculated
   * @param allocation Array representing the lines and their current position
   * @param len Length of the allocation array
   *
   * @return the nnc of the gate
   */
  unsigned nnc(const gate& g, unsigned *allocation, unsigned len){
    if(g.size() == 2 && g.controls().front().line() < len && g.targets().front() < len ){
      auto ctrltar = lookup2(g.controls().front().line(),g.targets().front(), allocation, len);
      return nnc_func(ctrltar.first, ctrltar.second);
    }
    return 0;
  }

  /**
   * @brief Function calculating the nearest neighbor cost (nnc) of a gate
   *
   * @param g Specific gate for which the nnc should be calculated
   * @return the nnc of the gate
   */
  unsigned nnc(const gate& g){
    if(g.size() == 2)
      return nnc_func(g.controls().front().line(), g.targets().front() );
    return 0;
  }

  /**
   * @brief Function calculating the nearest neighbor cost (nnc) of a circuit, which is the sum of the nnc of all gates
   *
   * @param circ Specific circuit for which the nnc should be calculated
   * @return the nnc of the circuit
   */
  unsigned NNC(const circuit& circ){
    unsigned sum = 0;
    for ( const gate& g : circ )
      sum += nnc(g);
    return sum;
  }

/**
 * @brief Function appending a swap gate exchanging line l1 and line l2 on circuit
 *
 * @param circ Circuit to which the swap gate should be appended
 * @param line1 first target line of the fredkin gate
 * @param line2 second target line of the fredkin gate
 */
  inline void append_swap_gate(circuit& circ, unsigned line1, unsigned line2)
  {
    gate::control_container lc; //control line container is empty
    append_fredkin(circ, lc , line1, line2);
  }

  /**
   * @brief sets the (first) control line of the gate
   *
   * @param source_gate The specific gate to modify
   * @param newcontrolline The new control line of the gate
   * @return A copy of the gate where the (first) control line is changed to the specified one
   */
  gate set_gate_control_line(const gate source_gate, unsigned newcontrolline){
    gate gate = source_gate;
    gate.remove_control(gate.controls().front());
    gate.add_control(make_var(newcontrolline));
    return gate;
  }

  /**
   * @brief sets the (first) target line of the gate
   *
   * @param source_gate The specific gate to modify
   * @param newtargetline The new target line of the gate
   * @return A copy of the gate where the (first) target line is changed to the specified one
   */
  gate set_gate_target_line(const gate source_gate, unsigned newtargetline){
    gate gate = source_gate;
    gate.remove_target(gate.targets().front());
    gate.add_target(newtargetline);
    return gate;
  }

/**
 * @brief applies the optimization of the naive lnn_optimization_mode on a circuit.
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 *
 * @return the nnc of the circuit
 */
  unsigned apply_naive_scheme(circuit& circ, const circuit& base){
    copy_metadata(base,circ);
    std::stack<std::pair<unsigned,unsigned> > stack;

    for ( const gate& g : base ){
      unsigned nnc_val = nnc(g);
      if(nnc_val>0){
        bool direction = g.controls().front().line() > g.targets().front() ;
        unsigned s = g.controls().front().line() ;

        for(unsigned i = 0; i < nnc_val; i++){

          auto swap = direction ?
            std::make_pair(s - i , s - 1 -i) :
            std::make_pair(s + i , s + 1 +i) ;
          append_swap_gate(circ , swap.first, swap.second);
          stack.push(swap);
        }

        unsigned newcontrol = g.targets().front() + (direction ? 1 : -1);
        circ.append_gate() = set_gate_control_line(g, newcontrol);

        for(; !stack.empty(); stack.pop())
          append_swap_gate(circ,stack.top().first,stack.top().second);
      }
      else
        circ.append_gate() = g;
    }

    return 2*NNC(base);
  }




  /**
   * @brief Function applies a permutation on a gate permuting its control and target line
   *
   * @param src_gate The gate to transform
   * @param allocation Array representing the lines and their current position
   * @return the transformed gate
   */
  inline gate transform_gate(const gate src_gate, unsigned* allocation){
    gate transformedgate;
    transformedgate.set_type(src_gate.type());
    transformedgate.add_target(allocation[src_gate.targets().front()]);
    if(src_gate.size() > 1){
      transformedgate.add_control(make_var(allocation[src_gate.controls().front().line()]));
    }
    return transformedgate;
  }

  /**
   * @brief Function applies a permutation on a gate permuting its control and target line
   *
   * @param src_gate The gate to transform
   * @param allocation Array representing the lines and their current position
   * @param len the length of the allocation array
   * @return the transformed gate
   */
  inline gate transform_gate2(const gate src_gate, unsigned* allocation, unsigned len){
    gate transformedgate;
    transformedgate.set_type(src_gate.type());

    if(src_gate.size() == 1){
      auto tar = lookup(src_gate.targets().front(), allocation, len);
      transformedgate.add_target(tar);
    }else{
      auto ctrltar = lookup2(src_gate.controls().front().line(),src_gate.targets().front(), allocation, len);
      transformedgate.add_control(make_var(ctrltar.first));
      transformedgate.add_target(ctrltar.second);
    }
    return transformedgate;
  }

  /**
   * @brief Function creates and initiates an allocation array
   *
   * @return the allocation array
   */
  unsigned* init_allocation(unsigned lines){
    unsigned *allocation;
    allocation = new unsigned[lines];
    for(unsigned i=0; i<lines; ++i)
      allocation[i] = i;
    return allocation;
  }


/**
 * @brief After switching lines in other functions and changing the allocation, this function maps the allocation on the output and garbage strings
 *
 * @param circ Target circuit of the performing action
 * @param allocation Array representing the lines and their current position
 * @param length Length of the allocation array
 *
 * @return Returns true if the inputs have the right length and the action can be performed
 */
  bool permute_outputs_and_garbage_lines(circuit& circ, unsigned* allocation, unsigned length){
    if(circ.lines() != length)
      return false;

    std::vector<std::string> out = circ.outputs();
    std::vector<std::string> new_out( circ.lines() );
    std::vector<bool> garbage = circ.garbage();
    std::vector<bool> new_garbage( circ.lines() );

    for ( unsigned u = 0; u<length; u++ ){
      new_out[u] = out[allocation[u]];
      new_garbage[u] = garbage[allocation[u]];
    }

    circ.set_outputs(new_out);
    circ.set_garbage(new_garbage);

    return true;
}

/**
 * @brief After switching lines in other functions and changing the allocation, this function maps the allocation on the input and constant strings
 *
 * @param circ Target circuit of the performing action
 * @param allocation Array representing the lines and their current position
 * @param length Length of the allocation array
 *
 * @return Returns true if the inputs have the right length and the action can be performed
 */
  bool permute_inputs_and_constant_inputs(circuit& circ, unsigned* allocation, unsigned length){
    if(circ.lines() != length){
      return false;
    }

    std::vector<std::string> in = circ.inputs();
    std::vector<std::string> new_in( length );
    std::vector<constant> constants = circ.constants();
    std::vector<constant> new_constants ( length );

    for ( unsigned u = 0; u<length; u++ ){
      new_in[u] = in[allocation[u]];
      if ( constants[u] )
        new_constants[u] = constants[allocation[u]];
    }
    circ.set_inputs(new_in);
    circ.set_constants(new_constants);

    return true;
  }

  /**
   * @brief Modifies the allocation by swaping two entries in the allocation table
   *
   * @param allocation Array representing the lines and their current position
   * @param l1 first line to swap
   * @param l2 second line to swap
   * @param length Length of the allocation array
   */
  inline void modify_allocation(unsigned* allocation, unsigned l1, unsigned l2, unsigned length ){
    if(l1 < length && l2 < length){
      //switch allocation table entries
      allocation[l2] += allocation[l1];
      allocation[l1] =  allocation[l2]-allocation[l1];
      allocation[l2] -= allocation[l1];
    }
  }

/**
 * @brief applies the optimization of the heuristic one-side-swapping lnn_optimization_mode on a circuit wrt. moving the target line towards the control line or the control line towards the target line
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 * @param towardstarget this param is set if the control lines should be moved by swap gates towards the target line
 *
 * @return the number of swaps in the circuit
 */
  unsigned apply_directed_local_reordering_scheme(circuit& circ, const circuit& base, bool towardstarget){
    copy_metadata(base,circ);
    unsigned num_swap_gates = 0;

    unsigned* allocation = init_allocation(base.lines());

    for ( const gate& g : base ){
      unsigned nnc_val = nnc(g, allocation, base.lines());
      if(nnc_val>0){

        auto ctrltar = lookup2(g.controls().front().line(),g.targets().front(), allocation, base.lines());

        bool direction = towardstarget ?
          (ctrltar.first > ctrltar.second):
          (ctrltar.first < ctrltar.second);
        unsigned s = towardstarget ? ctrltar.first : ctrltar.second;

        for(unsigned i = 0; i < nnc_val; i++){
          auto swap = direction ?
            std::make_pair(s - i , s - 1 -i) :
            std::make_pair(s + i , s + 1 +i) ;

          append_swap_gate(circ , swap.first, swap.second);
          num_swap_gates++;
          modify_allocation(allocation, swap.first, swap.second, base.lines());
        }

        circ.append_gate() = transform_gate2(g, allocation, base.lines());

      }
      else
        circ.append_gate() = transform_gate2(g, allocation, base.lines());
    }

    //clean up
    permute_outputs_and_garbage_lines(circ, allocation, base.lines());
    delete allocation;

    return num_swap_gates;
  }

/**
 * @brief applies the optimization of the heuristic one-side-swapping lnn_optimization_mode on a circuit.
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 *
 * @return the nnc of the circuit
 */
  unsigned apply_local_reordering_scheme(circuit& circ, const circuit& base){
    circuit c1;
    circuit c2;
    copy_metadata(base,c1);
    copy_metadata(base,c2);
    unsigned cost1 = apply_directed_local_reordering_scheme(c1, base, true);
    unsigned cost2 = apply_directed_local_reordering_scheme(c2, base, false);
    copy_circuit( cost1 < cost2 ? c1 : c2 ,circ);
    return (cost1 < cost2 ? cost1 : cost2);
  }

  void switch_lines(circuit& circ, const circuit& base, unsigned l1, unsigned l2 ){
    for( gate cg : base) {
      gate g(cg);
      switch( cg.size() ){
      case 2:
        if(cg.controls().front().line() == l1)
          g = set_gate_control_line(cg,l2);
        else if (cg.controls().front().line() == l2 )
          g = set_gate_control_line(cg,l1);
        //no break - check target line in case 1
      case 1:
        if( cg.targets().front() == l1)
          g = set_gate_control_line(g,l2);
        else if( cg.targets().front() == l2 )
          g = set_gate_control_line(g,l1);
        circ.append_gate() = g;
        break;
      }
    }

    //change circuit metadata: inputs, outputs, constant and garbage lines
    std::vector<std::string> out = base.outputs();
    std::string tmp_out;
    std::vector<std::string> in = base.inputs();
    std::string tmp_in;
    std::vector<bool> garbage = base.garbage();
    bool tmp_garbage;
    std::vector<constant> constants = base.constants();
    constant tmp_constants;

    tmp_out = out[l1];
    out[l1] = out[l2];
    out[l2] = tmp_out;

    tmp_in = in[l1];
    in[l1] = in[l2];
    in[l2] = tmp_in;

    tmp_garbage = garbage[l1];
    garbage[l1] = garbage[l2];
    garbage[l2] = tmp_garbage;

    tmp_constants = constants[l1];
    constants[l1] = constants[l2];
    constants[l2] = tmp_constants;

    circ.set_inputs(in);
    circ.set_constants(constants);
    circ.set_outputs(out);
    circ.set_garbage(garbage);
  }

  /**
   * @brief calculates the impact of the lines. More precisely, for each gate g with
     control line i and target line j, the NNC value is calculated.
     This value is added to variables imp_i and imp_j which are used
     to save the impacts of the circuit lines i and j on the total NNC
     value, respectively.
   *
   * @param circ the circuit for which the impact should be calculated
   *
   * @return a vector mapping the impact to a specific line
   */
  std::vector< std::pair<unsigned,unsigned> > calculate_impact(const circuit& circ){
    std::vector< std::pair<unsigned,unsigned> > impact;
    for(unsigned line_index=0; line_index < circ.lines(); line_index++)
      impact.push_back(std::make_pair(0,line_index));

    //calculate impact
    for ( const gate& g : circ) {
      unsigned nnc_val = nnc(g);
      if(nnc_val>0){
        impact[g.controls().front().line()].first +=nnc_val;
        impact[g.targets().front()].first +=nnc_val;
      }
    }
    impact.shrink_to_fit();
    return impact;
  }

  /**
   * @brief Function switching a line with a middle line
   *
   * @param circ Target circuit of the performing action
   * @param base Source circuit providing metadata and gates
   * @param
   *
   * return the nnc of the circuit
   */
  unsigned switch_lines(circuit& circ, const circuit& base, unsigned proposedline){
    unsigned middle = base.lines()/2;
    if(base.lines()%2){
      switch_lines(circ, base, proposedline, middle);
      return NNC(circ);
    }
    else{
      //two middle lines
      circuit circ2;
      copy_metadata(base,circ2);
      switch_lines(circ, base, proposedline, middle);
      switch_lines(circ2, base, proposedline, middle-1);
      unsigned NNC1 = NNC(circ);
      unsigned NNC2 = NNC(circ2);

      if(NNC2 < NNC1){
        circ = circ2;
        return NNC2;
      }
      return NNC1;
    }
  }

/**
 * @brief heuristic method for reordering the lines of a circuit, with the intention moving the line with the highest NNC impact in the middle
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 *
 * @return the nnc of the circuit
 */
  unsigned global_reorder_circuit(circuit& circ, const circuit& base){
    auto impact = calculate_impact(base);
    std::sort(impact.begin(), impact.end(), [](std::pair<unsigned,unsigned> a, std::pair<unsigned,unsigned> b){ return a.first > b.first; });
    //multiple lines may have the same highest impact (unspecified in the algorithm)
    std::vector<unsigned> max_lines = [](std::vector<std::pair<unsigned,unsigned>> input){
      unsigned max = input[0].first;
      std::vector<unsigned> out;
      for(std::pair<unsigned,unsigned> p : input)
        if(p.first == max)
          out.push_back(p.second);
      return out;
    }(impact);

    unsigned best_nnc = UINT_MAX;
    for(unsigned line : max_lines){
      if (max_lines.size() < impact.size() && (line == base.lines()/2 || line+1 == base.lines()/2))
        line = impact[max_lines.size()].second; //select line with the next highest impact
      circuit circ2;
      copy_metadata(base,circ2);
      unsigned circ_nnc = switch_lines(circ2, base, line);
      if(circ_nnc < best_nnc){
        circ = circ2;
        best_nnc = circ_nnc;
      }
    }
    return best_nnc;
  }

  /**
   * @brief An ordering of the circuit lines which reduces the total NNC value
     is desired. To do that, the contribution of each line to the total
     NNC value is calculated. More precisely, for each gate g with
     control line i and target line j, the NNC value is calculated.
     This value is added to variables imp_i and imp_j which are used
     to save the impacts of the circuit lines i and j on the total NNC
     value, respectively.
     Next, the line with the highest NNC impact is chosen for
     reordering and placed at the middle line (i.e., swapped with the
     middle line). If the selected line is the middle line itself, a
     line with the next highest impact is selected. This procedure is
     repeated until no better NNC value is achieved.
  *
  * @param circ Target circuit of the performing action
  * @param base Source circuit providing metadata and gates
  *
  * @return the nnc of the circuit
  */
unsigned apply_global_reordering_scheme( circuit& circ, const circuit& base) {
  copy_metadata(base,circ);
  unsigned circ_nnc = global_reorder_circuit(circ, base);
  while (true){
    circuit circ2;
    copy_metadata(base,circ2);
    unsigned circ2_nnc = global_reorder_circuit(circ2, circ);
    if(circ2_nnc < circ_nnc){
      circ = circ2;
      circ_nnc = circ2_nnc;
    }
    else
      return 2*circ_nnc;
  }
}

  /**
   * @brief Applies the linear nearest neighbor optimization to a quantum circuit. Different modes are possible.
      The global and local reordering scheme have been introduced in [\ref SWD10].
   *  reordering mode:
   *  1: naive, 2: local reordering, 3: global reordering
   *
   */
  bool lnn_optimization( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
  {
    /* settings */
    bool verbose = get(settings, "verbose", false);
    /*  1: naive, 2: local reordering, 3: global reordering */
    unsigned reordering_mode = get<unsigned>(settings, "reordering_mode", 0u);
    
    // Run-time measuring
    properties_timer t( statistics );

    unsigned number_of_swaps;
    switch(reordering_mode) {
    case LNN_OPTIMIZATION_NONE:
      copy_circuit(base,circ);
      number_of_swaps = NNC(base)*2;
      break;
    case LNN_OPTIMIZATION_NAIVE:
      if(NNC(base) == 0)
	copy_circuit(base,circ);
      else
	number_of_swaps = apply_naive_scheme(circ,base);
      break;
    case LNN_OPTIMIZATION_LOCAL_REORDER:
      if(NNC(base) == 0)
	copy_circuit(base,circ);
      else
	number_of_swaps = apply_local_reordering_scheme(circ, base);
      break;
    case LNN_OPTIMIZATION_GLOBAL_REORDER:
      number_of_swaps = apply_global_reordering_scheme(circ, base);
      break;

    default:
      if(verbose)
	std::cout << "invalid mode." << std::endl;
      return false;
    }

    if(verbose)
      std::cout << "SWAP gates: " << number_of_swaps << std::endl;
    
    return true;
}

optimization_func lnn_optimization_func( properties::ptr settings, properties::ptr statistics )
{
    optimization_func f = [&settings, &statistics]( circuit& circ, const circuit& base ) {
      return lnn_optimization( circ, base, settings, statistics );
    };
    f.init( settings, statistics );
    return f;
}
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
