#include <iostream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <cstdlib>
#include <string>
#include "time.h"
#include <random>
#include <fstream>
using namespace std;

ofstream ofile;               //Global variable for writing results to file.


//Declaration of functions.
void initialize(int, int **, double&, double&, string);
void Monte_Carlo_Metropolis_time(int, int, int, int **, int, double&, double&, double*, double*, double*,
                                  double*, double*, double, double*, double&, mt19937_64, uniform_int_distribution<int>, uniform_real_distribution<double>);
void analytical_values_2x2Lattice(double*, double);
void Monte_Carlo_Metropolis_2x2(int, int, int **, int, double&, double&, double*, double*, double*, double, double**);



inline int periodic(int coordinate, int dimensions, int step) {
  return (coordinate+dimensions+step) % (dimensions);
}

//Main program
int main(int nargs, char* args[]){

  string outfilename, outfilename2, initializing;
  double boltzmann_distribution[17];
  int n, MC_samples, J, number_of_temperatures;
  int **spin_matrix;
  double E_initial, M_initial;                    //Stores initial energy and magnetization of system.
  int n_spins;                                    //Total number of spins.
  double beta;                                    //beta = 1/(k_b T)
  double variance;                                //Stores the variance



  //Read from command line
  number_of_temperatures = atoi(args[1]); //Specifies the number of temperatures the program runs
  outfilename = string(args[2]);          //Name of the file to write the results to
  n = atoi(args[3]);                      //Dimension of spin matrix
  MC_samples = atoi(args[4]);             //Number of Monte Carlo samples
  initializing = string(args[5]);         //String which determines the initial spin matrix

  random_device rd;
  mt19937_64 gen(rd());
  uniform_int_distribution<int> RandomIntegerGenerator(0,n-1);        //Sets up the uniform distribution for x in [0,n-1]
  uniform_real_distribution<double> RandomNumberGenerator(0,1);       //Sets up the uniform distribution for x in [0,1]



  //initialize matrix
  spin_matrix = new int*[n];
  for (int i = 0; i < n; i++){
    spin_matrix[i] = new int[n];
  }

  int N = 1e6;                                                         //Burn in period
  E_initial = 0;
  M_initial = 0;
  n_spins = n*n;                                                       //Total number of spins
  J = 1;                                                               //Coupling constant
  initialize(n, spin_matrix, E_initial, M_initial, initializing);     //initialize spin matrix


  if (number_of_temperatures == 1 && n > 2){
    double *energy, *magnetization, *time, *acceptance, *energies;
    double T = atof(args[6]);                                       //Temperature
    outfilename2 = string(args[7]);
    int n_times = (MC_samples-N)/n_spins;                           //Number of lattice sweeps


    //Initialize pointers
    energy = new double[n_times];
    magnetization = new double[n_times];
    time = new double[n_times];
    acceptance = new double[n_times];
    energies = new double[MC_samples - N+1];
    variance = 0;                                                 //Hardcoding the variance to zero

    //Compute Boltzmann factors.
    beta = 1/(T);                //k_B = 1
    for (int i = -8; i < 9; i += 4){
      boltzmann_distribution[i + 8] = exp(-beta*i);
    }

    //Running the Monte Carlo Markov Chain with Metropolis algorithm
    Monte_Carlo_Metropolis_time(MC_samples, n, N, spin_matrix, J, E_initial, M_initial, boltzmann_distribution,
                                energy, magnetization, time, acceptance, beta, energies, variance, gen, RandomIntegerGenerator, RandomNumberGenerator);

    //Writing the produced data to files
    ofile.open(outfilename);
    for (int i = 0; i < n_times; i++){
      ofile << time[i] << " " << setprecision(8) << energy[i] << " " << setprecision(8) << " " << magnetization[i] << " " << acceptance[i] << endl;
    }
    ofile.close();

    ofile.open(outfilename2);
    for (int k = 0; k < MC_samples - N; k++){
      ofile << energies[k] << endl;
    }
    ofile.close();


    cout << "Standard deviation for T = " << T << ": " << sqrt(variance) <<endl;


  }

  if (n == 2){
    double *time, *acceptance;
    double T = atof(args[6]);                                       //Temperature
    double** expectation_values;
    double* analytical_values;
    double** relative_error;
    int n_times;

    outfilename2 = string(args[7]);


    n_times = MC_samples/n_spins;                                   //Number of lattice sweeps
    analytical_values = new double[8];
    expectation_values = new double*[8];
    relative_error = new double*[7];

    time = new double[n_times];
    acceptance = new double[n_times];

    //Initializing pointers
    for (int i = 0; i < 8; i++){
      expectation_values[i] = new double[n_times];
    }

    for (int i=0; i < 7; i++){
      relative_error[i] = new double[n_times];
    }


    //Computing the boltzmann distribution for 5 values of dE
    double beta = 1/(T);                //k_B = 1
    for (int i = -8; i < 9; i+=4){
      boltzmann_distribution[i + 8] = exp(-beta*i);
    }

    //Running the Monte Carlo Markov Chain with Metropolis algorithm
    Monte_Carlo_Metropolis_2x2(MC_samples, n, spin_matrix, J,  E_initial, M_initial, boltzmann_distribution, time, acceptance, beta, expectation_values);

    //Computing the analytical values for a 2 by 2 lattice
    analytical_values_2x2Lattice(analytical_values, T);


    //Computing relative error
    for (int i = 0; i < n_times; i++){

      relative_error[0][i] = abs((analytical_values[0]-expectation_values[0][i])/analytical_values[0]);   //Stores relative error in E
      relative_error[1][i] = abs((analytical_values[1]-expectation_values[1][i])/analytical_values[1]);   //Stores relative error in E_squared
      relative_error[2][i] = abs((analytical_values[2]-expectation_values[2][i])/analytical_values[2]);   //Stores relative error in Mabs
      relative_error[3][i] = abs((analytical_values[3]-expectation_values[3][i])/analytical_values[3]);   //Stores relative error in Mabs_squared
      relative_error[4][i] = abs((analytical_values[5]-expectation_values[5][i])/analytical_values[5]);   //Stores relative error in M_squared
      relative_error[5][i] = abs((analytical_values[6]-expectation_values[6][i])/analytical_values[6]);   //Stores relative error in Heat Capacity
      relative_error[6][i] = abs((analytical_values[7]-expectation_values[7][i])/analytical_values[7]);   //Stores relative error in Magnetic susceptibility
    }

    //Writing the produced data to file
    ofile.open(outfilename);
    for (int i = 0; i < n_times; i++){
      ofile << setprecision(9) << time[i] << " " << setprecision(9) << expectation_values[0][i] << " " << setprecision(9) << expectation_values[1][i] << " " << setprecision(9) << expectation_values[2][i]
      << " " << setprecision(9) << expectation_values[3][i]<< " " << setprecision(9) << expectation_values[4][i] << " " << setprecision(9) << expectation_values[5][i] << " " << setprecision(9) << expectation_values[6][i]
      << " " << setprecision(9) << expectation_values[7][i] << endl ;
    }
    ofile.close();


    ofile.open(outfilename2);
    for (int i = 0; i < n_times; i++){
      ofile << setprecision(9) << time[i] << " " << setprecision(9) << relative_error[0][i] << " " << setprecision(9) << relative_error[1][i] << " " << setprecision(9) << relative_error[2][i]
      << " " << setprecision(9) << relative_error[3][i]<< " " << setprecision(9) << relative_error[4][i] << " " << setprecision(9) << relative_error[5][i] << " " << setprecision(9) << relative_error[6][i] << endl;
    }

    ofile.close();

    }

  return 0;
}

