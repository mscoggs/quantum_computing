#ifndef __WRITE_H_INCLUDED__
#define __WRITE_H_INCLUDED__

#include <string>

#include "parameters.h"



void save_mcbf_data(Simulation_Parameters& sim_params);


/**
    Saves the monte carlo bang bang simulation data for a fixed tau

    @param sim_params contains all of the variables for the simulation
*/
void save_mcbb_data(Simulation_Parameters& sim_params);


/**
    Saves the monte carlo bang bang simulation data for a fixed tau

    @param sim_params contains all of the variables for the simulation
*/
void save_mcdb_data(Simulation_Parameters& sim_params);


/**
    Saves the adiabatic simulation data

    @param sim_params contains all of the variables for the simulation
*/
void save_adiabatic_data(Simulation_Parameters& sim_params);


/**
    Generates a path to save the data that depends on the type of simulation and the jkb values of the start and target hamiltonians

    @param sim_params contains all of the variables for the simulation
    @param type the type of simulation
*/
std::string make_path(Simulation_Parameters sim_params, std::string type);


/**
    A sub function of all the save_blank_data functions, which saves the start and target jkb values along with some info about the lattice

    @param sim_params contains all of the variables for the simulation
    @param path the location of the file that will be appended with the ham parameters
*/
void save_hamiltonian_parameters(Simulation_Parameters sim_params,std::string path);


/**
    Saves the 'gap' data, the difference between the first excited state and the ground state for different f and g inputs.


    @param sim_params contains all of the variables for the simulation
    @param f the hamiltonia's f parameter
    @param g the hamiltoian's g parameter
    @param gap the difference between the first excited state and the ground state
    @param L the 'dimension' of the f-g plane.
*/
void save_gap_data(Simulation_Parameters& sim_params, double *j, double *k, double *gap, int L);
#endif
