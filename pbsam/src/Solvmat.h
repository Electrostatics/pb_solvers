//
//  Solvmat.h
//  pbsam_xcode
//
//  Created by David Brookes on 6/20/16.
//  Copyright © 2016 David Brookes. All rights reserved.
//

#ifndef Solvmat_h
#define Solvmat_h

#include <stdio.h>
#include "MyMatrix.h"
#include "util.h"
#include "System.h"
#include "SHCalc.h"
#include "BesselCalc.h"
#include "TMatrix.h"

/*
 References:
 [1] Yap, E., Head-Gordon, T. 2010. JCTC
 [2] Yap, E., Head-Gordon, T. 2013. JCTC
 */


// For calculating the n grid points on surface
int calc_n_grid_pts(int poles, double r);

// use Rakhmanov method
vector<Pt> make_uniform_sph_grid(int m_grid, double r);

double inner_prod( MyMatrix<double> & M1, MyMatrix<double> & M2, int p );

/*
 Base class for coefficients of expansion
 */
class ExpansionConstants
{
protected:
  MyVector<double> expansionConst1_; // (2*l+1)/(4*pi)
  MyVector<double> expansionConst2_; // 1 / (2*l+1)
  vector<vector<int> > imatLoc_; // Locator for imat coeffs
  int p_;
  
  void compute_coeffs();
  
public:
  ExpansionConstants(int p);
  
  void set_const1_l(int l, double val) { expansionConst1_.set_val(l, val); }
  void set_const2_l(int l, double val) { expansionConst2_.set_val(l, val); }
  double get_const1_l(int l)        { return expansionConst1_[l]; }
  double get_const2_l(int l)        { return expansionConst2_[l]; }
  vector<int> get_imat_loc(int m)   { return imatLoc_[m]; }
  const int get_p() const           { return p_; }
};




/*
 Base class for matrices that will be re-expanded
 */
class ComplexMoleculeMatrix
{
protected:
  vector<MyMatrix<cmplx> > mat_;
  int p_;  // number of poles
  int I_;  // Index of molecule that this matrix
  
public:
  ComplexMoleculeMatrix(int I, int ns, int p);
  
  cmplx get_mat_knm(int k, int n, int m) { return mat_[k](n, m+p_); }
  void set_mat_knm(int k, int n, int m, cmplx val)
  { mat_[k].set_val(n, m+p_, val); }
  MyMatrix<cmplx> get_mat_k(int k)       { return mat_[k]; }
  const int get_I() const   { return I_; }
  const int get_p() const   { return p_; }
  const int get_ns() const  { return (int) mat_.size(); }
  
  void reset_mat();
  
  friend ostream & operator<<(ostream & fout, ComplexMoleculeMatrix & M)
  {
    for (int k = 0; k < M.get_ns(); k++)
    {
      fout << "For sphere " << k << endl;
      for (int n = 0; n < M.get_p(); n++)
      {
        for (int m = -n; m <= n; m++)
        {
          double real = M.get_mat_knm( k, n, m).real();
          double imag = M.get_mat_knm( k, n, m).imag();
          if(abs(real) < 1e-15 ) real = 0.0;
          if(abs(imag) < 1e-15 ) imag = 0.0;
          fout << "(" << real << ", " << imag << ") ";
        }
        fout << endl;
      }
      fout << endl;
    }
    return fout;
  }
  
};


class EMatrix;
class LEMatrix;
class IEMatrix;
class HMatrix;
class FMatrix;
class LFMatrix;
class LHMatrix;
class LHNMatrix;
class XFMatrix;
class XHMatrix;


/*
 Pre-computed values representing fixed charges within each sphere. Each object
 of this class refers to one molecule. This is then a vector of matrices where
 the vector index, k, loops through each coarse-grained sphere in the molecule.
 The indices of the inner matrices are then n and m, which loop over the number
 of poles in the system. See eq. 8a in Yap 2010 for more info
 */
class EMatrix: public ComplexMoleculeMatrix
{
public:
  EMatrix(int I, int ns, int p);
  virtual void calc_vals(Molecule mol, shared_ptr<SHCalc> sh_calc,
                         double eps_in);
  
};

