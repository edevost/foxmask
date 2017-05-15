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
class mog_diag_em_ml
  {
  
  public:
  
  ~mog_diag_em_ml();
  
  mog_diag_em_ml
    (
    const mog_diag<eT>&     in_model,
    const field< Col<eT> >& in_X,
    const u32  in_n_iter       = 1,
    const eT   in_var_floor    = Math<eT>::eps(),
    const eT   in_weight_floor = Math<eT>::eps(),
    const bool in_verbose      = false
    );

  
  void run(mog_diag<eT>& out_model);
  
  
  
  private:
  
  const field< Col<eT> >& X;
  
  const u32 n_dim;
  const u32 n_gaus;
  const u32 n_iter;
  const u32 n_threads;
  
  const eT  var_floor;
  const eT  weight_floor;
  
  const bool verbose;
  
  field< Col<eT> > means;
  field< Col<eT> > covs;
  Col<eT>          weights;
  
  Col<eT> log_det_etc;
  Col<eT> log_weights;

  field< field< Col<eT> > > parallel_acc_means;
  field< field< Col<eT> > > parallel_acc_covs;
  
  field< Col<eT> > parallel_gaus_log_lhoods;
  field< Col<eT> > parallel_gaus_lhoods;
  field< Col<eT> > parallel_acc_norm_lhoods;

  field< Col<u32> > boundaries;

  eT log_limit;
  
  
  //
  //
  inline void iterate();
  inline void sync_aux_params();

  inline eT   generate_acc( field< Col<eT> >& local_acc_means, field< Col<eT> >& local_acc_covs, Col<eT>& local_acc_norm_lhoods, Col<eT>& local_gaus_log_lhoods, Col<eT>& local_gaus_lhoods, const u32 start_index, const u32 end_index) const;
  inline eT   update_params();

  inline void fix_params();
  inline eT   internal_log_lhood_single(const eT* x_mem, const u32 gaus_id) const;
  };
