#include <iomanip>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <complex>
#include <math.h>
#include <sys/time.h>
#include <gsl/gsl_rng.h>

//g++ -o qubit_simulation qubit_simulation.cpp -llapack -lblas -lgsl
//./qubit_simulation

//Physical Parameters
#define PERIODIC false
#define DEVICE_DIMENSION 2
#define T 1
#define V 2
#define J_START 0.5
#define K_START 0.1
#define B_START 0.7



//Non-Physical Parameters
#define SEED 1
#define NX 2
#define NY NX
#define NUM_SITES NX*NY
#define DEVICE_SAME_MODEL true
#define DIAG false
#define RANDOM_STATES 3
#define TEMP_DECAY_LIMIT 20
#define DIFFERENCE_LIMIT 0.0001
/*
#define TAU_MAX 1.5
#define TAU_STEP 0.2
#define MAX_STEPS (int) ceil(TAU_MAX/TAU_STEP)*TOTAL_STEPS_STEP
#define TOTAL_STEPS_STEP 2
*/
#define MAX_TAU 10
#define TAU_INIT 0.2
#define TOTAL_STEPS_INIT 5
#define TAU_SCALAR 1.2
#define MAX_STEPS (int) ceil((MAX_TAU*TOTAL_STEPS_INIT)/TAU_INIT)
#define MAX_TAU_STEPS (int) ceil(log(MAX_TAU)/TAU_SCALAR)
#define SWEEPS 20
#define CHANGE 0.01
#define ACCEPTANCE_PROB 0.5
#define EXP_DECAY 0.9
#define TEMP_DECAY_ITERATIONS 20
#define UNIFORM true
#define ARRAY_SCALAR 2

#ifdef DEVICE_SAME_MODEL
#define UPJ 1
#define UPK 1
#define UPB 1
#define LOWJ 0
#define LOWK 0
#define LOWB 0
#endif

//Debugging Help
#define CHECK true
#define PRINT true
#define PRINT_MC_ITERATIONS false
#define PRINT_COMMUTATOR false
#define PRINTBEST false
#define EVOLVE_DATA false
#define TAU_DATA true

//The change_array variables. 0 -> This method will not be used, #>0 -> use this method on every #th iteration of change_array
#define ROW 1
#define COL 0
#define ALTROW 0
#define ALTCOL 0
#define ALTROW2 0
#define ALTCOL2 0
#define ROWK 0
#define ROWJ 0
#define ROWB 0
#define COLK 0
#define COLJ 0
#define COLB 0
#define SINGLE 0
#define VARIATIONS (ROW && 1)+(COL && 1)+(ALTROW && 1)+(ALTCOL && 1)+(ALTROW2 && 1)+(ALTCOL2 &&  1)+(SINGLE &&  1)+(ROWK && 1)+(ROWJ && 1)+(ROWB && 1)+(COLK && 1)+(COLJ && 1)+(COLB && 1)




using namespace std;


//Homemade functions
double monte_carlo(int *table, unsigned long long int *b, int num_electrons, int N, int lattice[][NX], double*ham_target, double* psi_start, double temp, double tau, int total_steps, double time_step, gsl_rng * r, double *k_best,double *j_best, double *b_best, double ground_E, double *best_E_array);
void evolve(int *table, unsigned long long int *b,int num_electrons,int N,  int lattice[][NX], double *psi, double *k_array, double *j_array, double *b_array, int *bonds, int total_steps, double time_step);
double cost(double *psi, double *ham_target, int N);
double calc_initial_temp(int *table, unsigned long long int *b, int num_electrons, int N, int lattice[][NX], double*ham_target, int total_steps, double time_step, double *psi_start, gsl_rng *r, int seed, double *jkb_initial, double *jkb_target);
void construct_device_hamiltonian(int *table, unsigned long long int *b,int num_electrons,int N, double *ham_dev, int lattice[][NX],double *k_array, double *j_array, double *b_array, int *bonds, int index, int D);
void construct_device_hamiltonian_uniform(int *table, unsigned long long int *b,int num_electrons,int N, double *ham_dev, int lattice[][NX],double *jkb, int D);
void construct_model_hamiltonian(int *table, unsigned long long int *b,int num_electrons,int N, double *ham_target,  int lattice[][NX], int D);
void diag_hermitian_real_double(int N,  double *A, double *Vdag,double *D);
void exp_diaganolized_mat(double *ham_real, double *Vdag, double* D, int N, double time_step);
void exp_general_complex_double(int N, double *A, double *B);
void matrix_vector_mult(double *exp_matrix, double *psi, int N);
int hop(unsigned long long int b, unsigned long long int *v,int n, int j);
int find(int N,unsigned long long int *v, unsigned long long int *b);
void get_ground_state(int N, double *ham, double* ground_state);
double get_ground_E(int N, double *ham);
unsigned long long int choose(int num_electrons);
int combinations ( int num_electrons, unsigned long long int *b,int *tab, int N);
void assign_bonds(int *bonds, int lattice[][NX]);
void copy_arrays(int N, double *k_array, double *j_array, double* b_array,  double* k_best,  double* j_best, double* b_best, int total_steps);
void init_arrays(double *k_array, double *j_array,double *b_array, gsl_rng *r, int total_steps);
void change_array(double *k_array, double *j_array, double *b_array, int random_row, int random_col, double change_pm, int i, int total_steps);
void change_row(double *k_array, double *j_array,double *b_array, int row, double change, bool k, bool j, bool b, int jump, int offset);
void change_col(int total_steps,double *k_array,double *j_array,double *b_array, int col, double change,bool k, bool j, bool b, int jump, int offset);
void change_single(double *k_array,double *j_array,double *b_array, int row,int col, double change);
void scale_arrays(int total_steps, double* j_best, double* k_best,double* b_best);
void construct_lattice(int lattice[][NX]);
int get_neighbors(int site, int *neighbors, int lattice[][NX], int D);
double get_random(double lower, double upper, gsl_rng *r);
void check_commutator(int N, double* A, double* B);
void check_norm(double* psi, int N);
void check_unitary(double* ham, int N);
void check_hermicity(double* ham_target, int N);
void check_weights(double* state, double* ham, int N);
void export_tau_data(double *tau_array, double *best_E_array, int total_steps, double tau, double ground_E, double *jkb_initial, double *jkb_target, int index);
void export_evolve_data(int *table, unsigned long long int *b, int num_electrons, int N,double* j_best, double* k_best, double* b_best, double tau, double time_step, int total_steps, int lattice[][NX],double* psi_start, double ground_E, double* ham_target);
void export_evolve_calc(int *table, unsigned long long int *b,int num_electrons,int N,  int lattice[][NX], double *psi, double *k_array, double *j_array, double *b_array, int *bonds, int total_steps, double time_step, double* E_list, double* T_list, double* ham_target);
void print_best_arrays(double *k_array, double *j_array, double *b_array, int totals_step);
void print_vector(double* psi,int N);
void print_hamiltonian(double* hamiltonian, int N);
void print_hamiltonianR(double *hamiltonian, int N);
void print_E(double ground_E, double best_E);
void test_function();


//All of the linear algebra functions
extern "C" int zgemm_(char *TRANSA, char *TRANSB, int *M, int *N, int *K, double *ALPHA, double *Z, int *LDA, double *X, int *LDB, double *BETA, double *Y, int *LDC); //complex matrix*matrix mult, odd indices hold imaginary values.
extern "C" int zgemv_(char *TRANS, int *M, int *N,double *ALPHA,double *A, int *LDA, double *X, int *INCX, double *BETA, double *Y, int *INCY); //complex matrix-vector mult, odd indices hold imaginary values.
extern "C" int dsyev_(char *JOBZ, char *UPLO, int *N, double *Vdag, int *LDA, double *D, double *WORK, int *LWORK, int *INFO);//diagonalization, returns the eigenvectors in Vdag and eigenvalues in D.
extern "C" int zgetrf_ (int *M, int *N, double *D, int *LDA, int *IPIV, int *INFO);//A matrix factorization?
extern "C" int zgetrs_( char *TRANS, int *N, int *NRHS,double *D, int *LDA, int *IPIV,double *E, int *LDB, int *INFO);//solves a system of equations
extern "C" void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);
extern "C" void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);
extern "C" int dgemm_(char *TRANSA, char *TRANSB, int *M, int *N, int *K, double *ALPHA, double *Z, int *LDA, double *X, int *LDB, double *BETA, double *Y, int *LDC); //real values matrix*matrix mult
extern "C" double zdotc_(int *N, double*ZX,int *INCX, double *ZY, int *INCY);//dots the complex conjugate of ZX with ZY