/*
 Pre-computed values representing fixed charges outside each sphere. Each object
 of this class refers to one molecule. This is then a vector of matrices where
 the vector index, k, loops through each coarse-grained sphere in the molecule.
 The indices of the inner matrices are then n and m, which loop over the number
 of poles in the system. See eq. 8b in Yap 2010 for more info
 */
class LEMatrix : public ComplexMoleculeMatrix
{
public:
  LEMatrix(int I, int ns, int p);
  void calc_vals(Molecule mol, shared_ptr<SHCalc> sh_calc,
                 double eps_in);
};


/*
 Class for pre-computing values of surface integral matrices I_E. Each object
 of this class refers to one molecule. See equation 21 in Yap 2010 for more
 info.
 */
class IEMatrix
{
protected:
  
  // indices in order are k, (n, m), (l, s)
  vector<MatOfMats<cmplx>::type > IE_;
  vector<vector<double> > IE_orig_;
  shared_ptr<ExpansionConstants> _expConst_;
  bool calc_pts_; // Boolean of whether or not to estimate number of points
  int p_;
  int I_;
  int gridPts_; // grid point count for surface integrals
  
  void set_IE_k_nm_ls(int k, int n, int m, int l, int s, cmplx val)
  {
    IE_[k](n, m+p_).set_val(l, s+p_, val);
  }
  
public:
  IEMatrix(int I, int ns, int p, shared_ptr<ExpansionConstants> _expconst);
  
  IEMatrix(int I, shared_ptr<Molecule> _mol, shared_ptr<SHCalc> sh_calc, int p,
           shared_ptr<ExpansionConstants> _expconst, bool calc_npts = false,
           int npts = Constants::IMAT_GRID );
  
  cmplx get_IE_k_nm_ls(int k, int n, int m, int l, int s)
  {
    return IE_[k](n, m+p_)(l, s+p_);
  }
  double get_IE_k_ind(int k, int ind) { return IE_orig_[k][ind]; }
  
  MyMatrix<cmplx> get_IE_knm(int k, int n, int m) { return IE_[k](n, m+p_); }
  
  void compute_grid_pts(shared_ptr<Molecule> _mol);
  vector<MatOfMats<cmplx>::type >compute_integral(shared_ptr<Molecule> _mol,
                                                  shared_ptr<SHCalc> sh_calc,
                                                  int k);
  void populate_mat(vector<MatOfMats<cmplx>::type > Ys, int k);
  void calc_vals(shared_ptr<Molecule> _mol, shared_ptr<SHCalc> sh_calc);
  void reset_mat();
};


class HMatrix;
class FMatrix;

/*
 Base class for numerical matrices
 */
class NumericalMatrix
{
protected:
  vector<vector<double> > mat_;
  int p_;  // number of poles
  int I_;  // Index of molecule that this matrix
  
public:
  NumericalMatrix(int I, int ns, int p);
  
  double get_mat_kh(int k, int h)           { return mat_[k][h]; }
  void set_mat_kh(int k, int h, double val) { mat_[k][h] = val; }
  vector<double> get_mat_k(int k)           { return mat_[k]; }
  vector<vector<double> > get_mat()         { return mat_; }
  int get_mat_k_len(int k)                 { return (int)mat_[k].size(); }
  const int get_I() const   { return I_; }
  const int get_p() const   { return p_; }
  const int get_ns() const  { return (int) mat_.size(); }
  
  void reset_mat();
  
  friend ostream & operator<<(ostream & fout, NumericalMatrix & M)
  {
    for (int k = 0; k < M.get_ns(); k++)
    {
      fout << "For sphere " << k << endl;
      for (int h = 0; h < M.get_mat_k_len(h); h++)
      {
        double real = M.get_mat_kh( k, h);
        if(abs(real) < 1e-15 ) real = 0.0;
        fout << real << ", ";
      }
      fout << endl;
    }
    return fout;
  }
  
};



/*
 Equation 8c
 */
class LFMatrix : public NumericalMatrix
{
public:
  LFMatrix(int I, int ns, int p);
  
  void calc_vals(shared_ptr<TMatrix> T, shared_ptr<FMatrix> F,
                 shared_ptr<SHCalc> shcalc, shared_ptr<System> sys);
  
  // analytic re-expansion (Equation 27a [1])
  MyMatrix<cmplx> numeric_reex(int I, int k, int j,
                               shared_ptr<FMatrix> F,
                               shared_ptr<SHCalc> shcalc,
                               shared_ptr<System> sys, int Mp=-1);
  
