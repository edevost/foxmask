// Copyright (C) 2009 - 2012 NICTA
// 
// Authors:
// - Conrad Sanderson (conradsand at ieee dot org)
//
// This file is provided without any warranty of
// fitness for any purpose. You can redistribute
// this file and/or modify it under the terms of
// the GNU General Public License (GPL) as published
// by the Free Software Foundation, either version 3
// of the License or (at your option) any later version.
// (see http://www.opensource.org/licenses for more info)



template<typename eT>
class mog_diag
  {

  public:

  const u32 n_gaus;
  const u32 n_dim;
  
  arma_aligned   field< Col<eT> > means;
  arma_aligned   field< Col<eT> > dcovs;
  arma_aligned   Col<eT>          weights;
  
  //
  //
  
  inline ~mog_diag();
  inline  mog_diag();
  
  inline                  mog_diag(const mog_diag& x);
  inline const mog_diag& operator=(const mog_diag& x);
  
  inline mog_diag(const std::string name);
  inline mog_diag(const u32 in_n_gaus, const u32 in_n_dim);
  
  template<typename eT2>
  inline mog_diag
    (
    const field< Col<eT2> >& in_means,
    const field< Col<eT2> >& in_dcovs,
    const Col<eT2> & in_weights
    );
  
  inline void reset();
  
  inline void load(const std::string name);
  inline void save(const std::string name) const;
  
  arma_inline eT log_lhood_single(const Col<eT> & x, const u32 gaus_id) const;
  inline eT log_lhood(const Col<eT> & x) const;
  inline eT avg_log_lhood(const field<Col<eT> >& X) const;
  
  inline void adapt(bool& out_matchflg, eT& out_log_lhood, u32& out_maxval_idx, const Col<eT>& x, const eT threshold, const eT alpha, const eT rho_fixed);
  inline void compute_loglikelihood(eT& out_log_lhood, const Col<eT>& x);


  inline void hist_norm(Col<eT>& out, const Col<eT>& x, const bool use_weights = true) const;
  
  inline bool check_size(const Col<eT> & x) const;
  inline bool check_size(const field< Col<eT> >& X) const;
  
  inline void train_kmeans
    (
    const field< Col<eT> >& in_X,
    const u32 in_n_gaus,
    const u32 in_n_iter,
    const eT in_trust,
    const bool in_normalise = true,
    const bool in_verbose   = false
    );
  
  inline void train_em_ml
    (
    const field< Col<eT> >& in_X,
    const u32 in_n_iter,
    const eT in_var_floor    = Math<eT>::eps(),
    const eT in_weight_floor = Math<eT>::eps(),
    const bool in_verbose = false
    );
  

  //
  //
  
  protected:
  
  inline void init(const mog_diag& x);
  inline void init(const u32 in_n_gaus, const u32 in_n_dim);
  
  template<typename eT2>
  inline void init
    (
    const field< Col<eT2> >& in_means,
    const field< Col<eT2> >& in_dcovs,
    const Col<eT2>& in_weights
    );
  
  inline void init_constants();
  
  inline eT internal_log_lhood_single(const eT* x, const u32 gaus_id) const;
  inline eT internal_log_lhood(const eT* x) const;
  
  //
  //
  
  arma_aligned eT log_limit;

  arma_aligned Col<eT> log_det_etc;
  arma_aligned Col<eT> log_weights;

  arma_aligned field< Col<eT> > dcovs_inv_etc;


  template<typename T1> T1& rw(const T1& x) { return const_cast<T1&>(x); }
  };