int main (int argc, char *argv[])
{
	int *table,*bonds,lattice[NY][NX],num_electrons,N,i,j, z, t,x, total_steps, y, ts, p, index, seed=0;
	unsigned long long int *b;
	double *ham_target, *ham_initial, *k_best, *j_best, *b_best, *psi_start, ground_E, initial_temp=1, best_E, time_step, tau, *best_E_array, *tau_array, *jkb_initial, *jkb_target, g_initial, f_initial, g_target, f_target, difference, tau_min;


	gsl_rng_env_setup();
	const gsl_rng_type * TT = gsl_rng_default;
	gsl_rng * r  = gsl_rng_alloc (TT);

	construct_lattice(lattice);



	jkb_initial = (double *) malloc(3*sizeof(double));
	jkb_target = (double *) malloc(3*sizeof(double));
	k_best = (double *) malloc(2*NUM_SITES*MAX_STEPS*sizeof(double));
	j_best = (double *) malloc(2*NUM_SITES*MAX_STEPS*sizeof(double));
	b_best = (double *) malloc(NUM_SITES*MAX_STEPS*sizeof(double));
	tau_array = (double *) malloc(MAX_TAU_STEPS*sizeof(double));
	best_E_array = (double *) malloc(MAX_TAU_STEPS*sizeof(double));




	//change this later to cycle through all electrons
	for(i=3;i<4;i++)
	{
		num_electrons=2;
		N=choose(num_electrons);

		b = (unsigned long long int*) malloc(N*sizeof(unsigned long long int));
		table=(int*) malloc(num_electrons*N*sizeof(int));
		ham_target = (double *) malloc (2*N*N*sizeof (double));
		ham_initial = (double *) malloc(2*N*N*sizeof(double));
		psi_start = (double *) malloc(2*N*sizeof(double));
		combinations (num_electrons,b,table, N);


		for(g_target=0.75;g_target<1.01;g_target+=0.25)
		{
			for(f_target=.3;f_target<1.1;f_target+=0.5)
			{
				for(seed=1; seed<4; seed++)
				{
					gsl_rng_set(r, seed);

					f_initial = f_target;
					g_initial= 0.1;
					jkb_initial[0] = g_initial;
					jkb_initial[1] = (1-g_initial)*f_initial;
					jkb_initial[2] = (1-f_initial)*(1-g_initial);
					construct_device_hamiltonian_uniform(table, b, num_electrons, N, ham_initial, lattice, jkb_initial, DEVICE_DIMENSION);

					jkb_target[0] = g_target;
					jkb_target[1] = (1-g_target)*f_target;
					jkb_target[2] = (1-f_target)*(1-g_target);
					construct_device_hamiltonian_uniform(table, b, num_electrons, N, ham_target, lattice, jkb_target, DEVICE_DIMENSION);

					if(CHECK) check_commutator(N, ham_initial, ham_target);

					//getting the ground state from the initial
					get_ground_state(N, ham_initial,psi_start);
					ground_E = get_ground_E(N, ham_target);
					if(CHECK) check_norm(psi_start, N);


					for (x=0;x<2*NUM_SITES*MAX_STEPS;x++) k_best[x] =0, j_best[x] = 0;
					for (x=0;x<NUM_SITES*MAX_STEPS;x++) b_best[x] =0;

					index = 1;
					tau_array[0] = 0;
					init_arrays(k_best, j_best, b_best, r, MAX_STEPS);

					tau = TAU_INIT;
					tau_min = TAU_INIT;
					total_steps = TOTAL_STEPS_INIT;
					time_step = tau/((double) total_steps);
					bool ground_state = false;
					bool binary_search = false;
					double tau_max = tau;
					int p;
					while(tau<MAX_TAU)
					{
						for(p=0;p<N;p++) printf("B: %llu\n",b[p]);

						combinations(num_electrons,b,table, N);


						initial_temp = calc_initial_temp(table, b, num_electrons, N, lattice, ham_target,total_steps, time_step, psi_start, r, seed, jkb_initial, jkb_target);
						best_E = monte_carlo(table, b, num_electrons, N, lattice, ham_target, psi_start, initial_temp,tau, total_steps, time_step,r, k_best, j_best, b_best, ground_E, best_E_array);

						difference = best_E-ground_E;
						if(difference < 0.000001) ground_state =true;

						if(ground_state)//the ground state has been reached, performing binary search

						{

							printf("\n\n\n\nSTARTING BINARY SEARCH METHOD\n\n\n\n");
							if(difference < 0.000001)//still in the ground state, backtrack tau
							{
								printf("\n\nbacktracking\n\n");
								if((tau-tau_min) < TAU_INIT/10) //Some arbitrary limit to stop the binary search
								{
									printf("\nGROUND STATE REACHED\n\n\n");
									goto end_loop;
								}

								tau_max = tau;
								tau = (tau_max+tau_min)/2;
								time_step = tau/((double) total_steps);
								printf("\n\nNEW TAU:%f\n\n", tau);
							}
							else
							{

								printf("\n\nTOo much backtracking\n\n");
								tau_array[index] = tau;
								best_E_array[index] = best_E;
								index++;

								tau_min = tau;
								tau = (tau_min+tau_max)/2;
								time_step = tau/((double) total_steps);

								printf("\n\ntau_max:%f\n\n", tau_max);
								printf("\n\ntau_min:%f\n\n", tau_min);

								printf("\n\nNEW TAU:%f\n\n", tau);

							}

						}
						else //operation as normal, pushing tau forward to look for the GS
						{
							tau_array[index] = tau;
							best_E_array[index] = best_E;
							index ++;
							tau_min = tau;
							tau = tau*TAU_SCALAR;
							total_steps = floor(TOTAL_STEPS_INIT*(tau/TAU_INIT));
							time_step = tau/((double) total_steps);
						}
					}
					end_loop:
					if(TAU_DATA) export_tau_data(tau_array, best_E_array, total_steps,tau, ground_E, jkb_initial, jkb_target, index);
				}
			}
		}
		free(ham_initial), free(ham_target), free(b), free(table), free(psi_start);
	}
	exit(0);
}




double monte_carlo(int *table, unsigned long long int *b, int num_electrons, int N, int lattice[][NX], double*ham_target, double* psi_start, double temperature, double tau, int total_steps, double time_step, gsl_rng * r, double *k_best,double *j_best, double *b_best, double ground_E, double *best_E_array)
/*monte_carlo the values in the j, b, and k list in order to produce the lowest energy (expectation value) between the final state (psi), produced by evolve, and the model hamiltonian. This is done by randomly selecting one row of each list, making a slight change, then determining if the new energy is lower than the old. If the new energy is greater than the old, keep with probability exp(delta_E/Temp)*/
{
	int i=0,j=0, random_row=0, random_col, proposal_accepted=0,proposal_count=0, poor_acceptance_count=0,*bonds;
	double *psi, *k_array, *j_array,*b_array,*k_temp, *j_temp, *b_temp, acceptance_rate=0, E_old=0, E_new=0,best_E=0, change_pm=0;
	bonds = (int*) malloc(NUM_SITES*NUM_SITES*sizeof(int));
	k_array = (double *) malloc(2*NUM_SITES*total_steps*sizeof(double));
	j_array = (double *) malloc(2*NUM_SITES*total_steps*sizeof(double));
	b_array = (double *) malloc(NUM_SITES*total_steps*sizeof(double));
	k_temp = (double *) malloc(2*NUM_SITES*total_steps*sizeof(double));
	j_temp = (double *) malloc(2*NUM_SITES*total_steps*sizeof(double));
	b_temp = (double *) malloc(NUM_SITES*total_steps*sizeof(double));
	psi = (double *) malloc (2*N*sizeof(double));

	assign_bonds(bonds, lattice);
	memcpy(psi,psi_start, 2*N*sizeof(double));

	if(CHECK) check_norm(psi, N);
	if(PRINTBEST) printf("\nPrinting the best K-J-B arrays from Monte_Carlo"), print_best_arrays(k_best, j_best, b_best, total_steps);
	copy_arrays(N, k_best, j_best,b_best, k_array, j_array, b_array, total_steps);
	evolve(table,b,num_electrons,N,lattice, psi, k_array,j_array,b_array,bonds, total_steps, time_step);
	if(PRINTBEST) print_vector(psi,N);
	if(PRINTBEST) print_hamiltonian(ham_target, N);
	best_E = cost(psi, ham_target, N);

	if(total_steps == TOTAL_STEPS_INIT) best_E_array[0] = best_E;
	E_old = best_E;
	if(PRINT) printf("Pre-Monte_Carlo Expectation:   %f\n", best_E);

	for (i=0;i<TEMP_DECAY_ITERATIONS;i++)
	{
		proposal_accepted = 0;
		proposal_count = 0;

		for (j=0; j<SWEEPS*total_steps;j++)
		{
			copy_arrays(N, k_array, j_array, b_array, k_temp, j_temp,b_temp,total_steps);//a temporary array, used in the undoing of the changes

			change_pm = pow(-1,(int)floor(get_random(0,10,r))) * CHANGE;
			random_row = floor(get_random(0,total_steps,r));
			random_col = floor(get_random(0,NUM_SITES*2,r));

			change_array(k_array,j_array,b_array,random_row,random_col,change_pm,j, total_steps);
			memcpy(psi,psi_start, 2*N*sizeof(double));//resetting psi

			evolve(table,b,num_electrons,N,lattice, psi, k_array,j_array,b_array, bonds, total_steps, time_step);
			E_new = cost(psi, ham_target, N);
			if (E_new<best_E) best_E=E_new, E_old=E_new,  copy_arrays(N, k_array, j_array, b_array, k_best, j_best,b_best, total_steps);
			else if (E_new<E_old) E_old=E_new;
			else if (get_random(0,1,r)<exp(-(E_new-E_old)/(temperature))) E_old=E_new, proposal_accepted++, proposal_count++;
			else copy_arrays(N, k_temp, j_temp, b_temp, k_array, j_array, b_array,total_steps),proposal_count++;//undoing the change
		}
		//printf("\n");

		acceptance_rate = (double)proposal_accepted/proposal_count;

		if(PRINT_MC_ITERATIONS) printf("accepted_props:%3i |total_props:%3i |AcceptanceRate: %3.4f |Best  Expectation: %3.6f\n", proposal_accepted, proposal_count,acceptance_rate,best_E);

		if(acceptance_rate<0.011) poor_acceptance_count++;
		else poor_acceptance_count = 0;

		if(poor_acceptance_count>TEMP_DECAY_LIMIT)
		{
			printf("NO MC PROGRESS FOR %i TEMP CHANGE ITERATIONS, TERMINATING\n", TEMP_DECAY_LIMIT);
		       	goto end_loop;
		}
		temperature=temperature*EXP_DECAY;
	}
	end_loop:

	if (PRINTBEST) printf("\nPrinting the best K-J-B arrays from main"), print_best_arrays(k_best, j_best, b_best, total_steps);
	if(PRINT) printf("Post-Monte_Carlo Expectation:  %f\n", best_E);
	if(PRINT) print_E(ground_E, best_E);
	if(EVOLVE_DATA) export_evolve_data(table, b, num_electrons, N, j_best, k_best, b_best, tau, time_step, total_steps, lattice, psi_start, ground_E, ham_target);

	free(k_array), free(j_array), free(b_array),free(k_temp), free(j_temp), free(b_temp),free(psi), free(bonds);
	return best_E;
}




