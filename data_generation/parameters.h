#ifndef __PARAMS_H_INCLUDED__
#define __PARAMS_H_INCLUDED__

#include <ctime>
#include <math.h>
#include <gsl/gsl_rng.h>




/**
    A header file containing all of the constant parameters involved in the simulation, along with the parameters class which contains all simulation variables
 */



/*SWITCHES FOR EACH TYPE OF SIMULATION METHOD*/
 const bool MCBF      = false;
 const bool MCBB      = false;
 const bool MCDB      = true;
 const bool ADIA      = true;
 const bool SAVE_DATA = true;


/*LATTICE PARAMETERS*/
const bool   PERIODIC              = false;
const bool   UNIFORM_SITES         = true;
const double MAX_PARAM             = 1.0;
const double MIN_PARAM             = 0.0;
const int    DEVICE_DIMENSION      = 2;
const int    NX                    = 2;
const int    NY                    = NX;
const int    NUMBER_OF_SITES       = NX*NY;

/*SIMULATION PARAMETERS*/
const bool   DIAG                  = false;
const int    NUM_SEEDS             = 1;
const double DISTANCE_LIMIT        = 0.01;
const double TAU_INIT              = 0.1;
const double MAX_TAU               = 5.5;
const double TAU_SCALAR            = 1.3;
const double TAU_SCALAR_TINY       = 1.1;
const double TAU_SCALAR_BIG        = 1.5;
const double ACCEPTANCE_PROB       = 0.85;
const double TEMP_EXP_DECAY        = 0.80;
const int    TEMP_DECAY_LIMIT      = 5;
const int    TEMP_DECAY_ITERATIONS = 15;
const int    RANDOM_STATES         = 3;


/*MCBB METHOD PARAMETERS*/
const double MAX_CHANGE_FRACTION_MCBB = 0.9;
const double MIN_CHANGE_FRACTION_MCBB = 0.01;
const int    NUMBER_OF_BANGS          = 3;
const int    SWEEPS_MCBB              = 30;

/*MCDB METHOD PARAMETERS*/
const int    MAX_STEPS_MCDB    = 64;
const int    MIN_STEPS_MCDB    = 16;
const int    SWEEPS_MCDB       = 50;
const double STEPS_CRUNCH_MCDB = 1.0;

/*MCBF METHOD PARAMETERS*/
const double MAX_CHANGE_MCBF_INIT = 0.5*(MAX_PARAM-MIN_PARAM);
const double MIN_CHANGE_MCBF_INIT = 0.1*(MAX_PARAM-MIN_PARAM);
const int    SWEEPS_MC            = 50;
const int    TOTAL_STEPS_INIT_MC  = 5;
const int    MAX_EVOLVE_STEPS_MC  = 4*TOTAL_STEPS_INIT_MC;
const int    ARRAY_SCALAR         = 2;
const int ROW        = 0;  //The change_array_mcbf variables. If 0 -> This method will not be used, if n -> use this method on every nth iteration of change_array_mcbf
const int COL        = 0;
const int ALTROW     = 0;
const int ALTCOL     = 0;
const int ALTROW2    = 0;
const int ALTCOL2    = 0;
const int ROWK       = 1; //Rows correspond to all sites for a fixed time, columns to all times for a fixed site.
const int ROWJ       = 2;
const int ROWB       = 0;
const int COLK       = 0;
const int COLJ       = 0;
const int COLB       = 0;
const int SINGLE     = 0;
const int VARIATIONS = (ROW && 1)+(COL && 1)+(ALTROW && 1)+(ALTCOL && 1)+(ALTROW2 && 1)+(ALTCOL2 &&  1)+(SINGLE &&  1)+(ROWK && 1)+(ROWJ && 1)+(ROWB && 1)+(COLK && 1)+(COLJ && 1)+(COLB && 1);

/*ADIABATIC METHOD PARAMETERS*/
const double TIME_STEP_ADIA = 1/1000.0;



//Tests and debugging help
const bool CHECK            = false;
const bool PRINT            = true;
const bool PRINT_COMMUTATOR = false;




class Simulation_Parameters{
public:
	std::clock_t start;
	double *ham_target, *ham_initial, *init_state, *target_state, *state, *jkb_initial, *jkb_target, *E_array_fixed_tau, *evolved_state_fixed_tau, *best_evolved_state, *j_best_fixed_tau, *k_best_fixed_tau, *b_best_fixed_tau, *j_best, *k_best, *b_best, *e11, *e01, *e10, *j_best_scaled, *k_best_scaled, *b_best_scaled, *saved_states;
	double ground_E, j_initial, k_initial, b_initial, j_target, k_target, b_target, tau, tau_previous, time_step, temperature,initial_temperature, best_E,initial_E, old_distance, new_distance, temp_distance, init_target_dot_squared, evolved_target_dot_squared, duration;
	unsigned long long int *b;
	int lattice[NX][NY], num_occupants,N,*table, *bonds, seed, total_steps, temp_iteration, max_steps_mcdb, sweeps_multiplier, total_sweeps;
	gsl_rng * rng;



	 /**
	 Constructs the lattice, generates the dimension of the quantum system, assigns the bonds, initializes the rng rng,
	 and creates the b and table arrays (see func 'combinations' in operations.h for more info)

	 @param number_of_occupants occupants on the lattice
	 @param sim_params contains all of the variables for the simulation
	 */
	void initialize_lattice(int number_of_occupants, Simulation_Parameters& sim_params);



	/**
	Generates the target and initial hamiltonian given the jkb initial and target values. Also grabs the ground state from
	the initial hamiltonian and the ground state from the target hamiltonian, which serve as the initial state in the
	evolution process and the target energy in the monte carlo simulations

	@param ji j_initial
	@param ki k_initial
	@param jt j_target
	@param kt k_target
	@param sim_params contains all of the variables for the simulation
	*/
	void initialize_hamiltonians(double ji, double ki, double jt, double kt, Simulation_Parameters& sim_params);


	/**
		Initializing/clearing the arrays for the simulations
	*/
	void init_mcbb_params();
	void clear_mcbb_params();
	void init_mcdb_params();
	void clear_mcdb_params();
	void init_mcbf_params();
	void clear_mcbf_params();
};




#endif
