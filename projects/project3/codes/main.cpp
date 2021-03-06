#include <iostream>
#include "lib.h"
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <cstdlib>
#include <armadillo>
#include <string>
#include "time.h"
#include <random>
#include <fstream>
#include <omp.h>
#define EPS 3.0e-14
#define MAXIT 10


using namespace std;

ofstream ofile;               //Global variable for writing results to file.

//Declaration of functions
double CoulombRepulsion(double, double, double, double, double, double);
double CoulombRepulsion_spherical(double, double, double);
double gammln(double);
void gauss_laguerre(double*, double*, int, double);
double laguerre_integrate_func_3(double, double, double);
double laguerre_integrate_func_6(double, double, double, double, double, double);
double CoulombRepulsionMC(double*);
double CoulombRepulsionMC_ImportanceSampling(double*);

int main(int nargs, char* args[]){
  string integration_method;              //Specifies the integration method to be applied
  string outfilename;                     //Name of the file to write the results to
  clock_t start, finish;                  //Declaration of variables to compute the time used.

  if (nargs == 1){

    cout << "Specify integration method, choose from: " << endl;
    cout << "------------------------------------------------------" << endl;
    cout << "Gauss Legendre method                  --> type 1 " << endl;
    cout << "Gauss Laguerre method                  --> type 2 " << endl;
    cout << "Brute force Monte Carlo                --> type 3 " << endl;
    cout << "Monte Carlo with importance sampling   --> type 4 " << endl;
    cout << "------------------------------------------------------" << endl;
    cin >> integration_method;
  }
  else{
    outfilename = string(args[1]);
    integration_method = string(args[2]);
  }

  //Integration method "1" implements the Gaussian-Legendre quadrature.
  if (integration_method == "1"){
    int n;                                                                  //Number of integration points
    double a,b;                                                             //Integration intervals [a,b] x ... x??[a,b]
    clock_t start, finish;                                                  //Variables used to compute the time used.

    //If no arguments is made at the command line, this prompts you to necessary parameters to run the code.
    if (nargs == 1){
      cout << "Specify number of integration points: " << endl;
      cin >> n;
      cout << "Specify integration limits [a,b] : " << endl;
      cin >> a >> b;
    }
    else{
      n = atoi(args[3]);
      a = atof(args[4]);
      b = atof(args[5]);

    }

    double *x1, *y1, *z1, *x2, *y2, *z2;                                //Pointers that store meshpoints (zeros of N-th legendre polynomial)
    double *w1, *w2, *w3, *w4, *w5, *w6;                                //Pointers that store weights.
    x1 = new double[n];
    y1 = new double[n];
    z1 = new double[n];
    x2 = new double[n];
    y2 = new double[n];
    z2 = new double[n];

    w1 = new double[n];
    w2 = new double[n];
    w3 = new double[n];
    w4 = new double[n];
    w5 = new double[n];
    w6 = new double[n];

    //Generate mesh points and weights using the Gauss-Legendre method found in Numerical Recipies.
    gauleg(a,b,x1,w1,n);
    gauleg(a,b,y1,w2,n);
    gauleg(a,b,z1,w3,n);
    gauleg(a,b,x2,w4,n);
    gauleg(a,b,y2,w5,n);
    gauleg(a,b,z2,w6,n);


    double integral_gauss_legendre = 0;                                     //Hardcode the integral to be zero initially.
    start = clock();                                                        //Starts the clock.

    //Solves the integral using the weights and mesh points obtained with the library function gauleg. This is the main algorithm.
    for (int i = 0; i < n; i++){
      for (int j = 0; j < n; j++){
        for (int k = 0; k < n; k++){
          for (int l = 0; l < n; l++){
            for (int p = 0; p < n; p++){
              for (int r = 0; r < n; r++){
                integral_gauss_legendre += w1[i]*w2[j]*w3[k]*w4[l]*w5[p]*w6[r]*CoulombRepulsion(x1[i], y1[j], z1[k], x2[l], y2[p], z2[r]);
              }
            }
          }
        }
      }
    }

    finish = clock();                                                     //Stops the clock.
    double exact = 5*pow(M_PI, 2)/(16*16);                                //Analytical value of the integral
    double timeused = (double) (finish - start)/(CLOCKS_PER_SEC);         //Computes the total time used by the main algorithm
    double relative_error = abs((integral_gauss_legendre-exact)/exact);   //Computes the relative error.

    //If no command line arguments are made, it prints the computed results to screen.
    if (nargs == 1){
      cout << "----------------------------------------------------------------" << endl;
      cout << "computed integral = " << integral_gauss_legendre << endl;
      cout << "exact value = " << exact << endl;
      cout << "relative error = " << relative_error << endl;
      cout << "time used = " << " " << timeused << " " << "seconds." << endl;
      cout << "----------------------------------------------------------------" << endl;
    }

    if (nargs != 1){
      ofile.open(outfilename);
      ofile << integral_gauss_legendre << " " << n << " " << relative_error << " " << timeused << endl;
    }
  }

  //Integration method "2" implements the Gaussian-Laguerre quadrature.
  if (integration_method == "2"){
    int n, dimensions;              //n is the number of integration points, dimensions denote what type of integral to do (3D vs 6D)
    double a,b, alpha;

    //Integration limits
    a = 0;
    b = M_PI;
    double integral_gauss_laguerre = 0;             //Hardcodes the integral to zero initially.

    //If no command line arguments are made, you'll be prompted to specify the necessary parameters for the code to run.
    if (nargs == 1){
      cout << "Specify number of integration points: " << endl;
      cin >> n;

      cout << "Choose dimension for integral:" << endl;
      cout << "------------------------------------------------------" << endl;
      cout << "For 3 dimensions                           --> type 3 " << endl;
      cout << "For 6 dimensions                           --> type 6 " << endl;
      cout << "------------------------------------------------------" << endl;
      cin >> dimensions;
      cout << "------------------------------------------------------" << endl;

    }
    else{
      dimensions = atoi(args[3]);
      n = atoi(args[4]);
    }


    //This part implements a 3-dimensional integral (the other 3-dimensions are solved analytically - because they're trivial).
    if(dimensions == 3){
      double *w1, *w2, *w3, *r1, *r2, *theta2;

      w1 = new double[n];
      w2 = new double[n];
      w3 = new double[n];
      r1 = new double[n+1];
      r2 = new double[n+1];
      theta2 = new double[n];
      alpha = 2;                      //the exponent in the weight function W(x) = x^(alpha)e^(-x).


      //Computes the weights with the Gaussian-Legendre method for the radial parts of the integral
      gauss_laguerre(r1, w1, n, alpha);
      gauss_laguerre(r2, w2, n, alpha);

      //Computes the weights for the angle theta2.
      gauleg(a, b, theta2, w3, n);

      start = clock();                        //Starts the clock.

      //Main algorithm
      for(int i = 1; i<(n+1); i++){
        for(int j = 1; j<(n+1); j++){
          for(int k = 0; k<n; k++){
            integral_gauss_laguerre += w1[i]*w2[j]*w3[k]*laguerre_integrate_func_3(r1[i], r2[j], theta2[k]);
          }
        }
      }


      integral_gauss_laguerre *= (M_PI*M_PI)/128;             //Multiplies by the factor found analytically.

      finish = clock();                                       //Stops the clock.
    }

    if(dimensions == 6){
      double *w1, *w2, *w3, *w4, *w5, *w6, *r1, *r2, *theta1, *theta2, *phi1, *phi2;

      w1 = new double[n+1];
      w2 = new double[n+1];
      w3 = new double[n];
      w4 = new double[n];
      w5 = new double[n];
      w6 = new double[n];

      r1 = new double[n+1];
      r2 = new double[n+1];

      theta1 = new double[n];
      theta2 = new double[n];

      phi1 = new double[n];
      phi2 = new double[n];

      alpha = 0;                              //The exponent in the weight function W(x) = x^(alpha)e^(-x) for Laguerre polynomials.

      //Computes the weights and meshpoints for the radial parts of the integral using the Gauss-Laguerre method.
      gauss_laguerre(r1, w1, n, alpha);
      gauss_laguerre(r2, w2, n, alpha);

      //Computes the weights and meshpoints for the angles using the Gauss-Legendre method.
      gauleg(a, b, theta1, w3, n);
      gauleg(a, b, theta2,w4, n);
      gauleg(a, 2*b, phi1, w5, n);
      gauleg(a, 2*b, phi2, w6, n);

      start = clock();              //Starts the clock

      //Main algorithm
      for (int i = 1; i <= n; i++){
        for (int j = 1; j <= n; j++){
          for (int k = 0; k < n; k++){
            for (int l = 0; l < n; l++){
              for (int p = 0; p < n; p++){
                for (int r = 0; r < n; r++){
                  integral_gauss_laguerre += w1[i]*w2[j]*w3[k]*w4[l]*w5[p]*w6[r]*laguerre_integrate_func_6(r1[i], r2[j], theta1[k], theta2[l], phi1[p], phi2[r]);
                }
              }
            }
          }
        }
      }
      finish = clock();       //Stops the clock
    }

      double timeused = (double) (finish - start)/CLOCKS_PER_SEC;               //Computes the total time used by the main algorithm
      double exact = 5*pow(M_PI, 2)/(16*16);                                    //The analytical value of the integral
      double rel_error = abs(integral_gauss_laguerre - exact)/exact;            //Computes the relative error.

      //If no command line arguments are provided, the computed results are printed to screen.
      if (nargs == 1){
        cout << "-------------------------------------------------------------------------" << endl;
        cout << "computed integral = " << integral_gauss_laguerre << endl;
        cout << "exact value = " << exact << endl;
        cout << "relative error = " << rel_error << endl;
        cout << "time used = " << " " << timeused << " " << "seconds." << endl;
        cout << "-------------------------------------------------------------------------" << endl;
    }
      //If the code is run with main.py, the necessary command line arguments are provided and the results are written to file.
      if (nargs != 1){
      ofile.open(outfilename);
      ofile << n << " " << setprecision(9) << integral_gauss_laguerre << " "<< setprecision(9) << rel_error << " " << setprecision(9) <<timeused <<endl;
      ofile.close();
    }

  }

  //Integration method "3" implements Monte Carlo integration with brute force (uniform distributions)
  if (integration_method == "3"){
    //Declaration of variables
    int n, d;
    double a, b, integral, sigma, variance, std_mean, jacobidet, func_value, exact, relative_error, timeused;
    double *x;
    clock_t start, finish;

    //Sets up the uniform distribution for x in [0,1]
    random_device rd;
    mt19937_64 gen(rd());
    uniform_real_distribution<double> RandomNumberGenerator(0,1);

    //If no command line arguments are made, you're prompted to provide the necessary parameters to run the code.
    if (nargs == 1){
      cout << "Specify number of monte carlo samples:" << endl;
      cin >> n;
      cout << "Specify integration limits [a,b]: " << endl;
      cin >> a >> b;
    }
    else{
      n = atoi(args[3]);                    //Number of monte carlo samples
      a = atof(args[4]);                    //Start point of integration
      b = atof(args[5]);                    //End point of integration
    }


    //Hardcode specific parameters
    d = 6;                                                //dimensionality of the integral
    jacobidet = pow((b-a), d);                            //Jacobideterminant
    x = new double[d];                                    //Integration coordinates x1,...,xd.
    integral = 0.;                                        //Will store the computed integral
    sigma = 0.;                                           //Will store the "standard deviation" during the computations.
    variance = 0.;                                        //The statistical variance
    std_mean = 0.;                                        //The standard deviation of the mean, this is the interesting statistical quantity.
    exact = 5*M_PI*M_PI/(16*16);                          //Analytical value of the integral.

    start = clock();                                      //Starts the clock.

    //Main algorithm: Here the integration begins, looping over all n monte carlo samples
    for (int i = 0; i < n; i++){
      //Collect a sample of the stochastic varable X = (X1,...,Xd)
      for (int j = 0; j < d; j++){
        //Use the mapping mu(x[j]) = a + (b-a)*x[j]
        x[j] = RandomNumberGenerator(gen);                              //Random number from uniform distribution on [0,1]
        x[j] = a + (b-a)*x[j];                                          //Change of coordinates.
      }
      func_value = CoulombRepulsionMC(x);                               //Computes the function value for the sample of X
      integral += func_value;                                           //computes the contribution to the integral
      sigma += func_value*func_value;                                   //computes the contribution to the variance
    }
    integral /= ((double) n);                                           //Arithmetic mean to obtain the integral
    sigma /= ((double) n);                                              //Arithmetic mean to obtain the variance
    variance = sigma - integral*integral;                               //Computes the variance
    integral *= jacobidet;                                              //Final contribution to the integral.
    std_mean = jacobidet*sqrt(variance/( (double) n));                  //Computes the standard deviation of the mean.
    relative_error = abs((integral - exact)/exact);                     //Computes the relative error
    finish = clock();                                                   //Stops the clock.
    timeused = (double) (finish - start)/CLOCKS_PER_SEC;                //Computes the total time used by the main algorithm.


    //If no command line arguments are provided, computed results are printed to screen
    if (nargs == 1){
      cout << "---------------------------------------------------------------------------" << endl;
      cout << "Computed value = " << integral << endl;
      cout << "standard deviation = " << std_mean << endl;
      cout << "exact value = " << exact << endl;
      cout << "relative error = " << relative_error << endl;
      cout << "time used = " << timeused << " " << "seconds." << endl;
      cout << "---------------------------------------------------------------------------" << endl;
    }

    //If run with main.py, the necessary command line arguments are provided and the results are written to file.
    if(nargs != 1){
    //Write the results to file
    ofile.open(outfilename);
    ofile << integral << " " << std_mean << " " << relative_error << " " << timeused << endl;
    ofile.close();
}
  }

  //Integration method "4" implements Monte Carlo integration with importance sampling.
  if (integration_method == "4"){
    //Declaration of variables
    int n, d;
    double max_radial_distance, integral, sigma, variance, std_mean, jacobidet, func_value, exact, alpha, relative_error, timeused;
    double *x;
    clock_t start, finish;

    //Sets up the uniform distribution for x in [0,1]
    random_device rd;
    mt19937_64 gen(rd());
    uniform_real_distribution<double> RandomNumberGenerator(0,1);

    //If no command line arguments are made, you're prompted to provide the necessary parameters to run the code.
    if (nargs == 1){
      cout << "Specify number of monte carlo samples:" << endl;
      cin >> n;
      cout << "Specify maximum radial distance: " << endl;
      cin >> max_radial_distance;
    }
    else{
      n = atoi(args[3]);                                            //Number of monte carlo samples.
      //max_radial_distance = atof(args[4]);                        //Useless parameter that has no purpose, but removing it requires restructuring the automated codes.
    }

    //Hardcode specific parameters
    d = 3;                                                //Dimensionality of the integral, 3 of the dimensions are solved analytically (because they're trivial).
    jacobidet = 8*pow(M_PI, 3)/16;                        //"Jacobideterminant", contains the analytical part as well as the Jacobideterminant from the change of coordinates.
    x = new double[d];                                    //Integration coordinates x1,...,xd.
    integral = 0.;                                        //Stores the computed the intergral
    sigma = 0.;                                           //Stores the "standard deviation"
    variance = 0.;                                        //Stores the statistical variance
    std_mean =0.;                                         //Stores the standard deviation of the mean (it's the statistical quantity we want)
    exact = 5*M_PI*M_PI/(16*16);                          //Exact value of the integral.
    alpha = 4;                                            //Exponent in the exponential function.

    start = clock();                                      //Starts the clock

    //And so it begins... summing up the function values for n monte carlo samples (Main algorithm)
    for (int i = 0; i < n; i++){
      //Sample from the respective probability distributions of X = (r1,r2,theta2)
      x[0] = RandomNumberGenerator(gen);
      x[0] = -log(1-x[0])/alpha;                                                //r1-coordinate
      x[1] = RandomNumberGenerator(gen);
      x[1] = -log(1-x[1])/alpha;                                                //r2-coordinate
      x[2] = RandomNumberGenerator(gen);
      x[2] = M_PI*x[2];                                                         //theta2-coordinate
      func_value = CoulombRepulsionMC_ImportanceSampling(x);                    //Computes the functino value of the vector x = (r1(x1), r2(x2), theta2(x3)).
      integral += func_value;                                                   //Adds the contribution to the integral
      sigma += func_value*func_value;                                           //Adds the contribution to the "standard deviation"
    }
    integral /= ((double) n);                                                   //Arithmetic mean to obtain the integral
    sigma /= ((double) n);                                                      //Arithmetic mean to obtain the variance
    variance = sigma - integral*integral;                                       //Computes the variance
    integral *= jacobidet;                                                      //Final contribution to the integral.
    std_mean = jacobidet*sqrt(variance/( (double) n));                          //The computed standard deviation of the mean
    finish = clock();                                                           //Stops the clock
    timeused = (double) (finish - start)/CLOCKS_PER_SEC;                        //computes the total time used by the main algorithm.
    relative_error = abs((integral - exact)/exact);                             //Computes the relative error


    //Approximates the ground state energy of the Helium atom.
    double bohr_radius, k, A, E_gs;
    k = 1.44;                                                                                 //eV nm
    bohr_radius = 5.29177e-11 * 1e9;                                                          //Bohr radius in nm.
    A = 8/(M_PI*pow(bohr_radius,3));                                                          //The appropriate normalization constant.
    E_gs = k*A*A*pow(bohr_radius,5) * integral + (-8*13.6057);                                //The expectation value of the Hamiltonian, approximates the ground state
    double exact_groundstate = -79.005151042;                                                 //The experimental value of the ground state energy.
    double ground_state_relative_error = abs((E_gs - exact_groundstate)/exact_groundstate);   //Relative error of the ground state energy.

    //If no command line arguments are provided, the results are printed to screen.
    if (nargs == 1){
      cout << "Computed value = " << integral << endl;
      cout << "standard deviation = " << std_mean << endl;
      cout << "exact value = " << exact << endl;
      cout << "relative error = " << relative_error << endl;
      cout << "time used = " << timeused << " " << "seconds." << endl;
      cout << "Computed ground state energy = " << E_gs << " " << "eV" << endl;
      cout << "relative error (ground state energy) = " << ground_state_relative_error << endl;
    }

    //If the code main.py is run, all the necessary parameters are provided automatically and the results are written to file.
    if(nargs != 1){
    //Write results to file.
    ofile.open(outfilename);
    ofile << integral << " " << std_mean << " " << relative_error << " " << timeused << " " << E_gs << " " << ground_state_relative_error << endl;
    ofile.close();
    }
  }

  return 0;
}