void evolve(int *table, unsigned long long int *b,int num_electrons,int N, int lattice[][NX], double *psi, double *k_array, double *j_array, double *b_array, int *bonds, int total_steps, double time_step)
/*evolve a starting state, psi, by acting on it with exp(ham_dev*-i*time_step). The resulting state is updated as psi and the process repeats total_steps times (tau/total_steps) until the final psi state is produced. The function contains two methods for calculating exp(ham_dev*-i+time_step), one is a diagonalization method, the other a Pade approximation*/
{
	int i,j;
	double *ham_dev,*ham_t_i, *ham_real,*exp_matrix,*D, *Vdag;
	ham_dev = (double *) malloc (2*N*N*sizeof (double));
	exp_matrix = (double *) malloc (2*N*N*sizeof (double));
	ham_t_i = (double *) malloc (2*N*N*sizeof (double));
	ham_real = (double *) malloc(N*N*sizeof(double));
	Vdag = (double*) malloc(N*N*sizeof(double));
	D = (double*) malloc(N*sizeof(double));

	for (i=0; i<total_steps;i++)
	{
		construct_device_hamiltonian(table, b, num_electrons, N, ham_dev, lattice, k_array, j_array, b_array, bonds,i,DEVICE_DIMENSION);

		if(CHECK) check_norm(psi, N);

		if(DIAG)
		{
			for (j=0; j<N*N; j++) ham_real[j]=0.0,Vdag[j]=0.0;
			for (j=0; j<N; j++) D[j]=0.0;
			for (j=0; j<N*N; j++) ham_real[j] = ham_dev[2*j];//converting an all-real-valued complex matrix into just real matrix
			diag_hermitian_real_double(N, ham_real,Vdag, D);
			exp_diaganolized_mat(ham_dev, Vdag, D, N, time_step);//This function exponentiates D to e^(-i*time_step*D)

			if(CHECK) check_unitary(ham_dev, N);
			matrix_vector_mult(ham_dev,psi, N);
		}
		else
		{
			for (j=0; j<N*N*2; j++) exp_matrix[j] = 0.0, ham_t_i[j]=0.0;
			for (j=0; j<N*N; j++)
			{
				ham_t_i[2*j+1] = (ham_dev[2*j]*-time_step); //multiplying by -i*dt for the Pade approximation
				ham_t_i[2*j] = (ham_dev[2*j+1]*time_step);
			}
			exp_general_complex_double(N, ham_t_i, exp_matrix);
			if(CHECK) check_unitary(exp_matrix, N);
			matrix_vector_mult(exp_matrix, psi, N);
		}
		if(CHECK) check_norm(psi, N);
		//print_vector(psi,N);
	}
	free(ham_dev), free(ham_t_i), free(exp_matrix), free(D), free(Vdag),free(ham_real);
}




double cost(double *psi, double *ham_target, int N)
/*Computing the expectation value between ham_target and psi, <psi|ham_target|psi>*/
{

	int i=0,INCX = 1,INCY = 1,j;
	double *psi_conj, *psi_temp, result=0, resulti=0, norm;
	psi_conj = (double*) malloc (N*2*sizeof(double));
	psi_temp = (double*) malloc (N*2*sizeof(double));
	memcpy(psi_conj, psi, 2*N*sizeof(double));
	memcpy(psi_temp, psi, 2*N*sizeof(double));


	if(CHECK) check_norm(psi_temp, N);
	if(CHECK) check_weights(psi_temp, ham_target, N);

	matrix_vector_mult(ham_target, psi_temp, N);//H*psi, the operator acting on the ket, storing in psi
	result = zdotc_(&N, psi_conj, &INCX, psi_temp, &INCY);//psi* * psi, the dot between the complex conj and the result of H*psi

	free(psi_conj), free(psi_temp);
	return result;
}




double calc_initial_temp(int *table, unsigned long long int *b, int num_electrons, int N, int lattice[][NX], double*ham_target, int total_steps, double time_step, double *psi_start, gsl_rng *r, int seed, double *jkb_initial, double *jkb_target)
/*Finding an good initial temp that allows an average increase acceptance probability of about ACCEPTANCE_PROB (typically 0.8). Choosing an initial temperature that allows the chance of accepting j,b, and k_array values which increase the expectation value to be ~80%, https://www.phy.ornl.gov/csep/mo/node32.html*/
{
	if(PRINT) printf("\n\n\n\n...Calculating initial temperature based on %i random starting states...\n", RANDOM_STATES);
	int *bonds, i=0,j=0, random_row=0,random_col=0, start_state=0, count=0;
	double *psi,*psi_random, *k_array, *j_array,*b_array, E_old=0, E_new=0,sum=0, change_pm=0, initial_temp=0;
	bonds = (int*) malloc(NUM_SITES*NUM_SITES*sizeof(int));
	k_array = (double *) malloc(2*NUM_SITES*total_steps*sizeof(double));
	j_array = (double *) malloc(2*NUM_SITES*total_steps*sizeof(double));
	b_array = (double *) malloc(NUM_SITES*total_steps*sizeof(double));
	psi_random = (double *) malloc (2*N*sizeof(double));
	psi = (double *) malloc (2*N*sizeof(double));

	assign_bonds(bonds, lattice);
	for (i=0; i<N*2;i++) psi[i]=0.0;
	for (j=0;j<RANDOM_STATES;j++)
	{
		for (i=0; i<N*2;i++) psi_random[i] =0.0;
		start_state = floor(get_random(0,N,r));

		psi_random[start_state*2] = 1;
		memcpy(psi,psi_random, 2*N*sizeof(double));

		init_arrays(k_array, j_array, b_array,r, total_steps);

		evolve(table,b,num_electrons,N,lattice, psi, k_array,j_array,b_array, bonds, total_steps, time_step);
		E_old = cost(psi, ham_target, N);

		for (i=0; i<SWEEPS;i++)
		{
			change_pm = pow(-1,(int)floor(get_random(0,10,r))) * CHANGE;
			random_row = floor(get_random(0,total_steps,r));
			random_col = floor(get_random(0,NUM_SITES*2,r));

			change_array(k_array,j_array,b_array,random_row,random_col,change_pm,i, total_steps);
			memcpy(psi,psi_random, 2*N*sizeof(double));//resetting psi
			evolve(table,b,num_electrons,N,lattice, psi, k_array,j_array,b_array, bonds, total_steps, time_step);
			E_new = cost(psi, ham_target, N);

			if (E_new>E_old) sum += (E_new-E_old), count++;
			E_old=E_new;
		}
	}
	free(k_array), free(j_array), free(b_array), free(psi), free(psi_random), free(bonds);
	initial_temp = -(sum/(count*log(ACCEPTANCE_PROB)));

	if(PRINT){
		printf("#######################################################################\n");

		printf("#######################################################################");
		printf("\n ELECTRONS:         %2i || DIMENSION:      %4i || SEED:         %4i ||\n TAU:             %4.2f || TOTAL_STEPS:    %4i || TIME_STEP:   %4.3f ||\n INIT_TEMP:     %5.4f || TOTAL_SWEEPS:   %4i || TEMP_DECAYS:  %4i ||\n J_INIT:         %4.3f || K_INIT:        %4.3f || B_INIT:      %4.3f ||\n J_TARGET:       %4.3f || K_TARGET:      %4.3f || B_TARGET:    %4.3f ||\n",num_electrons, N,seed,time_step*total_steps,total_steps, time_step,initial_temp, total_steps*SWEEPS, TEMP_DECAY_ITERATIONS, jkb_initial[0], jkb_initial[1], jkb_initial[2], jkb_target[0], jkb_target[1], jkb_target[2]);
		printf("\nPSI_START: "), print_vector(psi_start, N);
		printf("#######################################################################\n");
	}

	return initial_temp;
}




void construct_device_hamiltonian(int *table, unsigned long long int *b,int num_electrons,int N, double *ham_dev, int lattice[][NX],double *k_array, double *j_array, double *b_array, int *bonds,  int index, int D)
/*constructiing the hamiltonina matrix for the device amiltonian, using the index-ith row of each j, b, and k array*/
{
	int i=0,ii=0,j=0,x=0,y=0,state=0,site=0,sign=0,bond=0,neighbor_count=0,*neighbors;
	unsigned long long int *v,comparison=0;
	v = (unsigned long long int*) malloc(sizeof(unsigned long long int));
	neighbors = (int*) malloc(4*sizeof(int));
	for (i=0; i<N*N*2; i++) ham_dev[i] = 0.0;

	for (i=0;i<N;i++)
	{

		for (j=0;j<num_electrons;j++)//The J term calculation
		{

			site=table[i*num_electrons+j];
			neighbor_count = get_neighbors(site, neighbors, lattice, D);
			for (ii=0; ii<neighbor_count; ii++)
			{
				if (((1ULL<<(neighbors[ii]-1))&b[i])==0)//making sure neighbor is not occupied, otherwise nothing happens
				{

					hop(b[i], v,site, neighbors[ii]);

					state=find(N,v, b);

					bond = bonds[(site-1)*NUM_SITES+neighbors[ii]-1]-1;
					ham_dev[(N*i+state-1)*2] -= j_array[index*NUM_SITES*2+bond];
				}
			}
		}

		for (j=1;j<(NUM_SITES);j++)//The K term calculation
		{
			site=j;
			neighbor_count = get_neighbors(site, neighbors, lattice, D);
			for (ii=0; ii<neighbor_count;ii++)
			{
				if (neighbors[ii] > site)
				{
					sign = -1;
					comparison = (1ULL<<(neighbors[ii]-1))+(1ULL<<(site-1));
					if((comparison&b[i])==comparison || (comparison&b[i])==0) sign = 1;
					bond = bonds[(site-1)*NUM_SITES+neighbors[ii]-1]-1;
					ham_dev[((N*i)+i)*2] += k_array[index*NUM_SITES*2+bond]*sign;
				}
			}
		}

		for (j=0; j<NUM_SITES;j++)//The B term calculation
		{
			sign = -1;
			if(((1ULL<<j)&b[i])>0) sign=1;
			ham_dev[((N*i)+i)*2] += b_array[index*NUM_SITES+j]*sign;
		}
	}
	b = (unsigned long long int*) malloc(N*sizeof(double));
	free(neighbors);// free(v);
	if(CHECK) check_hermicity(ham_dev, N);
}



