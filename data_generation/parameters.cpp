#include <gsl/gsl_rng.h>

#include "parameters.h"
#include "operations.h"
#include "check.h"
#include "hamiltonian.h"




void Simulation_Parameters::initialize_simluation(int number_of_occupants, double gi, double fi, double gt, double ft, Simulation_Parameters& sim_params){
	const gsl_rng_type * TT;

	num_occupants = number_of_occupants;
	N	      = choose(num_occupants);
	b 	      = new unsigned long long int[N]();
	ham_target    = new double[2*N*N]();
	ham_initial   = new double[2*N*N]();
	start_state   = new double[2*N]();
	jkb_initial   = new double[3]();
	jkb_target    = new double[3]();
	table 	      = new int[num_occupants*N]();
	bonds	      = new int[NUMBER_OF_SITES*NUMBER_OF_SITES]();

	construct_lattice(lattice);
	assign_bonds(bonds, lattice);
	combinations(N, num_occupants,b,table);
	gsl_rng_env_setup();
	TT = gsl_rng_default;
	rng  = gsl_rng_alloc (TT);

	g_initial = gi;
	f_initial = fi;
	g_target = gt;
	f_target = ft;

	j_initial = g_initial;
	k_initial = (1-g_initial)*f_initial;
	b_initial = (1-f_initial)*(1-g_initial);

	j_target = g_target;
	k_target = (1-g_target)*f_target;
	b_target = (1-f_target)*(1-g_target);

	jkb_initial[0] = j_initial;
	jkb_initial[1] = k_initial;
	jkb_initial[2] = b_initial;

	jkb_target[0] = j_target;
	jkb_target[1] = k_target;
	jkb_target[2] = b_target;

	construct_device_hamiltonian_uniform(sim_params, ham_initial, jkb_initial);
	construct_device_hamiltonian_uniform(sim_params, ham_target, jkb_target);

	get_ground_state(N, ham_initial, start_state);
	ground_E = get_ground_E(N, ham_target);
	initial_E = cost(N, start_state, ham_target);

	if(CHECK) check_norm(start_state, N), check_commutator(N, ham_initial, ham_target);
}




void Simulation_Parameters::init_mcbb_params(){
	E_array_fixed_tau = new double[NUM_SEEDS]();
	j_best_fixed_tau  = new double[NUM_SEEDS*2*NUMBER_OF_BANGS]();
	k_best_fixed_tau  = new double[NUM_SEEDS*2*NUMBER_OF_BANGS]();
	b_best_fixed_tau  = new double[NUM_SEEDS*2*NUMBER_OF_BANGS]();

	j_best		  = new double[2*NUMBER_OF_BANGS]();
	k_best  	  = new double[2*NUMBER_OF_BANGS]();
	b_best 		  = new double[2*NUMBER_OF_BANGS]();

	tau		  = TAU_INIT_MCBB;
	old_distance      = 1;
	new_distance      = 1;
	temp_iteration    = 0;
	sweeps_multiplier = 1;
}





void Simulation_Parameters::clear_mcbb_params(){
	delete[] E_array_fixed_tau;
	delete[] j_best_fixed_tau;
	delete[] k_best_fixed_tau;
	delete[] b_best_fixed_tau;
	delete[] j_best;
	delete[] k_best;
	delete[] b_best;
}






void Simulation_Parameters::init_mcdb_params(){
	max_steps_mcdb    = floor(MAX_TAU_MCDB/TIME_STEP_MCDB);

	E_array_fixed_tau = new double[NUM_SEEDS]();
	j_best_fixed_tau  = new double[NUM_SEEDS*max_steps_mcdb]();
	k_best_fixed_tau  = new double[NUM_SEEDS*max_steps_mcdb]();
	b_best_fixed_tau  = new double[NUM_SEEDS*max_steps_mcdb]();

	j_best  	  = new double[max_steps_mcdb]();
	k_best  	  = new double[max_steps_mcdb]();
	b_best 		  = new double[max_steps_mcdb]();

	tau		            = TAU_INIT_MCDB;
	time_step         = TIME_STEP_MCDB;
	total_steps       = floor(tau/time_step);
	old_distance      = 1;
	new_distance      = 1;
	sweeps_multiplier = 1;


	e11  = new double[2*N*N]();
	e01  = new double[2*N*N]();
	e10  = new double[2*N*N]();
}




void Simulation_Parameters::clear_mcdb_params(){
	delete[] E_array_fixed_tau;
	delete[] j_best_fixed_tau;
	delete[] k_best_fixed_tau;
	delete[] b_best_fixed_tau;
	delete[] j_best;
	delete[] k_best;
	delete[] b_best;
	delete[] e10;
	delete[] e01;
	delete[] e11;
}




void Simulation_Parameters::init_mcbf_params(){
	E_array_fixed_tau = new double[NUM_SEEDS]();
	j_best_fixed_tau  = new double[NUM_SEEDS*2*NUMBER_OF_SITES*MAX_EVOLVE_STEPS_MC]();
	k_best_fixed_tau  = new double[NUM_SEEDS*2*NUMBER_OF_SITES*MAX_EVOLVE_STEPS_MC]();
	b_best_fixed_tau  = new double[NUM_SEEDS*NUMBER_OF_SITES*MAX_EVOLVE_STEPS_MC]();

	j_best            = new double[2*NUMBER_OF_SITES*MAX_EVOLVE_STEPS_MC]();
	k_best            = new double[2*NUMBER_OF_SITES*MAX_EVOLVE_STEPS_MC]();
	b_best            = new double[NUMBER_OF_SITES*MAX_EVOLVE_STEPS_MC]();

	tau               = TAU_INIT_MCBF;
	total_steps       = TOTAL_STEPS_INIT_MC;
	time_step         = tau/total_steps;
	old_distance      = 1;
	new_distance      = 1;
	sweeps_multiplier = 1;
}




void Simulation_Parameters::clear_mcbf_params(){
	delete[] E_array_fixed_tau;
	delete[] j_best_fixed_tau;
	delete[] k_best_fixed_tau;
	delete[] b_best_fixed_tau;
	delete[] j_best;
	delete[] k_best;
	delete[] b_best;
}