  /*
   Equation 15a [1]. For analytic re expansion
   */
  cmplx make_fb_Ij(int I, int j, Pt rb,
                   shared_ptr<FMatrix> F,
                   shared_ptr<SHCalc> shcalc);
};

/*
 Equation 10b [1]
 */
class LHMatrix : public NumericalMatrix
{
protected:
  double kappa_;
  
public:
  LHMatrix(int I, int ns, int p, double kappa);
  
  void init(Molecule mol, shared_ptr<HMatrix> H,
            shared_ptr<SHCalc> shcalc, shared_ptr<BesselCalc> bcalc,
            shared_ptr<ExpansionConstants> _expconst);
  
  void calc_vals(shared_ptr<TMatrix> T, shared_ptr<HMatrix> H,
                 shared_ptr<SHCalc> shcalc, shared_ptr<System> sys,
                 shared_ptr<BesselCalc> bcalc, int Mp=-1);
  
  // analytic re-expansion (Equation 27b [1])
  MyMatrix<cmplx> numeric_reex(int I, int k, int j,
                               shared_ptr<HMatrix> H,
                               shared_ptr<SHCalc> shcalc,
                               shared_ptr<System> sys,
                               vector<double> besseli,
                               vector<double> besselk,
                               int Mp=-1);
  
  /*
   Equation 15b [1]. For analytic re expansion
   */
  cmplx make_hb_Ij(int I, int j, Pt rb,
                   shared_ptr<HMatrix> H,
                   shared_ptr<SHCalc> shcalc,
                   vector<double> besseli);
  
};

/*
 Equation 10c [1]
 */
class LHNMatrix : public ComplexMoleculeMatrix
{
public:
  LHNMatrix(int I, int ns, int p);
  
  void calc_vals(shared_ptr<TMatrix> T, vector<shared_ptr<HMatrix> > H);
  
};

/*
 Equation 14a [1]
 */
class XHMatrix : public ComplexMoleculeMatrix
{
public:
  XHMatrix(int I, int ns, int p);
  
  void calc_vals(Molecule mol, shared_ptr<BesselCalc> bcalc,
                 shared_ptr<EMatrix> E, shared_ptr<LEMatrix> LE,
                 shared_ptr<LHMatrix> LH, shared_ptr<LFMatrix> LF,
                 shared_ptr<LHNMatrix> LHN);
  
};

/*
 Equation 14b [1]
 */
class XFMatrix : public ComplexMoleculeMatrix
{
protected:
  double eps_;
  
public:
  XFMatrix(int I, int ns, int p, double eps_in, double eps_out);
  
  void calc_vals(Molecule mol, shared_ptr<BesselCalc> bcalc,
                 shared_ptr<EMatrix> E, shared_ptr<LEMatrix> LE,
                 shared_ptr<LHMatrix> LH, shared_ptr<LFMatrix> LF,
                 shared_ptr<LHNMatrix> LHN, double kappa);
  
  const double get_eps() const { return eps_; }
};


class HMatrix: public ComplexMoleculeMatrix
{
protected:
  double kappa_;
  
public:
  HMatrix(int I, int ns, int p, double kappa);
  
  void init(Molecule mol, shared_ptr<SHCalc> _sh_calc, double eps_in);
  
  void calc_vals(Molecule mol, shared_ptr<HMatrix> prev,
                 shared_ptr<XHMatrix> XH,
                 shared_ptr<FMatrix> F,
                 shared_ptr<IEMatrix> IE,
                 shared_ptr<BesselCalc> bcalc);
  
  // calculate convergence criteria (Equation 23)
  static double calc_converge(shared_ptr<HMatrix> curr,
                              shared_ptr<HMatrix> prev);
  
};


class FMatrix: public ComplexMoleculeMatrix
{
protected:
  double kappa_;
  
public:
  FMatrix(int I, int ns, int p, double kappa);
  
  void calc_vals(Molecule mol, shared_ptr<FMatrix> prev,
                 shared_ptr<XFMatrix> XF,
                 shared_ptr<HMatrix> H,
                 shared_ptr<IEMatrix> IE,
                 shared_ptr<BesselCalc> bcalc);
  
};


#endif /* Solvmat_h */