void construct_device_hamiltonian_uniform(int *table, unsigned long long int *b,int num_electrons,int N, double *ham_dev, int lattice[][NX],double *jkb, int D)
/*constructiing the hamiltonina matrix for the device amiltonian, using the index-ith row of each j, b, and k array*/
{
	int i=0,ii=0,j=0,x=0,y=0,state=0,site=0,sign=0,bond=0,neighbor_count=0,*neighbors;
	unsigned long long int *v,comparison=0;
	v = (unsigned long long int*) malloc(sizeof(unsigned long long int));
	neighbors = (int*) malloc(4*sizeof(int));
	for (i=0; i<N*N*2; i++) ham_dev[i] = 0.0;

	for (i=0;i<N;i++)
	{

		for (j=0;j<num_electrons;j++)//The J term calculation
		{

			site=table[i*num_electrons+j];
			neighbor_count = get_neighbors(site, neighbors, lattice, D);
			for (ii=0; ii<neighbor_count; ii++)
			{

				if (((1ULL<<(neighbors[ii]-1))&b[i])==0)//making sure neighbor is not occupied, otherwise nothing happens
				{
					hop(b[i], v,site, neighbors[ii]);
					state=find(N,v, b);
					ham_dev[(N*i+state-1)*2] -= jkb[0];
				}
			}
		}

		for (j=1;j<(NUM_SITES);j++)//The K term calculation
		{
			site=j;
			neighbor_count = get_neighbors(site, neighbors, lattice, D);
			for (ii=0; ii<neighbor_count;ii++)
			{
				if (neighbors[ii] > site)
				{
					sign = -1;
					comparison = (1ULL<<(neighbors[ii]-1))+(1ULL<<(site-1));
					if((comparison&b[i])==comparison || (comparison&b[i])==0) sign = 1;
					ham_dev[((N*i)+i)*2] += jkb[1]*sign;
				}
			}
		}

		for (j=0; j<NUM_SITES;j++)//The B term calculation
		{
			sign = -1;
			if(((1ULL<<j)&b[i])>0) sign=1;
			ham_dev[((N*i)+i)*2] += jkb[2]*sign;
		}
	}
	free(neighbors), free(v);
	if(CHECK) check_hermicity(ham_dev, N);
}


void construct_model_hamiltonian(int *table, unsigned long long int *b,int num_electrons,int N, double *ham_mod, int lattice[][NX], int D)
/*Constructing the hamiltonian matrix for the model hamiltonian*/
{
	int i,ii,j,x,y,state,site,neighbor,sign,neighbor_count, *neighbors;
	unsigned long long int *v,comparison;
	v = (unsigned long long int*) malloc(sizeof(unsigned long long int));
	neighbors = (int*) malloc(sizeof(int)*4);
	for (i=0; i<N*N*2; i++) ham_mod[i] = 0;

	for (i=0;i<N;i++)
	{
		for (j=0;j<num_electrons;j++)//The T term calculation
		{
			site=table[i*num_electrons+j];

			neighbor_count = get_neighbors(site, neighbors, lattice, D);
			for (ii=0; ii<neighbor_count; ii++)
			{
				if (((1ULL<<(neighbors[ii]-1))&b[i])==0)//checking if the neighbor is occupied
				{
					sign = hop(b[i], v,site, neighbors[ii]);
					if (sign==0) sign=1;
					else sign=-1;
					state=find(N,v, b);
					ham_mod[(N*i+state-1)*2] -= (T*sign);
				}
			}
		}
		for (j=1;j<(NUM_SITES);j++)//The V term calculation
		{
			site=j;
			neighbor_count = get_neighbors(site, neighbors, lattice, D);
			for (ii=0; ii<neighbor_count;ii++)
			{
				if (neighbors[ii] > site)
				{
					sign = -1;
					comparison = (1ULL<<(neighbors[ii]-1))+(1ULL<<(site-1));
					if((comparison&b[i])==comparison || (comparison&b[i])==0) sign = 1;
					ham_mod[((N*i)+i)*2] += sign*V;
				}
			}
		}
	}
	free(neighbors), free(v);
	if(CHECK) check_hermicity(ham_mod, N);
}




void diag_hermitian_real_double(int N,  double *A, double *Vdag,double *D)
/*diagonalizing an real square matrix A. Stores the eigenvectors in Vdag and the eigenvalues in D*/
{
	char JOBZ='V',UPLO='U';
	int LDA=N,LWORK=-1, INFO;
	double *WORK;
	WORK=(double*) malloc(sizeof(double));

	memcpy(Vdag,A,N*N*sizeof(double));

	dsyev_(&JOBZ, &UPLO, &N, Vdag, &LDA, D, WORK, &LWORK, &INFO );
	if (INFO !=0) printf("\n\n\nDIAGONALIZATION ERROR, INFO = %i\n\n\n", INFO);
       	LWORK=WORK[0];
       	free(WORK);
       	WORK=(double*) malloc(LWORK*sizeof(double));

       	dsyev_(&JOBZ, &UPLO, &N, Vdag, &LDA, D, WORK, &LWORK, &INFO );
       	if (INFO !=0) printf("\n\n\nDIAGONALIZATION ERROR, INFO = %i\n\n\n", INFO);
       	free(WORK);
}




void exp_diaganolized_mat(double *ham, double *Vdag, double* D, int N, double time_step)
/*calculating the exponential of a diagonalized decomposition where the matrix A = Vdag*D*Vdag_inv. calculating Vdag*exp(D)*Vdag_inv =exp(A), storing the result in ham*/
{
	char TRANSA = 'N', TRANSB = 'N';
	int i, *IPIV, LWORK=N*N, INFO, LDA=N, LDB=N, LDC=N;
	double *exp_D, *temp_mat, *Vdag_z, *Vdag_z_inv, *WORK,ALPHA[2], BETA[2];
	ALPHA[0]=1.0, ALPHA[1]=0.0;
	BETA[0]=0.0, BETA[1]=0.0;

	IPIV = (int*) malloc(N*sizeof(int));
	WORK = (double*) malloc(LWORK*sizeof(double));
	exp_D = (double *) malloc (2*N*N*sizeof (double));
	temp_mat = (double *) malloc (2*N*N*sizeof (double));
	Vdag_z = (double *) malloc (2*N*N*sizeof (double));
	Vdag_z_inv = (double *) malloc (2*N*N*sizeof (double));

	for (i =0; i<N*N*2; i++) exp_D[i] = 0, temp_mat[i] = 0,Vdag_z[i]=0, Vdag_z_inv[i]=0, ham[i]=0;
	for (i =0; i<N*N; i++) Vdag_z[2*i] = Vdag[i];
	for (i =0; i<N; i++)
	{
		exp_D[2*(i*N+i)] = cos(-time_step*D[i]);
	       	exp_D[2*(i*N+i)+1] = sin(-time_step*D[i]);
	}

	dgetrf_(&N,&N,Vdag,&N,IPIV,&INFO);
	dgetri_(&N,Vdag,&N,IPIV,WORK,&LWORK,&INFO);//inverting vdag

	for (i =0; i<N*N; i++) Vdag_z_inv[2*i] = Vdag[i];
	zgemm_(&TRANSA, &TRANSB, &N, &N, &N, ALPHA, Vdag_z, &LDA, exp_D, &LDB, BETA, temp_mat, &LDC); //matrix mult
	zgemm_(&TRANSA, &TRANSB, &N, &N, &N, ALPHA, temp_mat, &LDA, Vdag_z_inv, &LDB, BETA, ham, &LDC); //matrix mult
	free(exp_D),free(temp_mat),free(Vdag_z), free(Vdag_z_inv), free(IPIV), free(WORK);
}




void exp_general_complex_double(int N, double *A, double *B)
/*Using the Pade Approximation to calulate exp(A), where A is a complex matrix storing real values at even indices and their imaginary parts at index +1. Storing the result in B*/
{
	int M=N,K=N,ii,jj,kk,s,p,q,INFO,LDA=N,LDB=N,LDC=N,NRHS=N, *IPIV;
	double *row_norm,*X,*Y,*Z,*E,*D,norm,c, ALPHA[2], BETA[2];
	char TRANSA='N',TRANSB='N',TRANS='N';
	ALPHA[0]=1.0,ALPHA[1]=0.0;
	BETA[0]=0.0,BETA[1]=0.0;

	row_norm=(double*) malloc(N*sizeof(double));
	X=(double*) malloc(2*N*N*sizeof(double));
	Y=(double*) malloc(2*N*N*sizeof(double));
	Z=(double*) malloc(2*N*N*sizeof(double));
	E=(double*) malloc(2*N*N*sizeof(double));
	D=(double*) malloc(2*N*N*sizeof(double));
	IPIV=(int*) malloc(N*sizeof(int));
	memcpy(Z,A,2*N*N*sizeof(double));

	for(ii=0;ii<N;ii++)
	{
		row_norm[ii]=0.0;
		for (jj=0;jj<N;jj++) row_norm[ii]=row_norm[ii]+cabs(A[2*(jj+N*ii)]+I*A[2*(jj+N*ii)+1]);

	}
	norm=row_norm[0];
	for(ii=1;ii<N;ii++) if (row_norm[ii]>norm) norm=row_norm[ii];
	s=(int) floor(log2(norm)+2);
	if (s<0) s=0;

	for(ii=0;ii<2*N*N;ii++) Z[ii]=Z[ii]/pow(2.0,s);

	memcpy(X,Z,2*N*N*sizeof(double));
	c=0.5,p=1,q=6;

	for(ii=0;ii<2*N*N;ii++) E[ii]=c*Z[ii];
	for(ii=0;ii<2*N*N;ii++) D[ii]=-c*Z[ii];
	for(ii=0;ii<N;ii++)E[2*(ii+N*ii)]=E[2*(ii+N*ii)]+1.0;
	for(ii=0;ii<N;ii++)D[2*(ii+N*ii)]=D[2*(ii+N*ii)]+1.0;

	for (kk=2;kk<=q;kk++)
	{
		c = c * (q-kk+1) / (kk*(2*q-kk+1));
		zgemm_ (&TRANSA, &TRANSB, &M, &N, &K, ALPHA, Z, &LDA, X, &LDB, BETA, Y, &LDC); //matrix mult
		memcpy(X,Y,2*N*N*sizeof(double));
		for(ii=0;ii<2*N*N;ii++) Y[ii]=c*X[ii];
		for(ii=0;ii<2*N*N;ii++) E[ii]=E[ii]+Y[ii];
		if (p==1)for(ii=0;ii<2*N*N;ii++) D[ii]=D[ii]+Y[ii];
		else for(ii=0;ii<2*N*N;ii++) D[ii]=D[ii]-Y[ii];
		p=abs(1-p);
	}

	zgetrf_ (&M, &N, D, &LDA, IPIV, &INFO);//getting a factorization of D
	if (INFO !=0) printf("\n\n\nERROR, INFO = %i\n\n\n", INFO);
	zgetrs_( &TRANS, &N, &NRHS,D,    &LDA, IPIV,E, &LDB, &INFO);//solving a system of equations
	if (INFO !=0) printf("\n\n\nERROR, INFO = %i\n\n\n", INFO);

	for (kk=1;kk<=s;kk++)
	{
		memcpy(X,E,2*N*N*sizeof(double));
		zgemm_ (&TRANSA, &TRANSB, &M, &N, &K, ALPHA, E, &LDA, X, &LDB, BETA, Y, &LDC);//matrixmultiplication
		if (INFO !=0) printf("\n\n\nERROR, INFO = %i\n\n\n", INFO);
		memcpy(E,Y,2*N*N*sizeof(double));
	}
	memcpy(B,E,2*N*N*sizeof(double));

	free(row_norm),free(X),free(Y),free(Z),free(E),free(D),free(IPIV);
}




