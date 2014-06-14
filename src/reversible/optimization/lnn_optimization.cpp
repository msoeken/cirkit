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

#include "lnn_optimization.hpp"

#include <boost/format.hpp>

#include <core/functor.hpp>
#include <core/utils/timer.hpp>

#include <reversible/circuit.hpp>
#include <reversible/functions/copy_circuit.hpp>
#include <reversible/functions/copy_metadata.hpp>
#include <reversible/functions/add_gates.hpp>
#include <reversible/target_tags.hpp>

#include <map>

#include <iostream>

namespace revkit
{

/**
 * optimizationmode
 */
enum lnn_optimization_mode { LNN_OPTIMIZATION_NAIVE, LNN_OPTIMIZATION_LOCAL_REORDER, LNN_OPTIMIZATION_GLOBAL_REORDER };

/**
 * Datastructure representing calculated values of a specific gate
 */
struct gate_details
{
    gate mod_gate;
    unsigned target;
    bool control_exists;
    unsigned control;
    int distance;
};

/**
 * @brief applies the optimization of the naive and heuristic one-side-swapping lnn_optimization_mode on a circuit.
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 * @param operation The optimizationlnn_optimization_mode for defining specific actions
 * @param allocation Array representing the lines and their current position
 *
 */
void apply_lnn( circuit& circ, const circuit& base, lnn_optimization_mode operation, unsigned allocation[]);

/**
 * @brief calculates the gate details specified in the struct
 *
 * @param cur_gate The gate which is target of the current action
 * @param operation The optimizationlnn_optimization_mode for defining specific actions
 * @param allocation Array representing the lines and their current position
 * @param base Source circuit providing metadata and gates
 *
 * @return A struct representing specific gate data
 *
 */
struct gate_details calculate_gate_details(gate cur_gate, lnn_optimization_mode operation, unsigned *allocation, const circuit& base) throw (std::exception);

/**
 * @brief Function providing the mapping of a specific new allocated line to its index
 *
 * @param src Source line which should be looked up
 * @param allocation Array representing the lines and their current position
 * @param length Length of the allocation array
 *
 * @return The line index of the source line
 *
 */
unsigned lookup(unsigned src, unsigned allocation[], unsigned length);

/**
 * @brief Modifies the target line of the current gate and inserts swap gates infront (and depending on the lnn_optimization_mode: behind) the gate
 *
 * @param cur_gate The gate which is target of the current action
 * @param gate_details Contains information of the current gate
 * @param index Position where the gate should be placed
 * @param operation The optimizationlnn_optimization_mode for defining specific actions
 * @param allocation Array representing the lines and their current position
 *
 */
void adding_swap_gates(circuit& circ, struct gate_details gate_details, unsigned index, lnn_optimization_mode operation, unsigned allocation[]);

/**
 * @brief inserts a swap gate between line l1 and line l2 on circuit target_circ at the position of the index
  *
 * @param cur_gate The gate which is target of the current action
 * @param index Position where the gate should be placed
 * @param l1 first target line of the fredkin gate
 * @param l2 second target line of the fredkin gate
 *
 */
void insert_swap_gate(circuit &circ, unsigned index, unsigned l1, unsigned l2);

/**
 * @brief reconfigure all the gates connecting to new allocated lines
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 * @param allocation Array representing the lines and their current position
 *
 */
void switch_lines(circuit& circ, const circuit& base, unsigned allocation[] );

/**
 * @brief heuristic method for reordering the lines of a circuit, with the intention moving the line with the highest NNC impact in the middle
 *
 * @param circ Target circuit of the performing action
 * @param base Source circuit providing metadata and gates
 * @param allocation Array representing the lines and their current position
 *
 */
void reorder( circuit& circ, const circuit& base, unsigned allocation[]);

/**
 * @brief After switching lines in other functions and changing the allocation, this function maps the allocation on the output strings
 *
 * @param circ Target circuit of the performing action
 * @param allocation Array representing the lines and their current position
 * @param length Length of the allocation array
 *
 * @return Returns true if the inputs have the right length and the action can be performed
 */
bool apply_allocation_on_outputs(circuit& circ, unsigned allocation[], unsigned length);


bool lnn_optimization( circuit& circ, const circuit& base, properties::ptr settings, properties::ptr statistics )
{
  unsigned selection_input = get<unsigned>(settings, "reordering", 0u);
  lnn_optimization_mode m;
    switch(selection_input) {
    case 1:
        m = LNN_OPTIMIZATION_NAIVE;
        break;
    case 2:
        m = LNN_OPTIMIZATION_LOCAL_REORDER;
        break;
    case 3:
        m = LNN_OPTIMIZATION_GLOBAL_REORDER;
        break;
    default:
        m = LNN_OPTIMIZATION_NAIVE;
        //std::cout << "invalid. set default: naive" << std::endl;
        break;
    }

    // Run-time measuring
    timer<properties_timer> t;

    if ( statistics )
    {
        properties_timer rt( statistics );
        t.start( rt );
    }

    //init allocation
    unsigned *allocation;
    if(m == LNN_OPTIMIZATION_LOCAL_REORDER || m == LNN_OPTIMIZATION_GLOBAL_REORDER)
    {
        unsigned amount = base.lines();
        allocation = new unsigned[amount];
        for(unsigned i=0; i<amount; i++)
            allocation[i] = i;
    }

    try {
        //apply algorithm
        switch(m)
        {
        case LNN_OPTIMIZATION_NAIVE:
            copy_circuit(base, circ);
            apply_lnn(circ, base, LNN_OPTIMIZATION_NAIVE, 0);
            break;
        case LNN_OPTIMIZATION_LOCAL_REORDER:
            copy_circuit(base, circ);
            apply_lnn(circ, base, LNN_OPTIMIZATION_LOCAL_REORDER, allocation);
            break;
        case LNN_OPTIMIZATION_GLOBAL_REORDER:
            copy_metadata(base, circ);
            reorder(circ, base, allocation);
            break;
        }
        //clean up
        if(m == LNN_OPTIMIZATION_LOCAL_REORDER || m == LNN_OPTIMIZATION_GLOBAL_REORDER)
        {
            apply_allocation_on_outputs(circ, allocation, base.lines());
            delete allocation;
        }
    }
    catch(std::exception) {
        set_error_message( statistics, boost::str( boost::format( "The circuit isn't a quantum circuit. Please transfer your current circuit into a quantum circuit before apply this optimization.")));
        copy_circuit(base, circ);
        return false;
    }

    return true;
}

void apply_lnn( circuit& circ, const circuit& base, lnn_optimization_mode operation, unsigned allocation[])
{
    unsigned index = 0;
    for( const auto& g : base) {
        if(is_toffoli(g)) {
            //do nothing for this gate
        }
        else {
            gate_details gate_d= calculate_gate_details(g, operation, allocation, base);
            //std::cout << gate_d.distance << std::endl;
            if(gate_d.control_exists)
            {
                unsigned abs_dist = abs(gate_d.distance);
                if(abs_dist > 0)
                {
                    index+=abs_dist;
                    adding_swap_gates(circ, gate_d , index, operation, allocation);

                    if(operation == LNN_OPTIMIZATION_NAIVE)
                        index+=abs_dist;

                    //remove old gate
                    circ.remove_gate_at(index+1);
                }
                else {
                    if(operation == LNN_OPTIMIZATION_LOCAL_REORDER) {
                        circ.insert_gate(index) = gate_d.mod_gate;
                        circ.remove_gate_at(index+1);
                    }
                }
            }
            index++;
        }
    }
}

void adding_swap_gates(circuit& circ, struct gate_details gate_details, unsigned index, lnn_optimization_mode operation, unsigned allocation[])
{
    gate cur_gate = gate_details.mod_gate;
    unsigned target_line = gate_details.target;
    int distance = gate_details.distance;
    unsigned abs_dist = abs(distance);
    //swap infront
    for(unsigned i = 0; i< abs_dist; i++)
    {
        cur_gate.remove_target(target_line);
        unsigned tmptar = target_line;
        if(distance>0)
        {
            tmptar--;
            insert_swap_gate(circ, index-abs_dist+i, target_line, tmptar );
            if(operation == LNN_OPTIMIZATION_LOCAL_REORDER)
            {
                unsigned tmp = allocation[target_line];
                allocation[target_line] = allocation[tmptar];
                allocation[tmptar] = tmp;
            }
            target_line--;
        }
        else
        {
            tmptar++;
            insert_swap_gate(circ, index-abs_dist+i, target_line, tmptar);
            if(operation == LNN_OPTIMIZATION_LOCAL_REORDER)
            {
                unsigned tmp = allocation[target_line];
                allocation[target_line] = allocation[tmptar];
                allocation[tmptar] = tmp;
            }
            target_line++;
        }
        cur_gate.add_target(target_line);
    }

    //now a gate with lnn distance from 0 can be added
    circ.insert_gate(index) = cur_gate;

    //swap behind
    if(operation == LNN_OPTIMIZATION_NAIVE)
    {
        for(unsigned i = 0; i< (unsigned)abs(distance); i++)
        {
            //invers to the other for loop
            if(distance>0 ) {
                insert_swap_gate(circ, index+1+i, target_line, target_line+1 );
                target_line++;
            }
            else {
                insert_swap_gate(circ, index+1+i, target_line, target_line-1);
                target_line--;
            }
        }
    }
}


inline void insert_swap_gate(circuit& circ, unsigned index , unsigned l1, unsigned l2)
{
    gate::control_container lc;
    insert_fredkin(circ,index, lc , l1, l2); //line container is empty
}

struct gate_details calculate_gate_details(gate cur_gate, lnn_optimization_mode operation, unsigned *allocation, const circuit& base) throw (std::exception)
{
    gate_details gate_d;
    gate_d.mod_gate = cur_gate;
    gate_d.control_exists = false;
    gate_d.control = 0;
    gate_d.target = 0;

