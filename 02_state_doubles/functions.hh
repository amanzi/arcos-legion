//! --------------------------------------------------------------------------------
//
// Amanzi -- Legion
//
// Author: Ethan Coon (coonet@ornl.gov)
// License: BSD
//
//  A mockup of some functions for use in testing.  This can be
//  replaced with actualy physics.
//
//
//  This only deals with evaluators A-H, as in Amanzi test example
//  src/state/state_dag.cc
//  ---------------------------------------------------------------------------------

#ifndef FUNCTIONS_HH_
#define FUNCTIONS_HH_

#include <iostream>

struct FA
{
  double operator()(double b, double c, double e, double h) const {
    std::cout << "  Running FA" << std::endl;
    return 2 * b + c*e*h;
  }
  double dA_dB(double b, double c, double e, double h) const {
    return 2.;
  }
  double dA_dC(double b, double c, double e, double h) const {
    return e*h;
  }
  double dA_dE(double b, double c, double e, double h) const {
    return c*h;
  }
  double dA_dH(double b, double c, double e, double h) const {
    return c*e;
  }

  static const char* name;
};


struct FC
{
  double operator()(double d, double g) const {
    std::cout << "  Running FC" << std::endl;
    return 2*d + g;
  }
  double dC_dD(double d, double g) const {
    return 2.;
  }
  double dC_dG(double d, double g) const {
    return 1.;
  }
  static const char* name;
};


struct FD
{
  double operator()(double g) const {
    std::cout << "  Running FD" << std::endl;
    return 2*g;
  }
  double dD_dG(double g) const {
    return 2;
  }
  static const char* name;
};

struct FE
{
  double operator()(double d, double f) const {
    std::cout << "  Running FE" << std::endl;
    return d*f;
  }
  double dE_dD(double d, double f) const {
    return f;
  }
  double dE_dF(double d, double f) const {
    return d;
  }
  static const char* name;
};



struct FF
{
  double operator()(double g) const {
    std::cout << "  Running FF" << std::endl;
    return 2.*g;
  }
  double dF_dG(double g) const {
    return 2.;
  }
  static const char* name;
};


struct FH
{
  double operator()(double f) const {
    std::cout << "  Running FH" << std::endl;
    return 2*f;
  }
  double dH_dF(double f) const {
    return 2.;
  }
  static const char* name;
};


#endif