double CoulombRepulsionMC(double *x){
  /*
  Electron-electron repulsion function used by the brute force Monte Carlo method
  */
  double func_value = 0;
  double norm = sqrt((x[0]-x[3])*(x[0]-x[3]) + (x[1]-x[4])*((x[1]-x[4])) + (x[2]-x[5])*(x[2]-x[5]));
  if (norm == 0){
    func_value = 0;
  }
  else{
    func_value = exp(-4*( sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]) + sqrt(x[3]*x[3] + x[4]*x[4] + x[5]*x[5]) ))/norm;
  }
  return func_value;
}

double CoulombRepulsionMC_ImportanceSampling(double *x){
  /*
  Electron-electron repulsion function used by the Monte carlo method with importance sampling.
  */
  double func_value;
  double norm = sqrt( x[0]*x[0] + x[1]*x[1] - 2*x[0]*x[1]*cos(x[2]) );
  if (norm == 1){
    func_value = 0;
  }
  else{
    func_value = x[0]*x[0]*x[1]*x[1]*sin(x[2])/norm;
  }
  return func_value;
}

double CoulombRepulsion(double x1, double y1, double z1, double x2, double y2, double z2){
  /*
  The function which we integrate using the Gauss-Legendre method.
  */
  if (sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2)) == 0){
    return 0;
  }
  return exp( -4*( sqrt(x1*x1 + y1*y1 + z1*z1) + sqrt(x2*x2 + y2*y2 + z2*z2) ))/sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2));
}

