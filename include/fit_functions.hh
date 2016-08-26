#ifndef FIT_FUNCTIONS_h
#define FIT_FUNCTIONS_h

#include <cmath>
#include <iostream>

// Normal distribution
double Normal(double x, double N, double mu, double sg){
  double pi = acos(-1.);
  double G = exp(-pow((x - mu),2) / (2 * pow(sg,2))) / (sqrt(2 * pi) * sg);
  return N * G;
}

double Gaus(double* xs, double* par){
  return Normal(xs[0], par[0], par[1], par[2]);
}

double DoubleGaus(double* xs, double* par){
  double G0 = Normal(xs[0], par[0], par[4]+par[1], par[2]);
  double G1 = Normal(xs[0], par[3], par[4], par[5]);
  return G0 + G1;
}

double DoubleGaus_SameNorm(double* xs, double* par){
  double G0 = Normal(xs[0], par[0], par[3]+par[1], par[2]);
  double G1 = Normal(xs[0], par[0], par[3], par[4]);
  return G0 + G1;
}

double P0_P2_P1(double* xs, double* par){
  double x = xs[0];

  double c0 = par[0];
  double A2 = par[1];
  double t02 = par[2];
  double d21 = par[3];

  if(x < t02)
    return c0;
  if(x < t02+d21)
    return c0 + A2*(x-t02)*(x-t02);

  return c0 + A2*d21*(2.*x - d21 - 2*t02);
}

double P1_P2_P0(double* xs, double* par){
  double x = xs[0];

  double c0 = par[0];
  double A2 = par[1];
  double t02 = par[2];
  double d21 = par[3];
 
  if(x > t02)
    return c0;
  if(x > t02+d21)
    return c0 + A2*(x-t02)*(x-t02);

  return c0 + A2*d21*(2.*x - d21 - 2*t02);
}

double P1(double* xs, double* par){
  double x = xs[0];

  double c = par[0];
  double m = par[1];
  
  return m*x+c;
}

#endif