void matrix_vector_mult(double *matrix, double *psi, int N)
/*Matrix vector multiplication, psi=matrix*psi*/
{
	char TRANS = 'N';
	int i,INCX = 1,INCY = 1,LDA = N,M=N;
	double *result, ALPHA[2], BETA[2];
	ALPHA[0]=1.0,ALPHA[1]=0.0;
	BETA[0]=0.0,BETA[1]=0.0;
	result = (double *) malloc (N*2*sizeof(double));

	zgemv_(&TRANS, &M, &N,ALPHA,matrix, &LDA, psi, &INCX, BETA, result, &INCY);
	memcpy(psi,result, 2*N*sizeof(double));

	free(result);
}




int hop(unsigned long long int b, unsigned long long int *v,int n, int j)
/*given a state b generates the state v obtained by hopping from site b[n] to site j (which should not be in b), and outputs the fermionic sign*/
{
	unsigned long long int i,x,y;
	int z_count = 0;

//	printf("site %i\n", n);
	//printf("neihbor %i\n", j);
//	printf("b %llu\n",b);


	x = (1ULL << (n-1)) + (1ULL << (j-1));
	for (i=n;i<j-1;i++)  if((1ULL<<i) & (b)) z_count++;
	y = (x ^ b);
	memcpy(v, &y, sizeof(unsigned long long int));
	return z_count%2;
}




int find(int N,unsigned long long int *v,unsigned long long int *b)
/*find the position of a given combination v (vector of length k) in the table of all combinations tab*/
{
	int first, last, mid;
	first=0;
	last=N-1;


	while (first <= last)
	{
		mid = (int) ((first + last) / 2.0);
		if (*v > b[mid]) first = mid + 1;
		else if (*v < b[mid]) last = mid - 1;
		else return mid+1;
	}
}





void check_commutator(int N, double* A, double* B)
{
	char TRANSA = 'N', TRANSB = 'N';
	int i, *IPIV, LWORK=N*N, INFO, LDA=N, LDB=N, LDC=N;
	double *C, *AB, *BA ,ALPHA[2], BETA[2];
	bool commute = true;
	ALPHA[0]=1.0, ALPHA[1]=0.0;
	BETA[0]=0.0, BETA[1]=0.0;


	C = (double*) malloc(2*N*N*sizeof(double));
	BA = (double*) malloc(2*N*N*sizeof(double));
	AB = (double*) malloc(2*N*N*sizeof(double));
	for (i =0; i<N*N*2; i++) C[i] = 0, AB[i] = 0, BA[i]=0;

	zgemm_(&TRANSA, &TRANSB, &N, &N, &N, ALPHA, A, &LDA, B, &LDB, BETA, AB, &LDC); //matrix mult
	zgemm_(&TRANSA, &TRANSB, &N, &N, &N, ALPHA, B, &LDA, A, &LDB, BETA, BA, &LDC); //matrix mult
	for (i =0; i<N*N*2; i++) C[i] = AB[i] - BA[i];
	for (i =0; i<N*N*2; i++) if(C[i] < -0.001 || 0.001 < C[i]) commute = false;
	if(commute) printf("\n\n\nWARNING: THE TARGET AND INITIAL HAMILTONIAN COMMUTE\n\n\n");
	if(PRINT_COMMUTATOR) print_hamiltonian(C, N);

//	print_hamiltonian(A, N);
//	print_hamiltonian(B, N);
	free(C), free(BA), free(AB);
}







void get_ground_state(int N, double *ham, double *ground_state)
/*Get the ground state of a hamiltonian matrix by finding the smallest eigen_value, the getting the eigenvector associated with that eigenvalue. Copying the eigenvector the ground_state*/
{
	int i, min_i,j;
	double *D, *Vdag, *ham_real, min_E=1000;
	Vdag = (double*) malloc(N*N*sizeof(double));
	D = (double*) malloc(N*sizeof(double));
	ham_real = (double*) malloc(N*N*sizeof(double));
	for(i=0;i<N*N;i++) ham_real[i] = ham[2*i];

	diag_hermitian_real_double(N, ham_real,Vdag, D);


	for(i=0; i<2*N; i++) ground_state[i] = 0.0;
	for(i=0; i<N; i++) if(D[i]<min_E){ min_E = D[i]; min_i = i;}
	for(i=0; i<N; i++) ground_state[i*2] = Vdag[min_i*N+i];

	free(Vdag), free(D), free(ham_real);
}




double get_ground_E(int N, double *ham)
/*Finding the ground energy, the smallest eigenvalue of the matrix*/
{
	int i,j;
	double *D, *Vdag, *ham_real,min_E=1000;
	Vdag = (double*) malloc(N*N*sizeof(double));
	D = (double*) malloc(N*sizeof(double));
	ham_real = (double*) malloc(N*N*sizeof(double));
	for(i=0;i<N*N;i++) ham_real[i] = ham[2*i];

	diag_hermitian_real_double(N, ham_real,Vdag, D);

	for(i=0; i<N; i++) if(D[i]<min_E) min_E = D[i];

	free(Vdag), free(D), free(ham_real);
	return min_E;
}




unsigned long long int choose(int num_electrons)
/*calculating (NUM_SITES choose num_electrons) = NUM_SITES!/((NUM_SITES-num_electrons)!*num_electrons!)*/
{
	int i;
	unsigned long long int c;
	c=1ULL;
	for (i=0;i<num_electrons;i++) c=c*(NUM_SITES-i);
	for (i=0;i<num_electrons;i++) c=c/(i+1);
	return c;
}




int combinations ( int num_electrons,  unsigned long long int *b,int *tab, int N)
/*Returning the combinations of NUM_SITES-choose-elctrons, each represented by NUM_SITES slots of a binary number, where the bit the furthest to the right is site 1, and
furthest to the left is site NUM_SITES. 1 respresents occupied, 0 empty. Finding number of unordered combinations of num_electrons in NUM_SITES.*/
{
	unsigned long long int x,y;
	int i,c,d;
	x=0ULL;
	for (i=0;i<num_electrons;i++) x=x+(1ULL<<i);
	b[0]=x;
	c=0;
	d=0;
	i=0;
	while ((c<NUM_SITES)&& (d<num_electrons))
	{
		if (x & (1ULL<<c))
		{
			tab[i*num_electrons+d]=c+1;
			d++;
		}
		c++;
	}
	for (i=1;i<N;i++)
	{
		y = (x | (x - 1)) + 1;
		x = y | ((((y & -y) / (x & -x)) >> 1) - 1);
		b[i]=x;
		c=0;
		d=0;
		while ((c<NUM_SITES)&& (d<num_electrons))
		{
			if (x & (1ULL<<c))
			{
				tab[i*num_electrons+d]=c+1;
				d++;
			}
			c++;
		}
	}
}




void assign_bonds(int *bonds, int lattice[][NX])
/*Attaching a bond number for each bond, allowing assignment of a specific bond-value between two neighbors, where this value is stored in j and k lists*/
{
	int bond_num, site,site2, j, neighbor_count, *neighbors;
	bond_num = 1;
	neighbors = (int*) malloc(4*sizeof(int));
	for(j=0;j<NUM_SITES*NUM_SITES;j++) bonds[j]=0;

	for(site=1; site<NUM_SITES+1;site++)
	{
		neighbor_count = get_neighbors(site, neighbors, lattice, 2);
		for(j=0;j<neighbor_count;j++)
		{
			site2 = neighbors[j];
			if(site2>site)
			{
				bonds[(site-1)*NUM_SITES+site2-1]=bond_num;
				bonds[(site2-1)*NUM_SITES+site-1]=bond_num;
				bond_num++;
			}
		}
	}
	free(neighbors);
}




void copy_arrays(int N, double *k_array, double *j_array, double* b_array,  double* k_to,  double* j_to, double* b_to, int total_steps)
/*storing the updated k, j, and b values*/
{
	memcpy(k_to, k_array, 2*NUM_SITES*total_steps*sizeof(double));
	memcpy(j_to, j_array, 2*NUM_SITES*total_steps*sizeof(double));
	memcpy(b_to, b_array, NUM_SITES*total_steps*sizeof(double));
}




void init_arrays(double *k_array,double *j_array,double *b_array,gsl_rng * r, int total_steps)
/*Initializng the values of the k, j, and b lists which hold the values of the constants for each site (b_array) and between each bond (k_array and j_array)*/
{
	int i,j;
	double upj=1.0,lowj=-1.0,upk=1.0,lowk=-1.0,upb=1.0,lowb=-1.0, random_val1, random_val2, random_val3;
	if(DEVICE_SAME_MODEL) upj=UPJ,lowj=LOWJ,upk=UPK,lowk=LOWK,upb=UPB,lowb=LOWB;
	if(UNIFORM)
	{
		for (i=0; i<total_steps;i++)
		{

			random_val1 = get_random(lowj,upj,r);
			random_val2 = get_random(lowk,upk,r);
			random_val3 = get_random(lowb,upb,r);
			for (j=0; j<NUM_SITES*2; j++)
			{
				j_array[i*NUM_SITES*2+j] = random_val2;
				k_array[i*NUM_SITES*2+j] = random_val3;
			}
			for (j=0; j<NUM_SITES; j++)
			{
				b_array[i*NUM_SITES+j]= random_val1;
			}
		}
	}
	else{
		for (i=0; i<total_steps;i++)
		{
			for (j=0; j<NUM_SITES*2; j++)
			{
				j_array[i*NUM_SITES*2+j] = get_random(lowj,upj,r);
				k_array[i*NUM_SITES*2+j] = get_random(lowk,upk,r);
			}
			for (j=0; j<NUM_SITES; j++)
			{
				b_array[i*NUM_SITES+j]= get_random(lowb,upb,r);
			}
		}
	}
	if(PRINTBEST) printf("\nPrinting the best K-J-B arrays from init_arrays"), print_best_arrays(k_array, j_array, b_array, total_steps);
}