double CoulombRepulsion_spherical(double r1, double r2, double theta2){
  /*
  CoulombRepulsion in spherical coordinates. Includes the Jacobi determinant r1^2r2^2 sin(theta2).
  The other coordinates are just multiplied as constants since the integral is not explicitly dependent upon phi1, phi2 or theta1.
  Is used in the part that implements the Gauss-Laguerre method.
  */
  if (sqrt(r1*r1 + r2*r2 - r1*r2*cos(theta2)) == 0){
    return 0;
  }
  return r1*r1*r2*r2*sin(theta2)*exp(-4*(r1+r2)) / ( sqrt(r1*r1 + r2*r2 -2*r1*r2*cos(theta2)) );
}


//  Note that you need to call it with a given value of alpha,
// called alf here. This comes from x^{alpha} exp(-x)
//Library function provided on the course page.
void gauss_laguerre(double *x, double *w, int n, double alf){
	int i,its,j;
	double ai;
	double p1,p2,p3,pp,z,z1;

	for (i=1;i<=n;i++) {
		if (i == 1) {
			z=(1.0+alf)*(3.0+0.92*alf)/(1.0+2.4*n+1.8*alf);
		} else if (i == 2) {
			z += (15.0+6.25*alf)/(1.0+0.9*alf+2.5*n);
		} else {
			ai=i-2;
			z += ((1.0+2.55*ai)/(1.9*ai)+1.26*ai*alf/
				(1.0+3.5*ai))*(z-x[i-2])/(1.0+0.3*alf);
		}
		for (its=1;its<=MAXIT;its++) {
			p1=1.0;
			p2=0.0;
			for (j=1;j<=n;j++) {
				p3=p2;
				p2=p1;
				p1=((2*j-1+alf-z)*p2-(j-1+alf)*p3)/j;
			}
			pp=(n*p1-(n+alf)*p2)/z;
			z1=z;
			z=z1-p1/pp;
			if (fabs(z-z1) <= EPS) break;
		}
		if (its > MAXIT) cout << "too many iterations in gaulag" << endl;
		x[i]=z;
		w[i] = -exp(gammln(alf+n)-gammln((double)n))/(pp*n*p2);
	}
}
// end function gaulag