void initialize(int dimensions, int **spin_matrix, double& E, double& M, string initialize){
  if (initialize == "ordered"){
    // setup spin matrix and intial magnetization
    for(int i =0; i < dimensions; i++) {
      for (int j= 0; j < dimensions; j++){
        spin_matrix[i][j] = 1; // spin orientation for the ground state
        M +=  (double) spin_matrix[i][j];
      }
    }
  }

  if (initialize == "random"){
    random_device rd;
    mt19937_64 gen(rd());
    uniform_real_distribution<double> RandomNumberGenerator(0,1);
    double x;

    // setup spin matrix and intial magnetization
    for(int i = 0; i < dimensions; i++) {
      for (int j= 0; j < dimensions; j++){
        x = RandomNumberGenerator(gen);
        if (x <= 0.5){
          spin_matrix[i][j] = 1;        //spin orientation for the ground state
        }
        else{
          spin_matrix[i][j] = -1;      //spin orientation for the ground state
        }
        M +=  (double) spin_matrix[i][j];
      }
    }

  }
  // setup initial energy
  for(int i = 0; i < dimensions; i++) {
    for (int j= 0; j < dimensions; j++){
      E -=  (double) spin_matrix[i][j] * (spin_matrix[periodic(i,dimensions,-1)][j] + spin_matrix[i][periodic(j,dimensions,-1)]);
      }
    }
  }