void change_array(double *k_array, double *j_array, double *b_array, int random_row, int random_col, double change_pm, int i, int total_steps)
/*changing the j, k, and b arrays. Use the defines at start of program to determine which manipulation functions will be used, and in what order*/
{
	int mod = VARIATIONS;
	if(i%mod==ROW-1) change_row(k_array,j_array,b_array,random_row,change_pm,true, true,true, 1, 0);
	else if(i%mod==COL-1) change_col(total_steps,k_array,j_array,b_array,random_col,change_pm, true, true, true, 1, 0);
	else if(i%mod==ALTROW-1) change_row(k_array,j_array,b_array,random_row,change_pm, true, true, true ,2, 0);
	else if(i%mod==ALTCOL-1) change_col(total_steps,k_array,j_array,b_array,random_col,change_pm, true, true, true, 2, 0);
	else if(i%mod==ALTROW2-1) change_row(k_array,j_array,b_array,random_row,change_pm, true, true, true, 2, 1);
	else if(i%mod==ALTCOL2-1) change_col(total_steps,k_array,j_array,b_array,random_col,change_pm, true, true, true, 2, 1);
	else if(i%mod==ROWK-1) change_row(k_array,j_array,b_array,random_row,change_pm, true, false, false, 1, 0);
	else if(i%mod==ROWJ-1) change_row(k_array,j_array,b_array,random_row,change_pm, false, true, false, 1, 0);
	else if(i%mod==ROWB-1) change_row(k_array,j_array,b_array,random_row,change_pm, false, false, true, 1, 0);
	else if(i%mod==COLK-1) change_col(total_steps,k_array,j_array,b_array,random_col,change_pm, true, false, false, 2, 1);
	else if(i%mod==COLJ-1) change_col(total_steps,k_array,j_array,b_array,random_col,change_pm, false, true, false, 2, 1);
	else if(i%mod==COLB-1) change_col(total_steps,k_array,j_array,b_array,random_col,change_pm, false, false, true, 2, 1);
	else if(i%mod==SINGLE-1) change_single(k_array,j_array,b_array,random_row,random_col,change_pm);
	else printf("NOPE\n");
}



void change_row(double *k_array,double *j_array,double *b_array, int row, double change, bool k, bool j, bool b, int jump, int offset)
/*Changing all of the lists by a value change at at the row number, row. Used in the monte_carlo function. offset gives the starting element, jump gives the amount to increase each increment.
  bool k, j, and b determine if the k,j,and b lists will be changes. Bounds of the values that the lists can take are given by the #defines if using a device that's the same as model, +-1 otherwise*/
{
	int i;
	double upj=1.0,lowj=-1.0,upk=1.0,lowk=-1.0,upb=1.0,lowb=-1.0;
	if(DEVICE_SAME_MODEL) upj=UPJ,lowj=LOWJ,upk=UPK,lowk=LOWK,upb=UPB,lowb=LOWB;


  if(k) for (i=offset; i<NUM_SITES*2; i+=jump)
	{
		//printf("ROW: %i", row);
		//printf("NUM_SITES*2*row+i: %i\n", NUM_SITES*2*row+i);
		if ((lowk < k_array[NUM_SITES*2*row+i] + change) &&  (k_array[NUM_SITES*2*row+i] + change < upk)) k_array[NUM_SITES*2*row+i] += change;
	}
	if(j) for (i=offset; i<NUM_SITES*2; i+=jump)
	{
		if ((lowj < j_array[NUM_SITES*2*row+i] + change) && (j_array[NUM_SITES*2*row+i] + change < upj)) j_array[NUM_SITES*2*row+i] += change;
	}
	if(b) for (i=offset; i<NUM_SITES; i+=jump)
	{
		if ((lowb < b_array[NUM_SITES*row+i] + change) && (b_array[NUM_SITES*row+i] + change < upb)) b_array[NUM_SITES*row+i] += change;
	}
}




void change_col(int total_steps,double *k_array,double *j_array,double *b_array, int col, double change,bool k, bool j, bool b, int jump, int offset)
/*Changing all of the lists by a value change at at the col number, col. Used in the monte_carlo function. offset gives the starting element, jump gives the amount to increase each increment.
  bool k, j, and b determine if the k,j,and b lists will be changes. Bounds of the values that the lists can take are given by the #defines if using a device that's the same as model, +-1 otherwise*/
{
	int i;
	double upj=1.0,lowj=-1.0,upk=1.0,lowk=-1.0,upb=1.0,lowb=-1.0;
	if(DEVICE_SAME_MODEL) upj=UPJ,lowj=LOWJ,upk=UPK,lowk=LOWK,upb=UPB,lowb=LOWB;

	if(k) for (i=offset; i<total_steps; i+=jump)
	{
		if ((lowk < k_array[NUM_SITES*2*i+col] + change) && (k_array[NUM_SITES*2*i+col] + change  < upk)) k_array[NUM_SITES*2*i+col] += change;
	}
	if(j) for (i=offset; i<total_steps; i+=jump)
	{
		if ((lowj < j_array[NUM_SITES*2*i+col] + change) && (j_array[NUM_SITES*2*i+col] + change < upj)) j_array[NUM_SITES*2*i+col] += change;
	}
	if(b) for (i=offset; i<total_steps; i+=jump)
	{
		if ((lowb < b_array[NUM_SITES*i+(int)floor(col/2.0)] + change) && (b_array[NUM_SITES*i+(int)floor(col/2.0)] + change < upb)) b_array[NUM_SITES*i+(int)floor(col/2.0)] += change;
	}
}




void change_single(double *k_array,double *j_array,double *b_array, int row,int col, double change)
/*Changing a single element in each array, at column col, row row*/
{
	double upj=1.0,lowj=-1.0,upk=1.0,lowk=-1.0,upb=1.0,lowb=-1.0;
	if(DEVICE_SAME_MODEL) upj=UPJ,lowj=LOWJ,upk=UPK,lowk=LOWK,upb=UPB,lowb=LOWB;

	if ((lowj < j_array[NUM_SITES*2*row+col] + change) && (j_array[NUM_SITES*2*row+col] + change < upj)) j_array[NUM_SITES*2*row+col] += change;
 	if ((lowk < k_array[NUM_SITES*2*row+col] + change) && (k_array[NUM_SITES*2*row+col] + change < upk)) k_array[NUM_SITES*2*row+col] += change;
	if ((lowb < b_array[NUM_SITES*row+(int)floor(col/2.0)] + change) && (b_array[NUM_SITES*row+(int)floor(col/2.0)] + change  < upb)) b_array[NUM_SITES*row+(int)floor(col/2.0)] += change;
}



void scale_arrays(int total_steps, double* j_best, double* k_best,double* b_best)
{
	int i, j;
	for(i=total_steps;i>0;i--)
	{
		for(j=0;j<NUM_SITES*2;j++)
		{

			k_best[NUM_SITES*2*(ARRAY_SCALAR*i-1)+j] = 	k_best[NUM_SITES*2*(i-1)+j];
			k_best[NUM_SITES*2*(ARRAY_SCALAR*i-2)+j] = 	k_best[NUM_SITES*2*(i-1)+j];
			j_best[NUM_SITES*2*(ARRAY_SCALAR*i-1)+j] = 	j_best[NUM_SITES*2*(i-1)+j];
			j_best[NUM_SITES*2*(ARRAY_SCALAR*i-2)+j] = 	j_best[NUM_SITES*2*(i-1)+j];

		}
		for(j=0;j<NUM_SITES;j++)
		{

			b_best[NUM_SITES*(ARRAY_SCALAR*i-1)+j] = 	b_best[NUM_SITES*(i-1)+j];
			b_best[NUM_SITES*(ARRAY_SCALAR*i-2)+j] = 	b_best[NUM_SITES*(i-1)+j];

		}
	}
}





void construct_lattice(int lattice[][NX])
/*Build a NX*NX lattice, with sites 1 through NX*NX listed in a snaking pattern*/
{
	int x,y;
	for (x=0;x<NX;x++)
	{
		if (x%2 ==1) for (y=0;y<NX;y++) lattice[x][y] = (NX*x)+y+1;
		else for (y=0;y<NX;y++) lattice[x][NX-1-y] = NX*x+y+1;
	}
}




int get_neighbors(int site, int *neighbors, int lattice[][NX], int D)
/*Gets the neighbors of a site, returning all 4 neighbors if open_boundry coditions are true, otherwise just closed-boudnary neighbors*/
{
	int x,y,i,count=0;
	if (D==2)
	{
		for (x=0;x<NX;x++) for (y=0;y<NX;y++) if(site == lattice[x][y]) goto end_loop;//Finding the coordinates
		end_loop: for(i=0;i<4;i++) neighbors[i]=0;

		if (PERIODIC)
		{
			neighbors[0] = lattice[(x+1)%NX][y];
			neighbors[1] = lattice[(x+NX-1)%NX][y];
			neighbors[2] = lattice[x][(y+1)%NY];
			neighbors[3] = lattice[x][(y+(NY-1))%NY];
			count = 4;
		}
		else
		{
			if (x+1<NX) neighbors[count] = lattice[x+1][y], count++;
			if (x>0) neighbors[count] = lattice[x-1][y], count++;
			if (y+1<NX) neighbors[count] = lattice[x][y+1], count++;
			if (y>0) neighbors[count] = lattice[x][y-1], count++;
		}
		return count;
	}
	else if (D==1)
	{
		if (PERIODIC)
		{
			neighbors[0] = (site%(NX*NY))+1;
			neighbors[1] = ((site+(NX*NY)-2)%(NX*NY))+1;
			count = 2;
		}
		else
		{
			if (site<(NX*NY)) neighbors[count] = site+1, count++;
			if (site>1) neighbors[count] = site-1, count++;
		}
		return count;
	}
	printf("\n\n\nERROR! NO NEIGHBORS FOUND.\n\n\n");
	exit(0);

}




