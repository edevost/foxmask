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
inline
mog_diag_em_ml<eT>::~mog_diag_em_ml()
  {
  arma_extra_debug_sigprint();
  }



template<typename eT>
inline
mog_diag_em_ml<eT>::mog_diag_em_ml
  (
  const mog_diag<eT>&              in_model,
  const field< Col<eT> >& in_X,
  
  const u32  in_n_iter,
  const eT   in_var_floor,
  const eT   in_weight_floor,
  const bool in_verbose
  )
  : X(in_X)
  , n_dim(in_model.means(0).n_elem)
  , n_gaus(in_model.means.n_elem)
  , n_iter(in_n_iter)
  , n_threads(omp_get_num_procs())
  , var_floor(in_var_floor)
  , weight_floor(in_weight_floor)
  , verbose(in_verbose)
  , means(in_model.means)
  , covs(in_model.dcovs)
  , weights(in_model.weights)
  {
  arma_extra_debug_sigprint();
  
  if(verbose)
    {
    cout << "n_threads = " << n_threads << endl;
    }

  log_det_etc.set_size(n_gaus);
  log_weights.set_size(n_gaus);

  parallel_acc_means.set_size(n_threads);
  parallel_acc_covs.set_size(n_threads);
  parallel_acc_norm_lhoods.set_size(n_threads);
  parallel_gaus_log_lhoods.set_size(n_threads);
  parallel_gaus_lhoods.set_size(n_threads);

  for(u32 thread_id=0; thread_id<n_threads; ++thread_id)
    {
    parallel_acc_means(thread_id).set_size(n_gaus);
    parallel_acc_covs(thread_id).set_size(n_gaus);

    for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
      {
      parallel_acc_means(thread_id)(gaus_id).set_size(n_dim);
      parallel_acc_covs(thread_id)(gaus_id).set_size(n_dim);
      }

    parallel_acc_norm_lhoods(thread_id).set_size(n_gaus);
    parallel_gaus_log_lhoods(thread_id).set_size(n_gaus);
    parallel_gaus_lhoods(thread_id).set_size(n_gaus);
    }

  boundaries.set_size(n_threads);
  const u32 chunk_size = X.n_elem / n_threads;

  if(verbose)
    {
    cout << "chunk_size = " << chunk_size << endl;
    cout << "X.n_elem   = " << X.n_elem << endl;
    }

  for(u32 thread_id=0; thread_id<n_threads; ++thread_id)
    {
    Col<u32>& boundary = boundaries(thread_id);
    boundary.set_size(2);

    boundary(0) = thread_id * chunk_size;
    boundary(1) = ((thread_id+1) * chunk_size) -1;
    }
  
  boundaries(n_threads-1)(1) = X.n_elem - 1;
  }



template<typename eT>
inline
void
mog_diag_em_ml<eT>::run(mog_diag<eT>& out_model)
  {
  arma_extra_debug_sigprint();
  
  iterate();
  
  out_model = mog_diag<eT>(means, covs, weights);
  }



template<typename eT>
inline
void
mog_diag_em_ml<eT>::iterate()
  {
  arma_extra_debug_sigprint();
  
  for(u32 i=0; i<n_iter; i++)
    {
    sync_aux_params();
    eT progress_log_lhood = update_params();
    fix_params();
    
    if(verbose)
      {
      std::cout << "mog_diag_em_ml::iterate(): iteration = " << i << "  progress_log_lhood = " << progress_log_lhood << '\n';
      }
    
    }
  
  if(verbose)
    {
    std::cout.flush();
    }
  
  }



template<typename eT>
inline
void
mog_diag_em_ml<eT>::sync_aux_params()
  {
  arma_extra_debug_sigprint();
  
  const eT tmp = (eT(n_dim)/eT(2)) * std::log(eT(2) * Math<eT>::pi());
  
  for(u32 i=0; i<n_gaus; ++i)
    {
    log_det_etc(i) = eT(-1) * ( tmp + eT(0.5) * std::log( det( diagmat(covs(i)) ) ) );
    }
  
  log_weights = log(weights);
  
  log_limit = std::log( std::numeric_limits<eT>::max() * max(weights) );
  }



