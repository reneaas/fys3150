#include <cmath>      //included some of the function descriptions below used functions from <cmath>.
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include "time.h"

using namespace std;

//Algorithm for forward Euler method
void Explicit_scheme_1D(double *v_new, double *v_old , double r, int gridpoints, double dt, double total_time, double &t){

  while (t < total_time){
    for (int j = 1; j < gridpoints-1; j++){
      v_new[j] = (1-2*r)*v_old[j] + r*(v_old[j+1] + v_old[j-1]);
    }
    for (int k = 0; k < gridpoints; k++) v_old[k] = v_new[k];
    t += dt;
  }
}

//Generalized forward substitution of a tridiagonal matrix eq. Ax = b
void Forward_substitution(double* a, double* b, double* c, double* y, int n){
  for (int i = 1; i < n; i++){
    b[i] -= a[i-1]*c[i-1]/b[i-1];
    y[i] -= a[i-1]*y[i-1]/b[i-1];
  }
  return;
}

//Generalized backward substitution of a tridiagonal matrix eq. Ax = b.
void Back_substitution(double* x, double* b, double* c, double* y, int n){
  for (int i = n-1; i >= 0; i--){
    if (i == n-1){
      x[i] = y[i]/b[i];
    }
    else{
      x[i] = (y[i]-c[i]*x[i+1])/b[i];
    }
  }
  return;
}


//Algorithm for forward Euler method in 2D
void  Explicit_scheme_2D(double **v_new, double **v_old, double r, int gridpoints, double dt, double total_time, double& t){

  while (t < total_time){
      for (int i = 1; i < gridpoints-1; i++){
        for (int j = 1; j < gridpoints-1; j++){
            v_new[i][j] = (1-4*r)*v_old[i][j] + r*(v_old[i+1][j] + v_old[i-1][j] + v_old[i][j+1] + v_old[i][j-1]);
        }
      }
      for (int i = 0; i < gridpoints; i++){
        for (int j = 0; j < gridpoints; j++){
          v_old[i][j] = v_new[i][j];
        }
      }
    t += dt;
  }
}