//Library function provided on the course page.
double gammln( double xx){
	double x,y,tmp,ser;
	static double cof[6]={76.18009172947146,-86.50532032941677,
		24.01409824083091,-1.231739572450155,
		0.1208650973866179e-2,-0.5395239384953e-5};
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*log(tmp);
	ser=1.000000000190015;
	for (j=0;j<=5;j++) ser += cof[j]/++y;
	return -tmp+log(2.5066282746310005*ser/x);
}

// end function gammln
#undef EPS
#undef MAXIT


double laguerre_integrate_func_6(double u1, double u2, double theta1, double theta2, double phi1, double phi2){
  /*
  The CoulombRepulsion function used with the Gauss-Laguerre method over all 6 dimensions.
  */
  if (u1*u1 + u2*u2 - 2*u1*u2*(cos(theta1)*cos(theta2)+sin(theta1)*sin(theta2)*cos(phi1 - phi2)) <  1e-8){
    return 0;
  }

  return exp(-3*(u1+u2))*(sin(theta2)*sin(theta1)*u1*u1*u2*u2/sqrt(u1*u1 + u2*u2 - 2*u1*u2*(cos(theta1)*cos(theta2)+sin(theta1)*sin(theta2)*cos(phi1 - phi2))));
}

double laguerre_integrate_func_3(double u1, double u2, double theta2){
  /*
  The CoulombRepulsion function used with the Gauss-Laguerre method over 3 dimensions.
  */
  if(sin(theta2)/sqrt(u1*u1 + u2*u2 - 2*u1*u2*cos(theta2)) < 1e-8){
    return 0;
  }

  return (sin(theta2)/sqrt(u1*u1 + u2*u2 - 2*u1*u2*cos(theta2)));
}