template<typename eT>
inline
eT
mog_diag_em_ml<eT>::generate_acc
  (
  field< Col<eT> >& acc_means,
  field< Col<eT> >& acc_covs,
  Col<eT>&          acc_norm_lhoods,
  Col<eT>&          gaus_log_lhoods,
  Col<eT>&          gaus_lhoods,
  const u32         start_index,
  const u32         end_index
  )
  const
  {
  arma_extra_debug_sigprint();

  eT progress_log_lhood = eT(0);
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    acc_means(gaus_id).zeros();
    acc_covs(gaus_id).zeros();
    }
  
  acc_norm_lhoods.zeros();
  gaus_log_lhoods.zeros();
  gaus_lhoods.zeros();

  const eT* log_weights_mem  = log_weights.mem;
  
  for(u32 i=start_index; i<=end_index; i++)
    {
    const Col<eT> & x = X(i);
    const eT* x_mem = x.mem;
    
    bool overflow_danger = false;
    
    for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
      {
      const eT tmp_log_lhood = log_weights_mem[gaus_id] + internal_log_lhood_single(x_mem, gaus_id);
      gaus_log_lhoods(gaus_id) = tmp_log_lhood;
      
      if(tmp_log_lhood > log_limit)
        {
        overflow_danger = true;
        }
      }
    
    if(overflow_danger == false)
      {
      eT lhood_sum = eT(0);
      
      for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
        {
        const eT tmp_lhood = std::exp( gaus_log_lhoods(gaus_id) );
        gaus_lhoods(gaus_id) = tmp_lhood;  // can reuse gaus_log_lhoods here
        lhood_sum += tmp_lhood;
        }
      
      progress_log_lhood += std::log(lhood_sum);
      
      for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
        {
        const eT norm_lhood = gaus_lhoods(gaus_id) / lhood_sum;
        
        acc_norm_lhoods(gaus_id) += norm_lhood;
        
        eT* acc_mean_mem = acc_means(gaus_id).memptr();
        eT* acc_cov_mem  = acc_covs(gaus_id).memptr();
        
        for(u32 dim=0; dim<n_dim; ++dim)
          {
          const eT tmp1 = x_mem[dim];
          const eT tmp2 = norm_lhood * tmp1;
          
          acc_mean_mem[dim] += tmp2;
          acc_cov_mem[dim]  += tmp2*tmp1;
          }
        }
      }
    else
      {
      eT log_lhood_sum = gaus_log_lhoods(0);
      
      for(u32 gaus_id=1; gaus_id<n_gaus; ++gaus_id)
        {
        log_lhood_sum = log_add(log_lhood_sum, gaus_log_lhoods(gaus_id));
        }
      
      progress_log_lhood += log_lhood_sum;
      
      for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
        {
        const eT norm_lhood = std::exp(gaus_log_lhoods(gaus_id) - log_lhood_sum);
        
        acc_norm_lhoods(gaus_id) += norm_lhood;
        
        eT* acc_mean_mem = acc_means(gaus_id).memptr();
        eT* acc_cov_mem  = acc_covs(gaus_id).memptr();
        
        for(u32 dim=0; dim<n_dim; ++dim)
          {
          const eT tmp1 = x_mem[dim];
          const eT tmp2 = norm_lhood * tmp1;
          
          acc_mean_mem[dim] += tmp2;
          acc_cov_mem[dim]  += tmp2*tmp1;
          }
        }
      }
    }
  

  return progress_log_lhood;
  }