double get_random(double lower, double upper, gsl_rng * r)
/*randomly generating a number in [lower, upper), including lower, up to upper (but not including)*/
{
	double u;

	/*varying seed, based on time
	const gsl_rng_type * TT;
	gsl_rng * r;
	struct timeval tv;
	double seed;
	gsl_rng_env_setup();
	gettimeofday(&tv,0);
	seed = tv.tv_usec;
	TT = gsl_rng_default;
	r = gsl_rng_alloc (TT);
	gsl_rng_set(r, seed);
	u = gsl_rng_uniform(r);
	gsl_rng_free (r);
	return u*(upper-lower)+lower;
	*/

	u = gsl_rng_uniform(r);
	return u*(upper-lower)+lower;
}





void check_norm(double* psi, int N)
{
	int i,j=0;
	double sum=0;
	for(i=0;i<N*2; i+=2)
	{
		sum+= psi[i]*psi[i]+(psi[i+1]*psi[i+1]);

	}
	if(sum>1.000000001 or sum<0.99999999) printf("\n\n\nNORM ERROR, SIZE: %f\n\n\n", sum), exit(0);
}




void check_unitary(double* ham, int N)
{
	int i,j, *IPIV, LWORK=N*N, INFO, LDA=N, LDB=N, LDC=N;
	double *ham_t,*unitary, *WORK,ALPHA[2], BETA[2];
	char TRANSA = 'C', TRANSB = 'N';


	ALPHA[0]=1.0, ALPHA[1]=0.0;
	BETA[0]=0.0, BETA[1]=0.0;

	ham_t = (double*) malloc(2*N*N*sizeof(double));
	unitary = (double*) malloc(2*N*N*sizeof(double));
	memcpy(ham_t, ham, sizeof(double)*2*N*N);
	zgemm_(&TRANSA, &TRANSB, &N, &N, &N, ALPHA, ham_t, &LDA, ham, &LDB, BETA, unitary, &LDC); //matrix mult

	for(i=0;i<N;i++)
	{
		unitary[2*(i*N+i)] = unitary[2*(i*N+i)] -1;
	}

	for(i=0;i<N*N*2;i++)
	{
		if(unitary[i] < -0.00000001 or unitary[i] > 0.00000001) printf("\n\n\nERROR, NON UNITARY ELEMENTS AT %i, VALUE: %f\n\n\n", i, unitary[i]), exit(0);

	}
	free(ham_t), free(unitary);
}





void check_hermicity(double* ham, int N)
{
	int i,j;
	for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
		{
			if(abs(ham[2*(j*N+i)] - ham[2*(i*N+j)]) > 0.00001) printf("\n\n\nERROR, NON HERMITIAN ELEMENT AT i,j: %i,%i\nElement_ij = %10.7f\nElement_ji = %10.7f\n\n\n", i*N+j, j*N+i, ham[2*(i*N+j)], ham[2*(j*N+i)]), exit(0);
		}
	}
}





void check_weights(double* state, double* ham, int N)
{
	int i,j;
	double *D, *Vdag, *ham_real;
	Vdag = (double*) malloc(N*N*sizeof(double));
	D = (double*) malloc(N*sizeof(double));
	ham_real = (double*) malloc(N*N*sizeof(double));
	for (i=0;i<N*N;i++) ham_real[i] = ham[2*i];

	diag_hermitian_real_double(N, ham_real,Vdag, D);

	double sum_real, sum_im, c_squared_sum=0;

	for(j=0;j<N;j++)
	{
		sum_real=0;
		sum_im=0;
		for(i=0; i<N; i++)
		{
			sum_real += state[2*i]*Vdag[j*N+i];
			sum_im += -state[2*i+1]*Vdag[j*N+i];
		}
		c_squared_sum += sum_real*sum_real+sum_im*sum_im;
	}
	if(c_squared_sum>1.00001 or c_squared_sum <0.999999) printf("\n\n\nERROR, BAD WEIGHTS\n\n\n"), exit(0);
	free(Vdag), free(D), free(ham_real);
}


void export_tau_data(double *tau_array, double *best_E_array, int total_steps, double tau, double ground_E,double *jkb_initial, double *jkb_target, int index)
{
	int i;
	ofstream file;
	file.open("data/exp_difference_vs_tau/MAX_TAU=" + to_string(MAX_TAU) + "_SEED=" + to_string(SEED) + ".txt");
	char output[200];
	sprintf(output, "MAX TAU  |  MAX STEPS\n %5f    |   %5i\n", tau, total_steps);
	file << output;


	file << "j_initial= " << jkb_initial[0] << "\n";
	file << "k_initial= " << jkb_initial[1] << "\n";
	file << "b_initial= " << jkb_initial[2] << "\n";
	file << "j_target= " << jkb_target[0] << "\n";
	file << "k_target= " << jkb_target[1] << "\n";
	file << "b_target= " << jkb_target[2] << "\n";
	file << "\n\n\ntimes= [";
	for(i=0;i<index;i++) file << tau_array[i] << ", ";

	file << "]\n\n\nE_best = [";
	for(i=0;i<index;i++) file << (best_E_array[i]) << ", ";
	file << "]";


	file << "]\n\n\nE_ground = [";
	for(i=0;i<index;i++) file << ground_E << ", ";
	file << "]";
	file << "\n\nplot(times, E_dif)";

	file.close();
}



void export_evolve_data(int *table, unsigned long long int *b, int num_electrons, int N,double* j_best, double* k_best, double* b_best, double tau, double time_step, int total_steps, int lattice[][NX],double* psi_start, double ground_E, double* ham_target)
{
	ofstream file;
	file.open("data/exp_difference_vs_time/E_VS_T___ToteTime=" + to_string(tau) + "_ToteStep" + to_string(total_steps) + "_SEED=" + to_string(SEED) + ".txt");
	char output[200];
	sprintf(output, "TOTAL TIME  |  TIME STEP  |  TOTAL STEPS\n %5f      |%10f   |   %5i\n", tau, time_step,total_steps);
	file << output;

	int *bonds, i, j;
	double *k_avg, *b_avg, *j_avg, *E_list, *T_list, *psi, j_av,k_av,b_av;

	bonds = (int*) malloc(NUM_SITES*NUM_SITES*sizeof(int));
	E_list = (double*) malloc(total_steps*sizeof(double));
	T_list = (double*) malloc(total_steps*sizeof(double));
	psi = (double*) malloc(N*2*sizeof(double));
	k_avg = (double*) malloc(total_steps*sizeof(double));
	j_avg = (double*) malloc(total_steps*sizeof(double));
	b_avg = (double*) malloc(total_steps*sizeof(double));

	assign_bonds(bonds, lattice);
	memcpy(psi,psi_start, 2*N*sizeof(double));


	for(i=0;i<total_steps;i++) //Averaging all of the j-K-B values
	{
		k_av =0, b_av =0, j_av=0;

		for(j=0;j<NUM_SITES*2;j++) j_av += j_best[i*NUM_SITES*2+j], k_av += k_best[i*NUM_SITES*2+j];
		for(j=0;j<NUM_SITES;j++) b_av += b_best[i*NUM_SITES+j];

		k_avg[i] = k_av/(NUM_SITES*2);
		j_avg[i] = j_av/(NUM_SITES*2);
		b_avg[i] = b_av/(NUM_SITES);
	}

	file << "\n\n\nK averages\n[";
	for(i=0;i<total_steps;i++) file << k_avg[i] << ", ";

	file << "]\n\n\nJ averages\n[";
	for(i=0;i<total_steps;i++) file << j_avg[i] << ", ";

	file << "]\n\n\nB averages\n[";
	for(i=0;i<total_steps;i++) file << b_avg[i] << ", ";
	file << "]\n\n\n";


	export_evolve_calc(table,b,num_electrons,N,lattice, psi, k_best,j_best,b_best, bonds, total_steps, time_step, E_list, T_list, ham_target);

	file << "\n\n\ntimes:\n[";
	for(i=0;i<total_steps;i++) file << T_list[i] << ", ";

	file << "]\n\n\nExpectation-E_ground:\n[";
	for(i=0;i<total_steps;i++) file << (E_list[i]-ground_E) << ", ";
	file << "]";


	free(k_avg), free(b_avg), free(j_avg), free(T_list), free(E_list), free(bonds), free(psi);
	file.close();
}



void export_evolve_calc(int *table, unsigned long long int *b,int num_electrons,int N,  int lattice[][NX], double *psi, double *k_array, double *j_array, double *b_array, int *bonds, int total_steps, double time_step, double* E_list, double* T_list, double* ham_target)
{
	int i,j;
	double *ham_dev,*ham_t_i, *ham_real,*exp_matrix,*D, *Vdag;
	ham_dev = (double *) malloc (2*N*N*sizeof (double));
	exp_matrix = (double *) malloc (2*N*N*sizeof (double));
	ham_t_i = (double *) malloc (2*N*N*sizeof (double));
	ham_real = (double *) malloc(N*N*sizeof(double));
	Vdag = (double*) malloc(N*N*sizeof(double));
	D = (double*) malloc(N*sizeof(double));

	for (i=0; i<total_steps;i++)
	{
		construct_device_hamiltonian(table, b, num_electrons, N, ham_dev, lattice, k_array, j_array, b_array, bonds,i,DEVICE_DIMENSION);

		if(CHECK) check_norm(psi, N);

		if(DIAG)
		{
			for (j=0; j<N*N; j++) ham_real[j]=0.0,Vdag[j]=0.0;
			for (j=0; j<N; j++) D[j]=0.0;
			for (j=0; j<N*N; j++) ham_real[j] = ham_dev[2*j];//converting an all-real-valued complex matrix into just real matrix
			diag_hermitian_real_double(N, ham_real,Vdag, D);
			exp_diaganolized_mat(ham_dev, Vdag, D, N, time_step);//This function exponentiates D to e^(-i*time_step*D)

			if(CHECK) check_unitary(ham_dev, N);
			matrix_vector_mult(ham_dev,psi, N);
		}
		else
		{
			for (j=0; j<N*N*2; j++) exp_matrix[j] = 0.0, ham_t_i[j]=0.0;
			for (j=0; j<N*N; j++)
			{
				ham_t_i[2*j+1] = (ham_dev[2*j]*-time_step); //multiplying by -i*dt for the Pade approximation
				ham_t_i[2*j] = (ham_dev[2*j+1]*time_step);
			}
			exp_general_complex_double(N, ham_t_i, exp_matrix);
			if(CHECK) check_unitary(exp_matrix, N);
			matrix_vector_mult(exp_matrix, psi, N);
		}
		if(CHECK) check_norm(psi, N);
		E_list[i] = cost(psi, ham_target, N);
		T_list[i] = (i+1)*time_step;

	}
	free(ham_dev), free(ham_t_i), free(exp_matrix), free(D), free(Vdag),free(ham_real);
}




