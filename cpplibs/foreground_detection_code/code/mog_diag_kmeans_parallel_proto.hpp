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
class mog_diag_kmeans
  {

  public:
  
  ~mog_diag_kmeans();
  
  mog_diag_kmeans
    (
    const field< Col<eT> >& in_X,
    const u32  in_n_gaus    = 1,
    const u32  in_n_iter    = 1,
    const bool in_normalise = true,
    const bool in_verbose   = false
    );


  
  void run(mog_diag<eT>& out_model, const eT trust = eT(0.9));

  //
  //

  private:
  
  const field< Col<eT> >& X;

  const u32 n_dim;
  const u32 n_gaus;
  const u32 n_iter;
  const u32 n_threads;

  const bool normalise;
  const bool verbose;
  
  u32 local_offset;
  
  Col<eT> overall_mu;
  Col<eT> overall_sd;
  Col<eT> dist_weight;

  field< Col<eT> >* means_new_ptr;
  field< Col<eT> >* means_cur_ptr;
  
  field< Col<eT> >* covs_new_ptr;
  field< Col<eT> >* covs_cur_ptr;

  field< Col<eT> > means_A;
  field< Col<eT> > means_B;
  
  field< Col<eT> > covs_A;
  field< Col<eT> > covs_B;

  field< field< Col<eT> > > parallel_acc_mu;
  field< field< Col<eT> > > parallel_acc_cov;
  field< Col<u32> >         parallel_counts;

  field< Col<u32> > boundaries;
  
  
  //
  //

  inline void calc_overall_stats();
  inline void initial_means();
  inline void iterate();

  inline void gen_acc(field< Col<eT> >& acc_mu, field< Col<eT> >& acc_cov, Col<u32>& counts, const u32 start_index, const u32 end_index, const field< Col<eT> >& means) const;
  inline void calc_new_means();

  inline bool check_new_means();
  inline eT   measure_change() const;
  inline eT   dist(const Col<eT>& x, const Col<eT>& y) const;
  };