void analytical_values_2x2Lattice(double* analytical_values, double T){

      double Z_a,E_a,M_a,E_squared_a,M_squared_a, Mabs_a, Mabs_squared_a,beta;

      int n_spins = 4;

      beta = 1/T;

      //Calculates the analytical expectation values
      Z_a =  4*(3 + cosh(8*beta));
      E_a = -32*sinh(8*beta)/Z_a;
      E_squared_a = 256*cosh(8*beta)/Z_a;
      M_a = 0;
      M_squared_a = 32*(exp(8*beta) + 1)*beta/Z_a ;
      Mabs_a =  8*(exp(8*beta) + 2)/Z_a;
      Mabs_squared_a = (32*exp(8*beta) + 4)/Z_a;


      //Stores the analytical expectation values
      analytical_values[0] = E_a/n_spins;
      analytical_values[1] = E_squared_a/n_spins;
      analytical_values[2] = Mabs_a/n_spins;
      analytical_values[3] = Mabs_squared_a/n_spins;
      analytical_values[4] = M_a/n_spins;
      analytical_values[5] = M_squared_a/n_spins;
      analytical_values[6] = (analytical_values[1]-analytical_values[0]*analytical_values[0])*beta*beta;    //Heat Capacity
      analytical_values[7] = (analytical_values[5])*beta;                                                   //Magnetic susceptibility


    }


void Monte_Carlo_Metropolis_time(int MC, int n, int N, int **spin_matrix, int J, double& E, double& M, double* boltzmann_distribution,
                                  double* energy, double* magnetization, double* time, double* acceptance, double beta,
                                  double* energies, double& variance, mt19937_64 gen, uniform_int_distribution<int> RandomIntegerGenerator, uniform_real_distribution<double> RandomNumberGenerator){


  int x_flip, y_flip, dE, dM, n_spins, i, accept, l;
  double E_sum, M_sum, Mabs_sum, Mabs_sum_squared, E_squared, M_squared;

  l = 0;
  n_spins = n*n;
  E_sum = 0.0;
  M_sum = 0.0;
  Mabs_sum = 0.;
  Mabs_sum_squared = 0.;
  E_squared = 0.;
  M_squared = 0.;
  accept = 0;


  //Running over Monte Carlo samples
  for (int k = 1; k <= MC; k++){


    x_flip = RandomIntegerGenerator(gen);
    y_flip = RandomIntegerGenerator(gen);      //Randomized indices to matrix element that will be flipped

    //Computing the energy difference
    dE = (int) 2*J*spin_matrix[x_flip][y_flip] * (spin_matrix[periodic(x_flip,n,1)][y_flip] + spin_matrix[periodic(x_flip, n,-1)][y_flip]
                                                  + spin_matrix[x_flip][periodic(y_flip, n,1)] + spin_matrix[x_flip][periodic(y_flip, n,-1)]);

    //Metropolis algorithm
    if(dE < 0){
      spin_matrix[x_flip][y_flip] *= (-1);   //Accepting the flip
      dM = 2*spin_matrix[x_flip][y_flip];
      accept += 1;
    }
    else if(RandomNumberGenerator(gen) < boltzmann_distribution[dE + 8]){
      spin_matrix[x_flip][y_flip] *= (-1);     //Accepting flip
      dM = 2*spin_matrix[x_flip][y_flip];
      accept += 1;
    }
    else{
      dE = 0;                             //Rejecting the flip
      dM = 0;
    }

    E += dE;
    M += dM;


    //Summing up the quantities after the steady state is reached
    if (k > N-1){
      energies[l] = E;
      l += 1;

      E_sum += (double) E;
      M_sum += (double) M;
      Mabs_sum += (double) abs(M);

      E_squared += (double) E*E;
      M_squared += (double) M*M;
      Mabs_sum_squared += (double) abs(M)*abs(M);
    }

    //Calculating the energy, magnetization and accepted spin-flips per lattice sweep
    if (k > N-1 && k % n_spins == 0){
      i = (k - N)/ n_spins;
      energy[i] = E_sum/ ((double) (k-N) * n_spins);
      //energy[i] = E/n_spins;
      magnetization[i] = Mabs_sum / ((double) (k-N) * n_spins);
      //magnetization[i] = M/n_spins;
      time[i] = i+1;
      acceptance[i] = accept;
    }


  }
  E_sum /= (MC-N);
  E_squared /= (double) (MC-N);
  M_squared /= (double) (MC*n_spins);
  Mabs_sum /= (double) (MC*n_spins);
  Mabs_sum_squared /= (double) (MC*n_spins);
  variance = E_squared - E_sum*E_sum;

}