    //Control_lines
    if(cur_gate.size() > 2) { //more than 1 control line
        // failed
        throw std::exception();
    }
    else {
        // TODO negative controls
        for ( const auto& v : cur_gate.controls() )
        {
            assert( v.polarity() );
            // there is only one control line
            gate_d.control = v.line();
            gate_d.control_exists = true;
        }
    }

    // only one target line
    gate_d.target = cur_gate.targets().front();

    //calc distance
    if(gate_d.control_exists)
    {
        //if we use the heuristic lnn optimization the lines could be on a different position
        if(operation == LNN_OPTIMIZATION_LOCAL_REORDER)
        {
            gate_d.mod_gate.remove_target(gate_d.target);
            gate_d.target = lookup(gate_d.target, allocation, base.lines());
            gate_d.mod_gate.add_target(gate_d.target);

            gate_d.mod_gate.remove_control(make_var(gate_d.control));
            gate_d.control = lookup(gate_d.control, allocation, base.lines());
            gate_d.mod_gate.add_control(make_var(gate_d.control));
        }
        gate_d.distance = (gate_d.target - gate_d.control);
        gate_d.target > gate_d.control ? gate_d.distance-- : gate_d.distance++;
    }
    return gate_d;
}


unsigned lookup(unsigned src, unsigned allocation[], unsigned length)
{
    unsigned index = 0;
    for(; index < length; index++)
    {
        if(allocation[index] == src)break;
    }
    return index;
}

void switch_lines(circuit& circ, const circuit& base, unsigned allocation[] )
{
    //reconfigure gates in base circ
    for( const auto& cg : base) {
        gate_details gate_d = calculate_gate_details(cg, LNN_OPTIMIZATION_GLOBAL_REORDER, 0, base);
        gate g = cg;

        //interferes the current gate the choosen line?
        if(gate_d.target != allocation[gate_d.target]) {
            g.remove_target(gate_d.target);
            g.add_target(allocation[gate_d.target]);
        }
        if(gate_d.control != allocation[gate_d.control]) {
            g.remove_control(make_var(gate_d.control));
            g.add_control(make_var(allocation[gate_d.control]));
        }

        circ.append_gate() = g;
    }
}


void reorder( circuit& circ, const circuit& base, unsigned allocation[]) {

    unsigned impact[base.lines()][2];   //the size of the array is the amount of lines, because each line has an impact
    //init
    for (unsigned i=0; i<base.lines(); i++) {
        impact[i][0] = 0; //impact
        impact[i][1] = i;
    }

    //calculate impact
    for ( const auto& g : base) {
        gate_details gate_d = calculate_gate_details(g, LNN_OPTIMIZATION_GLOBAL_REORDER, 0, base);
        if(gate_d.control_exists)
        {
            int abs_dist = abs(gate_d.distance);
            impact[gate_d.target][0] += abs_dist;
            impact[gate_d.control][0] += abs_dist;
        }
    }

    //for(unsigned i=0; i<base.lines();i++)  std::cout << impact[i][0] << std::endl;

    //bubblesort
    for(unsigned j=0; j<base.lines(); ++j) {
        for(unsigned i = base.lines()-1; i>j; --i) {
            if(impact[i][0] > impact[i-1][0]) {
                unsigned tmp = impact[i][0];
                impact[i][0] = impact[i-1][0];
                impact[i-1][0] = tmp;

                tmp = impact[i][1];
                impact[i][1] = impact[i-1][1];
                impact[i-1][1] = tmp;
            }
        }
    }


    //for(unsigned i=0; i<base.lines();i++) std::cout << impact[i][0] << impact[i][1] << std::endl;

    // resort: set the line with the highest NNC impact in the middle. if the selected line is allready the middle line, select the second highest, and so on. alternating position infront and behind the middle line

    for(unsigned i = 0, pos = base.lines()/2; i<base.lines(); ++i) {
        i%2 ? pos+=i :pos-=i;
        allocation[pos] = impact[i][1];
    }

    switch_lines(circ, base, allocation);

}

bool apply_allocation_on_outputs(circuit& circ, unsigned allocation[], unsigned length) {

    std::vector<std::string>  in = circ.inputs();
    std::vector<std::string>  out = circ.outputs();
    std::vector<std::string> new_out( length );

    if((in.size() != length) || (in.size() != out.size()))
        return false;

    //map: input - allocation
    std::map<std::string, unsigned> input_allocation;
    for(unsigned i = 0; i< in.size(); i++)
        input_allocation[in[i]] = i;

    //map: input - output
    std::map<std::string, std::string> input_output;
    for(unsigned i = 0; i< in.size(); i++)
        input_output[in[i]] = out[i];

    //init out
    for(unsigned i = 0; i<length; i++)
        new_out[i] = out[i];

    //reorder names
    for(unsigned i= 0; i<length; i++)
        if(allocation[i] != i) //position of input changed?
            for(std::map<std::string, unsigned>::const_iterator p_strline = input_allocation.begin(); p_strline!= input_allocation.end(); p_strline++)
                if(allocation[i] == p_strline->second) //find allocation at inputs
                    for(std::map<std::string, std::string>::const_iterator p_strstr = input_output.begin(); p_strstr!= input_output.end(); p_strstr++)
                        if(p_strline->first == p_strstr->first) //find input at outputs
                            new_out[i] = p_strstr->second;

    //convert array to vector
    std::vector<std::string> new_out_v;
    for (unsigned j=0; j<length; ++j) {
        new_out_v.push_back(new_out[j]);
        //   std::cout << new_out[j] << std::endl;
    }

    circ.set_outputs(new_out_v);

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
// End:
