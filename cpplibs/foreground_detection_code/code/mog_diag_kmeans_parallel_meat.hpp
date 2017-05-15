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
mog_diag_kmeans<eT>::~mog_diag_kmeans()
  {
  arma_extra_debug_sigprint();
  }



template<typename eT>
inline
mog_diag_kmeans<eT>::mog_diag_kmeans
  (
  const field< Col<eT> >& in_X,
  const u32  in_n_gaus,
  const u32  in_n_iter,
  const bool in_normalise,
  const bool in_verbose
  )
  : X(in_X)
  , n_dim(in_X(0).n_elem)
  , n_gaus(in_n_gaus)
  , n_iter(in_n_iter)
  , n_threads(omp_get_num_procs())
  , normalise(in_normalise)
  , verbose(in_verbose)
  {
  arma_extra_debug_sigprint();
  
  if(verbose)
    {
    cout << "n_threads = " << n_threads << endl;
    }

  local_offset = 0;
  
  overall_mu.zeros(n_dim);
  overall_sd.zeros(n_dim);
  dist_weight.zeros(n_dim);
  
  means_A.set_size(n_gaus);
  means_B.set_size(n_gaus);
  
  covs_A.set_size(n_gaus);
  covs_B.set_size(n_gaus);
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    means_A(gaus_id).zeros(n_dim);
    means_B(gaus_id).zeros(n_dim);
    
    covs_A(gaus_id).zeros(n_dim);
    covs_B(gaus_id).zeros(n_dim);
    }
  
  means_new_ptr = &means_A;
  means_cur_ptr = &means_B;
  
  covs_new_ptr  = &covs_A;
  covs_cur_ptr  = &covs_B;

  
  parallel_acc_mu.set_size(n_threads);
  parallel_acc_cov.set_size(n_threads);
  parallel_counts.set_size(n_threads);

  for(u32 thread_id=0; thread_id<n_threads; ++thread_id)
    {
    parallel_acc_mu(thread_id).set_size(n_gaus);
    parallel_acc_cov(thread_id).set_size(n_gaus);

    for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
      {
      parallel_acc_mu(thread_id)(gaus_id).zeros(n_dim);
      parallel_acc_cov(thread_id)(gaus_id).zeros(n_dim);
      }

    parallel_counts(thread_id).set_size(n_gaus);
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
mog_diag_kmeans<eT>::run(mog_diag<eT>& out_model, const eT trust)
  {
  arma_extra_debug_sigprint();
  
  calc_overall_stats();
  
  if(normalise)
    {
    for(u32 i=0; i<n_dim; ++i)
      {
      dist_weight(i) = eT(1) / overall_sd(i);
      }
    }
  else
    {
    dist_weight = ones< Col<eT> >(n_dim);
    }
  
  
  initial_means();
  iterate();
  
  const field< Col<eT> >& means_new  = (*means_new_ptr);
  const field< Col<eT> >& covs_new   = (*covs_new_ptr);
  const Col<u32>&         counts     = parallel_counts(0);

  field< Col<eT> > covs(n_gaus);
  Col<eT>  weights(n_gaus);
  
  for(u32 i=0; i<n_gaus; ++i)
    {
    covs(i)    = trust*( covs_new(i) )                  + ( eT(1)-trust )*ones< Col<eT> >(n_dim);
    weights(i) = trust*( eT(counts(i)) / eT(X.n_elem) ) + ( eT(1)-trust )*( eT(1) / eT(n_gaus) );
    }
  
  out_model = mog_diag<eT>(means_new, covs, weights);
  }



template<typename eT>
inline
void
mog_diag_kmeans<eT>::calc_overall_stats()
  {
  arma_extra_debug_sigprint();
  
  overall_mu.zeros();
  overall_sd.zeros();
  
  for(u32 i=0; i<X.n_elem; ++i)
    {
    overall_mu += X(i);
    overall_sd += square(X(i));
    }
  
  overall_mu /= eT(X.n_elem);
  overall_sd = sqrt( overall_sd / eT(X.n_elem) - square(overall_mu) );
  
  for(u32 i=0; i<n_dim; ++i)
    {
    overall_sd(i) = (overall_sd(i) >= Math<eT>::eps() ) ? overall_sd(i) : eT(1);
    }
  
  }



template<typename eT>
inline
void
mog_diag_kmeans<eT>::initial_means()
  {
  arma_extra_debug_sigprint();
  
  const u32 step = u32(floor(eT(X.n_elem) / eT(n_gaus)));
  
  Col<eT>  tmp;
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    tmp = eT(0.4)*X(gaus_id*step) + eT(0.6)*overall_mu;
    
    (*means_new_ptr)(gaus_id) = tmp;
    (*means_cur_ptr)(gaus_id) = tmp;
    }
  }



template<typename eT>
inline
void
mog_diag_kmeans<eT>::iterate()
  {
  arma_extra_debug_sigprint();
  
  for(u32 i=0; i<n_iter; i++)
    {
    calc_new_means();
    
    if(check_new_means() == false)
      {
      return;
      }
    
    const eT change = measure_change();
  
    if(verbose)
      {
      std::cout << "mog_diag_kmeans::iterate(): iteration = " << i << "  change = " << change << '\n';
      }
    
    if(change <= math::eps())
      {
      break;
      }
  
    std::swap(means_cur_ptr, means_new_ptr);
    std::swap(covs_cur_ptr,  covs_new_ptr);
    }
  
  if(verbose)
    {
    std::cout.flush();
    }
  
  }



