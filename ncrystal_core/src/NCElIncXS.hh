#ifndef NCrystal_ElIncXS_hh
#define NCrystal_ElIncXS_hh

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   //
//                                                                            //
//  Copyright 2015-2019 NCrystal developers                                   //
//                                                                            //
//  Licensed under the Apache License, Version 2.0 (the "License");           //
//  you may not use this file except in compliance with the License.          //
//  You may obtain a copy of the License at                                   //
//                                                                            //
//      http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                            //
//  Unless required by applicable law or agreed to in writing, software       //
//  distributed under the License is distributed on an "AS IS" BASIS,         //
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//  See the License for the specific language governing permissions and       //
//  limitations under the License.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "NCrystal/NCDefs.hh"
#include <vector>
#include <utility>

namespace NCrystal {

  class ElIncXS {
  public:

    //Helper class which implements a model of incoherent elastic scattering,
    //based on mean-squared-displacements and bound incoherent cross sections
    //for each element. The basic mono-atomic formulas are available in static
    //methods, while a class instance allows calculations and samplings of
    //polyatomic systems.
    //
    //The constructor takes three lists of element data (must be of equal
    //length): The mean-squared-displacements, the bound incoherent cross
    //sections, and the scale (weight) of the element. The scale parameter is
    //typically 1.0 in mono-atomic systems and otherwise represents the fraction
    //of elements (by count). E.g. for sapphire (Al2O3), Al should be added with
    //a scale of 0.4 and O with a fraction of 0.6.

    ElIncXS( const std::vector<double>& elements_meanSqDisp,
             const std::vector<double>& elements_boundincohxs,
             const std::vector<double>& elements_scale );

    //Default constructor. Representing no elements added, so evaluate(..) will
    //always return 0.0. Usually this constructor is meant to be used with a
    //subsequent invocation of set(..):
    ElIncXS();

    //(re)initialise:
    void set( const std::vector<double>& elements_meanSqDisp,
              const std::vector<double>& elements_boundincohxs,
              const std::vector<double>& elements_scale );


    ~ElIncXS(){}

    //Empty if no elements added (evaluate() will always return zero).
    bool empty() const { return m_elm_data.empty(); }

    //Evaluate the incoherent elastic cross section:
    static double evaluateMonoAtomic(double ekin, double meanSqDisp, double bound_incoh_xs);
    double evaluate(double ekin) const;

    //Sample cosine of scatter angle:
    static double sampleMuMonoAtomic( RandomBase *, double ekin, double meanSqDisp );
    double sampleMu( RandomBase *, double ekin );

    ////////////////////////////////////////////////////////////////////////////////////
    //
    // Theoretical background of implemented model:
    //
    // As per the second equation in section 2.3 of arxiv 1901.08890 (equation
    // #23), the per-atomic incoherent elastic cross section due to atoms of a
    // given element is:
    //
    //    XS = sigma_inc/4pi * exp(-2*W)
    //
    // Where sigma_inc is the incoherent cross section of the element in question
    // and W is the Debye-Waller function, which in the Debye model with
    // isotropic atomic displacements (eq. #40) becomes 1/2 * Q^2 * d^2 where Q is
    // the wavevector transfer (Q=kf-ki) and d^2 the mean squared displacements
    // (projected along a given direction). Thus:
    //
    //   XS = sigma_inc/4pi * exp(-Q^2*d^2)
    //
    // To get the total cross section at a given kinetic energy (and
    // corresponding k=ki=kf), one must integrate over all outgoing directions of
    // kf. Using mu=costheta and Q^2 = ki^2+kf^2-2ki*kf*mu = 2k^2*(1-mu).
    //
    // Now, dOmega = dmu dphi and integration over dphi yields a factor of 2pi, so:
    //
    //  XStot = sigma_inc/4pi * 2*pi\int_{-1}^{1} [ exp(-2k^2*d^2*(1-mu)) ] dmu   [***]
    //        = sigma_inc/(4k^2d^2) * exp(-2k^2*d^2) * [exp((2k^2*d^2)*mu]_{-1}^{1}
    //        = sigma_inc/(4k^2d^2) * ( 1 - exp(-4k^2*d^2) )
    //        = sigma_inc * ( 1-exp(-t) ) / t  with  t = 4k^2d^2 = (2kd)^2
    //
    // And E = p^2/2m, lambda=2pi/k, p=hbar*k so t = (16*pi^2)*d^2 / lambda^2.
    //
    // In order to sample a value of mu=costheta for a particular scattering, it
    // must occur as per the integrand in [***], i.e. a mu value must be sampled in
    // [-1,1] with probability proportional to exp(a*mu) where a=2*k^2*d^2.
    //
    ////////////////////////////////////////////////////////////////////////////////////

  private:
    std::vector<std::pair<double,double> > m_elm_data;//for exact eval, (msd,boundincohxs*scale)
    static double eval_1mexpmtdivt(double t);//safe/fast eval of (1-exp(-t))/t for t>=0.0 with >10 sign. digits

  };
}

#endif