void Monte_Carlo_Metropolis_2x2(int MC, int n, int **spin_matrix, int J, double& E, double& M, double* boltzmann_distribution, double* time, double* acceptance, double beta,double** expectation_values){


  random_device rd;
  mt19937_64 gen(rd());
  uniform_int_distribution<int> RandomIntegerGenerator(0,n-1);        //Sets up the uniform distribution for x in [0,n-1]
  uniform_real_distribution<double> RandomNumberGenerator(0,1);       //Sets up the uniform distribution for x in [0,1]

  int x_flip, y_flip, dE, dM, n_spins, i, accept, n_times;
  double E_sum, M_sum, Mabs_sum, Mabs_sum_squared, E_sum_squared, M_sum_squared;
  double* Mabs, *Mabs_squared, *E_squared, *M_squared, *energy, *magnetization;

  n_spins = n*n;                                    //Total number of spins
  n_times = MC/n_spins;                             //Number of lattice sweeps

  E_sum = 0.0;
  M_sum = 0.0;
  Mabs_sum = 0.;
  Mabs_sum_squared = 0.;
  E_sum_squared = E*E;
  M_sum_squared = M*M;
  accept = 0;

  energy = new double[n_times];
  magnetization = new double[n_times];
  Mabs = new double[n_times];
  Mabs_squared = new double[n_times];
  E_squared = new double[n_times];
  M_squared = new double[n_times];


  //Running over Monte Carlo samples
  for (int k = 1; k <= MC; k++){


    //for (int j = 0; j < n_spins; j++){
      x_flip = RandomIntegerGenerator(gen);
      y_flip = RandomIntegerGenerator(gen);      //Randomized indices to matrix element that will be flipped

      //Calculating the energy difference
      dE =  2*J*spin_matrix[x_flip][y_flip] * (spin_matrix[periodic(x_flip,n,1)][y_flip]
            + spin_matrix[periodic(x_flip, n,-1)][y_flip]
            + spin_matrix[x_flip][periodic(y_flip, n,1)]
            + spin_matrix[x_flip][periodic(y_flip, n,-1)]);

      //Metropolis algorithm
      if(dE < 0){
        spin_matrix[x_flip][y_flip] *= (-1);   //Accepting the flip
        dM = 2*spin_matrix[x_flip][y_flip];
        accept += 1;
      }
      else if(RandomNumberGenerator(gen) < boltzmann_distribution[dE + 8]){
        spin_matrix[x_flip][y_flip] *= (-1);     //Accepting flip
        dM = 2*spin_matrix[x_flip][y_flip];
        accept += 1;
      }
      else{
        dE = 0;                             //Rejecting the flip
        dM = 0;
      }

      E += dE;
      M += dM;

    //}

    E_sum += (double) E;
    M_sum += (double) M;
    Mabs_sum += (double) abs(M);

    E_sum_squared += (double) E*E;
    M_sum_squared += (double) M*M;
    Mabs_sum_squared += (double) abs(M)*abs(M);

    //Summing the quantities after a lattice sweep
    if (k % n_spins == 0){
      i = k / n_spins - 1;
      energy[i] = E_sum/ (k);
      magnetization[i] = M_sum / (k);
      Mabs[i] = Mabs_sum/ ( k);
      Mabs_squared[i] = Mabs_sum_squared/ (k);
      E_squared[i] = E_sum_squared/ (k);
      M_squared[i] = M_sum_squared/ (k);
      time[i] = i + 1;
      acceptance[i] = accept;
    }



  }
  //Filling the pointer with the expectation values
  expectation_values[0] = energy;
  expectation_values[1] = E_squared;
  expectation_values[2] = Mabs;
  expectation_values[3] = Mabs_squared;
  expectation_values[4] = magnetization;
  expectation_values[5] = M_squared;
  for (int i = 0; i < n_times; i++){
    expectation_values[6][i] = (expectation_values[1][i]-expectation_values[0][i]*expectation_values[0][i])*beta*beta;             //Stores the computed expectation value for heat capacity
    expectation_values[7][i] = (expectation_values[3][i]-(expectation_values[2][i]*expectation_values[2][i]))*beta;
  }     //Stores the computed expectation value for susceptibility
}