template<typename eT>
inline
void
mog_diag_kmeans<eT>::gen_acc
  (
  field< Col<eT> >&       acc_mu,
  field< Col<eT> >&       acc_cov,
  Col<u32>&               counts,
  const u32               start_index,
  const u32               end_index,
  const field< Col<eT> >& means
  )
  const
  {
  arma_extra_debug_sigprint();
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    acc_mu(gaus_id).zeros();
    acc_cov(gaus_id).zeros();
    }

  counts.fill(0);
  
  for(u32 i=start_index; i<=end_index; i++)
    {
    const Col<eT>& x = X(i);
    
    u32 gaus_winner = 0;
    eT min_dist = dist(x, means(gaus_winner));
    
    for(u32 gaus_id=1; gaus_id<n_gaus; ++gaus_id)
      {
      const eT tmp_dist = dist(x, means(gaus_id));
      
      if(min_dist > tmp_dist)
        {
        min_dist    = tmp_dist;
        gaus_winner = gaus_id;
        }
      }
    
    acc_mu(gaus_winner)  += x;
    acc_cov(gaus_winner) += square(x);
    
    counts(gaus_winner)++;
    }

  }



template<typename eT>
inline
void
mog_diag_kmeans<eT>::calc_new_means()
  {
  arma_extra_debug_sigprint();
  
  const field< Col<eT> >& means_cur = (*means_cur_ptr);
  
  omp_set_dynamic(0);
  omp_set_num_threads(n_threads);

  #pragma omp parallel for
  for(u32 thread_id=0; thread_id<n_threads; ++thread_id)
    {
    gen_acc
      (
      parallel_acc_mu(thread_id),
      parallel_acc_cov(thread_id),
      parallel_counts(thread_id),
      boundaries(thread_id)(0),
      boundaries(thread_id)(1),
      means_cur
      );
    }


  field< Col<eT> >& final_acc_mu  = parallel_acc_mu(0);
  field< Col<eT> >& final_acc_cov = parallel_acc_cov(0);
  Col<u32>&         final_counts  = parallel_counts(0);

  for(u32 thread_id=1; thread_id<n_threads; ++thread_id)
    {
    for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
      {
      final_acc_mu(gaus_id)  += parallel_acc_mu(thread_id)(gaus_id);
      final_acc_cov(gaus_id) += parallel_acc_cov(thread_id)(gaus_id);
      final_counts(gaus_id)  += parallel_counts(thread_id)(gaus_id);
      }
    }

  field< Col<eT> >& means_new = (*means_new_ptr);
  field< Col<eT> >& covs_new  = (*covs_new_ptr);
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    const u32 tmp = final_counts(gaus_id);
    if(tmp > 0)
      {
      means_new(gaus_id) = final_acc_mu(gaus_id)  / eT(tmp);
      covs_new(gaus_id)  = final_acc_cov(gaus_id) / eT(tmp) - square( means_new(gaus_id) );
      }
    }
  
  }



template<typename eT>
inline
bool
mog_diag_kmeans<eT>::check_new_means()
  {
  arma_extra_debug_sigprint();
  
  // TODO:
  // in armadillo, need to implement a secondary max function which also returns the index of the max value

  const Col<u32>& counts = parallel_counts(0);
  
  u32 gaus_id_hog = 0;
  u32 max_count = parallel_counts(0)(gaus_id_hog);

  for(u32 gaus_id=1; gaus_id<n_gaus; ++gaus_id)
    {
    if(parallel_counts(0)(gaus_id) > max_count)
      {
      max_count   = counts(gaus_id);
      gaus_id_hog = gaus_id;
      }
    }


  field< Col<eT> >& means_new = (*means_new_ptr);

  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    if(counts(gaus_id) == 0)
      {
      if(verbose)
        {
        std::cerr << "mog_diag_kmeans::check_new_means(): detected zombie mean" << std::endl;
        }
      
      if(gaus_id == gaus_id_hog)
        {
        std::cout << "mog_diag_kmeans::check_new_means(): cannot fix zombie means" << std::endl;
        return false;
        }
      
      //cout << means_new(gaus_id) << endl;
      means_new(gaus_id) = eT(0.9)*means_new(gaus_id_hog) + eT(0.1)*X(local_offset);
      //cout << means_new(gaus_id) << endl;
      
      ++local_offset;
      local_offset = (local_offset >= X.n_elem) ? 0 : local_offset;
      
      }
    
    }

  return true;
  }



template<typename eT>
inline
eT
mog_diag_kmeans<eT>::measure_change() const
  {
  arma_extra_debug_sigprint();
  
  const field< Col<eT> >& A = (*means_new_ptr);
  const field< Col<eT> >& B = (*means_cur_ptr);
  
  eT val = eT(0);
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    val += dist(A(gaus_id), B(gaus_id));
    }
  
  return val;
  }



template<typename eT>
inline
eT
mog_diag_kmeans<eT>::dist(const Col<eT>& x, const Col<eT>& y) const
  {
  const eT* x_mem = x.mem;
  const eT* y_mem = y.mem;
  
  const eT* dist_weight_mem = dist_weight.mem;
  
  eT acc = eT(0);
  
  for(u32 i=0; i<n_dim; i++)
    {
    const eT tmp = (x_mem[i] - y_mem[i]) * dist_weight_mem[i];
    acc += tmp*tmp;
    }
  
  return acc;
  }