void print_vector(double* psi,int N)
{
	int i;
	printf("[");
	for(i=0;i<N;i++)
	{
		printf("%04.3f+%04.3fi; ", psi[2*i], psi[2*i+1]);
	}
	printf("]\n");
}




void print_best_arrays(double *k_array, double *j_array, double *b_array, int total_steps)
{
	int i,j;

	if (UNIFORM)
	{
		printf("\nK_array: %iX%i (stepsXsites)\n[", total_steps, NUM_SITES*2);
		for(i=0;i<total_steps;i++) printf("%5.2f ",k_array[i*NUM_SITES*2]);
		printf(";]\n");

		printf("J_array: %iX%i (stepsXsites)\n[", total_steps, NUM_SITES*2);
		for(i=0;i<total_steps;i++) printf("%5.2f ",j_array[i*NUM_SITES*2]);
		printf(";]\n");

		printf("B_array: %iX%i (stepsXsites)\n[", total_steps, NUM_SITES);
		for(i=0;i<total_steps;i++) printf("%5.2f ",b_array[i*NUM_SITES]);
		printf(";]\n");
	}

	else
	{
		printf("\nK_array: %iX%i (stepsXsites)\n[", total_steps, NUM_SITES*2);
		for(i=0;i<total_steps;i++)
		{
			for(j=0;j<2*NUM_SITES;j++)
			{
				printf("%5.2f ",k_array[i*NUM_SITES*2+j]);
				if(k_array[i*NUM_SITES+j] > 1 || -1 > k_array[i*NUM_SITES+j]) printf("\n\n\nERROR IN PRINT BEST\n\n\n");
			}
			if (i==total_steps-1) printf("]\n");
			else printf(";\n ");
		}

		printf("J_array: %iX%i (stepsXsites)\n[", total_steps, NUM_SITES*2);
		for(i=0;i<total_steps;i++)
		{
			for(j=0;j<2*NUM_SITES;j++)
			{
				printf("%5.2f ",j_array[i*NUM_SITES*2+j]);
				if(k_array[i*NUM_SITES+j] > 1 || -1 > k_array[i*NUM_SITES+j]) printf("\n\n\nERROR IN PRINT BEST\n\n\n");
			}
			if (i==total_steps-1) printf("]\n");
			else printf(";\n ");
		}

		printf("B_array: %iX%i (stepsXsites)\n[", total_steps, NUM_SITES);
		for(i=0;i<total_steps;i++)
		{
			for(j=0;j<NUM_SITES;j++)
			{
				printf("%5.2f ",b_array[i*NUM_SITES+j]);
				if(b_array[i*NUM_SITES+j] > 1 || -1 > b_array[i*NUM_SITES+j]) printf("\n\n\nERROR IN PRINT BEST\n\n\n");
			}
			if (i==total_steps-1) printf("]\n");
			else printf(";\n ");
		}
		printf("\n");
	}
}





void print_hamiltonianR(double* hamiltonian, int N)
/*prints a real-valued hamiltonian in matrix form*/
{
	int i,j;
	printf("\nPrinting the %ix%i Hamiltonian:\n[", N,N);
	for (i=0;i<N;i++)
	{
		for (j=0;j<N;j++) printf("%09.5f ",(hamiltonian[j*N+i])+0.0);
		if (i==N-1) printf("]");
		else printf("\n ");
	}
	printf("\n");
}




void print_hamiltonian(double* hamiltonian, int N)
/*prints a complex hamiltonian in matrix form, with the option to just print out the real values*/
{
	int i,j;
	printf("\nPrinting the %ix%i Hamiltonian:\n[", N,N);
	for (i=0;i<N;i++)
	{
		for (j=0;j<N;j++) printf("%09.5f+%09.5fi  ",(hamiltonian[2*(j*N+i)]+0.0), hamiltonian[2*(j*N+i)+1]);
		if (i==N-1) printf("]");
		else printf(";\n ");
	}
	printf("\n");
}




void print_E(double ground_E, double best_E)
{
	printf("#######################################################################\n");
	printf("TARGET ENERGY:  %9.6f\n",ground_E);
	printf("BEST ENERGY:    %9.6f\n",best_E);
	printf("DIFFERENCE:     %9.6f\n",best_E-ground_E);
	printf("#######################################################################\n");
	printf("#######################################################################\n");
}




void test_function()
{
	int i,j;
	int N = 3;
	double *D, *Vdag, *ham_target, min_E=1000;
	Vdag = (double*) malloc(N*N*sizeof(double));
	D = (double*) malloc(N*sizeof(double));
}


/*
 *
int main (int argc, char *argv[])
{


	int max_steps = ARRAY_SCALAR*total_steps_array[sizeof(total_steps_array)/sizeof(total_steps_array[0]) - 1]; //getting the last element


	if(PRINT) printf("\nSeed: %i\n", SEED);
	int *table,*bonds,lattice[NY][NX],num_electrons,N,i,j, z, t,x, tau, total_steps, y, ts, p;
	unsigned long long int *b;
	double *ham_target, *ham_start, *k_best, *j_best, *b_best, *k_ground, *j_ground, *b_ground, *psi_start, ground_E, initial_temp=1, best_E, b0,j0,k0, seed = SEED, time_step;

	int taus[] = {1,2,3,4,5,6,7,8,9,10};//{1,2,4};
	int total_steps_array[] = {1,2,4,8,16};//,16};

	int max_steps = ARRAY_SCALAR*total_steps_array[sizeof(total_steps_array)/sizeof(total_steps_array[0]) - 1]; //getting the last element


	gsl_rng_env_setup();
	const gsl_rng_type * TT = gsl_rng_default;
	gsl_rng * r  = gsl_rng_alloc (TT);
	gsl_rng_set(r, seed);

	construct_lattice(lattice);

	//change this later to cycle through all electrons
	for(i=2;i<3;i++)
	{
		num_electrons=i;
		N=choose(num_electrons);

		b = (unsigned long long int*) malloc(N*sizeof(double));
		table=(int*) malloc(num_electrons*N*sizeof(int));
		combinations (num_electrons,b,table, N);

		ham_target = (double *) malloc (2*N*N*sizeof (double));
		psi_start = (double *) malloc(2*N*sizeof(double));
		k_ground = (double *) malloc(NUM_SITES*4*sizeof(double));
		j_ground = (double *) malloc(NUM_SITES*4*sizeof(double));
		b_ground = (double *) malloc(NUM_SITES*2*sizeof(double));
		bonds = (int*) malloc(NUM_SITES*NUM_SITES*sizeof(int));
		ham_start = (double *) malloc(2*N*N*sizeof(double));

		for(j=0;j<NUM_SITES*NUM_SITES;j++) bonds[j]=1;

		//iterate_j_k_b()
		//change bounds later to cycle through multiple hams
		for(b0=0.9;b0<1.0;b0+=0.1)
		{
			for(k0=0.9;k0<1.0;k0+=0.1)
			{

				for(j0=0.9;j0<1.0;j0+=0.1)
				{
					//Creating the initial and target hamiltonian.
					k_ground[0] = K_START, j_ground[0] = J_START, k_ground[NUM_SITES*2] = k0, j_ground[NUM_SITES*2] = j0;
					for(z=0;z<NUM_SITES;z++) b_ground[z] = B_START, b_ground[NUM_SITES+z] = b0;
					construct_device_hamiltonian(table, b, num_electrons, N, ham_start,  lattice, k_ground, j_ground, b_ground, bonds, 0, DEVICE_DIMENSION);
					construct_device_hamiltonian(table, b, num_electrons, N, ham_target, lattice, k_ground, j_ground, b_ground, bonds, 1, DEVICE_DIMENSION);
					if(CHECK) check_commutator(N, ham_start, ham_target);


					//getting the ground state from the initial
					get_ground_state(N, ham_start,psi_start);
					ground_E = get_ground_E(N, ham_target);
					if(CHECK) check_norm(psi_start, N);



					k_best = (double *) malloc(2*NUM_SITES*max_steps*sizeof(double));
					j_best = (double *) malloc(2*NUM_SITES*max_steps*sizeof(double));
					b_best = (double *) malloc(NUM_SITES*max_steps*sizeof(double));
					for (x=0;x<2*NUM_SITES*max_steps;x++) k_best[x] =0, j_best[x] = 0;
					for (x=0;x<NUM_SITES*max_steps;x++) b_best[x] =0;



					for (t = 0; t<ceil(sizeof(taus)/sizeof(taus[0]));t++)
					{

						init_arrays(k_best, j_best, b_best, r, 1);
						tau =taus[t];
						for(ts=0;ts<sizeof(total_steps_array)/sizeof(total_steps_array[0]);ts++)
						{
							total_steps = total_steps_array[ts];
							time_step = tau/((double) total_steps);


							initial_temp = calc_initial_temp(table, b, num_electrons, N, lattice, ham_target,total_steps, time_step, psi_start, r);
							monte_carlo(table, b, num_electrons, N, lattice, ham_target, psi_start, initial_temp,tau, total_steps, time_step,r, k_best, j_best, b_best, ground_E);



							scale_arrays(total_steps, j_best, k_best, b_best);	//Scaling the arrays by two, using 1 array. pushing the data back
						}
					}
				}
			}
		}
		free(ham_start), free(bonds), free(k_ground), free(j_ground), free(b_ground), free(ham_target), free(b), free(table), free(k_best), free(j_best), free(b_best);
	}
	exit (0);
}
*/