template<typename eT>
inline
eT
mog_diag_em_ml<eT>::update_params()
  {
  arma_extra_debug_sigprint();

  eT progress_log_lhood = eT(0);

  omp_set_dynamic(0);
  omp_set_num_threads(n_threads);

  #pragma omp parallel for reduction (+:progress_log_lhood)
  for(u32 thread_id=0; thread_id<n_threads; ++thread_id)
    {
    progress_log_lhood
      +=
      generate_acc
        (
        parallel_acc_means(thread_id),
        parallel_acc_covs(thread_id),
        parallel_acc_norm_lhoods(thread_id),
        parallel_gaus_log_lhoods(thread_id),
        parallel_gaus_lhoods(thread_id),
        boundaries(thread_id)(0),
        boundaries(thread_id)(1)
        );
    }


  field< Col<eT> >& final_acc_means       = parallel_acc_means(0);
  field< Col<eT> >& final_acc_covs        = parallel_acc_covs(0);
  Col<eT>&          final_acc_norm_lhoods = parallel_acc_norm_lhoods(0);


  for(u32 thread_id=1; thread_id<n_threads; ++thread_id)
    {
    for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
      {
      final_acc_means(gaus_id) += parallel_acc_means(thread_id)(gaus_id);
      final_acc_covs(gaus_id)  += parallel_acc_covs(thread_id)(gaus_id);
      }

    final_acc_norm_lhoods += parallel_acc_norm_lhoods(thread_id);
    }

//   final_acc_means(0).print("final_acc_means(0) =");
//   final_acc_covs(0).print("final_acc_covs(0) =");
//   final_acc_norm_lhoods.print("final_acc_norm_lhoods =");

  eT* weights_mem = weights.memptr();
    
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    eT* mean_mem = means(gaus_id).memptr();
    eT* cov_mem  = covs(gaus_id).memptr();
    
    eT* final_acc_mean_mem = final_acc_means(gaus_id).memptr();
    eT* final_acc_cov_mem  = final_acc_covs(gaus_id).memptr();
    
    const eT final_acc_norm_lhood = final_acc_norm_lhoods(gaus_id);
    
    weights_mem[gaus_id] = final_acc_norm_lhood / eT(X.n_elem);
    
    for(u32 dim=0; dim<n_dim; ++dim)
      {
      const eT tmp = final_acc_mean_mem[dim] / final_acc_norm_lhood;
      
      mean_mem[dim] = tmp;
      cov_mem[dim]  = final_acc_cov_mem[dim]/final_acc_norm_lhood - tmp*tmp;
      }
    }
  
  
  progress_log_lhood /= eT(X.n_elem);
  
  return progress_log_lhood;
  }



template<typename eT>
inline
void
mog_diag_em_ml<eT>::fix_params()
  {
  arma_extra_debug_sigprint();
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    eT* cov_mem = covs(gaus_id).memptr();
    
    for(u32 dim=0; dim<n_dim; ++dim)
      {
      if(cov_mem[dim] < var_floor)
        {
        cov_mem[dim] = var_floor;
        }
      }
    }
  
  
  eT w_sum = eT(0);
  
  eT* weights_mem = weights.memptr();
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    const eT w_old = weights_mem[gaus_id];
    const eT w_new = (w_old < weight_floor) ? weight_floor : ( (w_old > eT(1)) ? eT(1) : w_old );
    
    weights_mem[gaus_id] = w_new;
    w_sum += w_new;
    }
  
  weights /= w_sum;
  }



template<typename eT>
inline
eT
mog_diag_em_ml<eT>::internal_log_lhood_single(const eT* x_mem, const u32 gaus_id) const
  {
  arma_extra_debug_sigprint();
  
  const eT* mean_mem = means(gaus_id).mem;
  const eT* cov_mem  = covs(gaus_id).mem;
  
  eT val = eT(0);
  
  for(u32 i=0; i<n_dim; ++i)
    {
    const eT tmp = x_mem[i] - mean_mem[i];
    val += (tmp*tmp) / cov_mem[i];
    }
  
  return log_det_etc.mem[gaus_id] - eT(0.5)*val;
  }
