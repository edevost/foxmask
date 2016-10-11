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
mog_diag<eT>::~mog_diag()
  {
  arma_extra_debug_sigprint();
  }



template<typename eT>
inline
mog_diag<eT>::mog_diag()
  : n_gaus(0)
  , n_dim(0)
  {
  arma_extra_debug_sigprint();
  }



template<typename eT>
inline
mog_diag<eT>::mog_diag(const mog_diag<eT>& x)
  : n_gaus(0)
  , n_dim(0)
  {
  arma_extra_debug_sigprint();
  
  init(x);
  }



template<typename eT>
inline
const mog_diag<eT>&
mog_diag<eT>::operator=(const mog_diag<eT>& x)
  {
  arma_extra_debug_sigprint();
  
  init(x);
  
  return *this;
  }



template<typename eT>
inline
mog_diag<eT>::mog_diag(const std::string name)
  : n_gaus(0)
  , n_dim(0)
  {
  arma_extra_debug_sigprint();
  
  load(name);
  }


template<typename eT>
inline
mog_diag<eT>::mog_diag(const u32 in_n_gaus, const u32 in_n_dim)
  : n_gaus(0)
  , n_dim(0)
  {
  arma_extra_debug_sigprint();
  init(in_n_gaus, in_n_dim);
  }



template<typename eT>
template<typename eT2>
inline
mog_diag<eT>::mog_diag
  (
  const field< Col<eT2> >& in_means,
  const field< Col<eT2> >& in_dcovs,
  const Col<eT2>& in_weights
  )
  : n_gaus(0)
  , n_dim(0)
  {
  arma_extra_debug_sigprint();
  
  init(in_means, in_dcovs, in_weights);
  }



template<typename eT>
inline
void
mog_diag<eT>::reset()
  {
  init(0,0);
  }



template<typename eT>
inline
void
mog_diag<eT>::load(const std::string name)
  {
  arma_extra_debug_sigprint();
  
  bool load_okay = true;
  
  rw(means).load  ( name + ".means" );
  rw(dcovs).load  ( name + ".dcovs" );
  rw(weights).load( name + ".wghts" );
        
  if(
      ( (means.n_elem == 0) || (dcovs.n_elem == 0) || (weights.n_elem == 0) )
      ||
      ( (means.n_elem != dcovs.n_elem) || (means.n_elem != weights.n_elem) )
    )
    {
    cout << "problem with loaded model" << endl;
    load_okay = false;
    }
  
  
  if(load_okay == true)
    {
    init_constants();
    }
  else
    {
    reset();
    }
  }



template<typename eT>
inline
void
mog_diag<eT>::save(const std::string name) const
  {
  arma_extra_debug_sigprint();
  
  means.save  ( name + ".means" );
  dcovs.save  ( name + ".dcovs" );
  weights.save( name + ".wghts" );
  }



template<typename eT>
inline
eT
mog_diag<eT>::log_lhood_single(const Col<eT>& x, const u32 gaus_id) const
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( !check_size(x),      "mog_diag::log_lhood_single(): incompatible dimensions" );
  arma_debug_check( (gaus_id >= n_gaus), "mog_diag::log_lhood_single(): gaus_id is out of bounds" );
  
  return internal_log_lhood_single(x.mem, gaus_id);
  }



template<typename eT>
inline
eT
mog_diag<eT>::log_lhood(const Col<eT>& x) const
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (!check_size(x) || (n_gaus == 0)), "mog_diag::log_lhood_single_gaus(): incompatible dimensions" );
  
  return internal_log_lhood(x.mem);
  }



template<typename eT>
inline eT mog_diag<eT>::avg_log_lhood(const field< Col<eT> >& X) const
  {
  arma_extra_debug_sigprint();
  
  arma_debug_check( (!check_size(X) || (n_gaus == 0)), "mog_diag::avg_log_lhood(): incompatible dimensions" );
  
  eT log_acc = eT(0);
  
  for(u32 i=0; i<X.n_elem; ++i)
    {
    log_acc += internal_log_lhood(X[i].mem);
    }
  
  return ( (X.n_elem == 0) ? eT(0) : log_acc / eT(X.n_elem) );
  
  }



template<typename eT>
inline
void
mog_diag<eT>::adapt(bool& out_matchflg, eT& out_log_lhood, u32& out_maxval_idx , const Col<eT>& x, const eT threshold, const eT alpha, const eT rho_fixed)
  {
  arma_extra_debug_sigprint();

  out_matchflg = false;
  out_log_lhood = eT(0);

  if(n_gaus < 1)  return;

  Col<eT> lhoods(n_gaus);

  for(u32 i=0; i<n_gaus; ++i)
    {
    //lhoods(i) = log_lhood_single(x, i);
    lhoods(i) = log_weights(i) + log_lhood_single(x, i);
    }

  eT   max_val = lhoods(0);
  eT   min_val = lhoods(0);

  u32  max_val_id = 0;
  u32  min_val_id = 0;

  for(u32 i=0; i<n_gaus; ++i)
    {
	const eT tmp_val = lhoods(i);

	if(tmp_val > max_val)
      {
	  max_val    = tmp_val;
	  max_val_id = i;
      }

	if(tmp_val < min_val)
	  {
      min_val    = tmp_val;
	  min_val_id = i;
	  }
    }

  out_log_lhood = max_val;
//  cout << "max out_log_lhood" << out_log_lhood << endl;
  if(isnan(out_log_lhood) == 1)
  {
	  cout << lhoods << endl;
	  cout << log_lhood_single(x, 0) << endl;
	  means.print("means = ");
  }
  out_maxval_idx = max_val_id;
  if(max_val >= threshold)
    {
	// adapt the gaussian indicated by max_val_id;

    field< Col<eT> >& out_means   = rw(means);
	field< Col<eT> >& out_dcovs   = rw(dcovs);
	Col<eT>&          out_weights = rw(weights);

	out_means(max_val_id) = ((eT(1) - rho_fixed) * out_means(max_val_id)) + (rho_fixed * x);

	const Col<eT> tmp_vec = x - out_means(max_val_id);
	const Mat<eT> tmp_cov = (tmp_vec) * trans(tmp_vec);

	out_dcovs(max_val_id) = ((eT(1) - (rho_fixed)) * out_dcovs(max_val_id)) + ( (rho_fixed) * tmp_cov.diag() );

	for(u32 i=0; i<n_gaus; ++i)
	  {
      const eT M = (i == max_val_id) ? eT(1) : eT(0);

	  out_weights(i) = ((eT(1) - alpha) * out_weights(i)) + (alpha * M);
	  }


	// init_constants will renormalise the weights
	init_constants();
	out_matchflg = true;
    }
  else
    {
//    // replace the gaussian indicated by min_val_id
//
//    field< Col<eT> >& out_means   = rw(means);
//	field< Col<eT> >& out_dcovs   = rw(dcovs);
//	Col<eT>&          out_weights = rw(weights);
//
//	out_means(min_val_id)   = x;
//	out_dcovs(min_val_id)   = 100.0 * ones< Col<eT> >(n_dim);
//	out_weights(min_val_id) = 0.05;
//
//	// init_constants will renormalise the weights
//	init_constants();

    }

  }


template<typename eT>
inline
void
mog_diag<eT>::init(const mog_diag<eT>& x)
  {
  arma_extra_debug_sigprint();
  
  mog_diag<eT>& t = *this;
  
  if(&t != &x)
    {
    rw(t.means)   = x.means;
    rw(t.dcovs)   = x.dcovs;
    rw(t.weights) = x.weights;
    
    init_constants();
    }
  }



template<typename eT>
inline
void
mog_diag<eT>::init(const u32 in_n_gaus, const u32 in_n_dim)
  {
  arma_extra_debug_sigprint();
  
  rw(means).set_size(in_n_gaus);
  rw(dcovs).set_size(in_n_gaus);
  rw(weights).set_size(in_n_gaus);
  
  for(u32 i=0; i<in_n_gaus; ++i)
    {
    rw(means(i)).zeros(in_n_dim);
    rw(dcovs(i)) = ones<Col<eT> >(in_n_dim);
    rw(weights(i)) = eT(1) / eT(in_n_gaus);
    }
  
  init_constants();
  }



template<typename eT>
template<typename eT2>
inline
void
mog_diag<eT>::init
  (
  const field< Col<eT2> >& in_means,
  const field< Col<eT2> >& in_dcovs,
  const Col<eT2>& in_weights
  )
  {
  arma_extra_debug_sigprint();
  
  field< Col<eT> >& out_means   = rw(means);
  field< Col<eT> >& out_dcovs   = rw(dcovs);
  Col<eT>&          out_weights = rw(weights);
  
  out_means.set_size  (in_means.n_elem);
  out_dcovs.set_size  (in_dcovs.n_elem);
  out_weights.set_size(in_weights.n_elem);
  
  for(u32 i=0; i<in_means.n_elem; ++i)
    {
    out_means(i) = conv_to< Col<eT> >::from(in_means(i));
    }
  
  for(u32 i=0; i<in_dcovs.n_elem; ++i)
    {
    out_dcovs(i) = conv_to< Col<eT> >::from(in_dcovs(i));
    }
  
  out_weights = conv_to< Col<eT> >::from(in_weights);
  
  init_constants();
  }



template<typename eT>
inline
void
mog_diag<eT>::init_constants()
  {
  arma_extra_debug_sigprint();
  
  //
  // check consistency
  
  const bool same_size = (means.n_elem == dcovs.n_elem) && (means.n_elem == weights.n_elem);
  arma_check( !same_size, "mog_diag::init_constants(): detected inconsistency in model parameters" );
  
  rw(n_gaus) = means.n_elem;
  rw(n_dim)  = (means.n_elem > 0) ? means(0).n_elem : 0;
  
  bool same_dim = true;
  for(u32 i=0; i<n_gaus; ++i)
    {
    if( (means(i).n_elem != n_dim) || (dcovs(i).n_elem != n_dim) )
      {
      same_dim = false;
      break;
      }
    }
    
  arma_check( !same_dim, "mog_diag::init_constants(): detected inconsistency in model parameters" );
  
  
  //
  // initialise constants
  
  const eT tmp = (eT(n_dim)/eT(2)) * std::log(eT(2) * Math<eT>::pi() );
  
  log_det_etc.set_size(n_gaus);
  for(u32 i=0; i<n_gaus; ++i)
    {
    // log_det_etc(i) = eT(-1) * ( tmp + eT(0.5) * std::log( det( diagmat(dcovs(i)) ) ) );

    eT tmp_det = det( diagmat(dcovs(i)) );
    tmp_det = (tmp_det == eT(0)) ? Math<eT>::eps() : tmp_det;

    log_det_etc(i) = eT(-1) * ( tmp + eT(0.5) * std::log( tmp_det ) );
    }
  
  rw(weights) /= accu(weights);
  log_weights = log(weights);
  
  if(n_gaus > 0)
    {
    log_limit = std::log( std::numeric_limits<eT>::max() * max(weights) );
    }
  }



template<typename eT>
inline
void
mog_diag<eT>::hist_norm(Col<eT>& out, const Col<eT>& x, const bool use_weights) const
  {
  arma_extra_debug_sigprint();
  
  out.set_size(n_gaus);
  eT* out_mem = out.memptr();
  
  bool overflow_danger = false;
  
  const eT* x_mem           = x.mem;
  const eT* log_weights_mem = log_weights.mem;
  
  
  eT lhood_sum = eT(0);
  eT log_lhood_sum;
  
  u32 start_log_id;
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    const eT log_lhood = 
      (use_weights)
      ? internal_log_lhood_single(x_mem, gaus_id) + log_weights_mem[gaus_id]
      : internal_log_lhood_single(x_mem, gaus_id);
      
    
    if(overflow_danger == false)
      {
      if(log_lhood <= log_limit)
        {
        const eT lhood = std::exp(log_lhood);
        
        out_mem[gaus_id]  = lhood;
        lhood_sum        += lhood;
        }
      else
        {
        overflow_danger = true;
        
        start_log_id     = gaus_id;        
        out_mem[gaus_id] = log_lhood;
        log_lhood_sum    = (gaus_id > 0) ? log_add( std::log(lhood_sum), log_lhood) : log_lhood;
        }
      }
    else
      {
      out_mem[gaus_id] = log_lhood;
      log_lhood_sum    = log_add(log_lhood_sum, log_lhood);
      }
    
    }
  
  if(overflow_danger == false)
    {
    for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
      {
      out_mem[gaus_id] /= lhood_sum;
      }
    }
  else
    {
    for(u32 gaus_id=0; gaus_id<start_log_id; ++gaus_id)
      {
      out_mem[gaus_id] = std::exp( std::log(out_mem[gaus_id]) - log_lhood_sum );
      }
    
    for(u32 gaus_id=start_log_id; gaus_id<n_gaus; ++gaus_id)
      {
      out_mem[gaus_id] = std::exp( out_mem[gaus_id] - log_lhood_sum );
      }
    }
  }



template<typename eT>
inline
bool
mog_diag<eT>::check_size(const Col<eT>& x) const
  {
  arma_extra_debug_sigprint();
  
  return (x.n_elem == n_dim);
  }



template<typename eT>
inline
bool
mog_diag<eT>::check_size(const field< Col<eT> >& X) const
  {
  arma_extra_debug_sigprint();
  
  for(u32 i=0; i<X.n_elem; ++i)
    {
    if( X(i).n_elem != n_dim )
      {
      return false;
      }
    }
  
  return ( (X.n_elem == 0) ? false : true );
  }



template<typename eT>
arma_inline
eT
mog_diag<eT>::internal_log_lhood_single(const eT* x_mem, const u32 gaus_id) const
  {
  arma_extra_debug_sigprint();
  
  const eT* mean_mem = means[gaus_id].mem;
  const eT* cov_mem  = dcovs[gaus_id].mem;
  
  eT val = eT(0);
  
  for(u32 i=0; i<n_dim; ++i)
    {
    const eT tmp = x_mem[i] - mean_mem[i];
    val += (tmp*tmp) / cov_mem[i];
    }
  
  return log_det_etc.mem[gaus_id] - eT(0.5)*val;
  }



template<typename eT>
inline
eT
mog_diag<eT>::internal_log_lhood(const eT* x_mem) const
  {
  arma_extra_debug_sigprint();
  
  bool overflow_danger = false;
  
  eT exp_sum = 0.0;
  eT log_sum;
  
  const eT* log_weights_mem = log_weights.mem;
  
  for(u32 gaus_id=0; gaus_id<n_gaus; ++gaus_id)
    {
    const eT partial_result = log_weights_mem[gaus_id] + internal_log_lhood_single(x_mem, gaus_id);
    
    if(overflow_danger == false)
      {
      if( partial_result < log_limit )
        {
        exp_sum += std::exp( partial_result );
        }
      else
        {
        overflow_danger = true;
        log_sum = (gaus_id > 0) ? log_add( std::log(exp_sum), partial_result ) : partial_result;
        }
      }
    else
      {
      log_sum = log_add(log_sum, partial_result);
      }
    
    }
  
  
  if(overflow_danger == false)
    {
    return( std::log(exp_sum) );
    }
  else
    {
    return(log_sum);
    }
  
  }



template<typename eT>
inline
void
mog_diag<eT>::train_kmeans
  (
  const field< Col<eT> >& in_X,
  const u32  in_n_gaus,
  const u32  in_n_iter,
  const eT   in_trust,
  const bool in_normalise,
  const bool in_verbose
  )
  {
  arma_debug_check( (in_X.n_elem < 2), "kmeans(): X must have at least two elements" );
  arma_debug_check( (in_n_gaus == 0),  "kmeans(): n_gaus must be non-zero" );
  arma_debug_check( (in_n_iter == 0),  "kmeans(): n_iter must be non-zero" );
  arma_debug_check( ((in_trust < eT(0)) || (in_trust > eT(1))), "kmeans(): trust must be in [0,1]" );
  
  const u32 n_dim = in_X(0).n_elem;
  
  bool same_dim = true;
  for(u32 i=0; i<in_X.n_elem; ++i)
    {
    if(in_X(i).n_elem != n_dim)
       {
       same_dim = false;
       break;
       }
    }

  arma_debug_check( (same_dim == false), "kmeans(): X has vectors with inconsistent dimensionality" );


  bool all_finite = true;
  for(u32 i=0; i<in_X.n_elem; ++i)
    {
    if(in_X(i).is_finite() == false)
       {
       all_finite = false;
       break;
       }
    }
  
  arma_debug_check( (all_finite == false), "kmeans(): X has vectors with non-finite elements" );
  
  mog_diag_kmeans<eT> trainer(in_X, in_n_gaus, in_n_iter, in_normalise, in_verbose);
  trainer.run(*this, in_trust);
  }



template<typename eT>
inline
void
mog_diag<eT>::train_em_ml
  (
  const field< Col<eT> >& in_X,
  const u32  in_n_iter,
  const eT   in_var_floor,
  const eT   in_weight_floor,
  const bool in_verbose
  )
  {
  arma_debug_check( (in_n_iter == 0), "kmeans(): n_iter must be non-zero" );
  
  const u32 n_dim = in_X(0).n_elem;
  
  bool same_dim = true;
  for(u32 i=0; i<in_X.n_elem; ++i)
    {
    if(in_X(i).n_elem != n_dim)
       {
       same_dim = false;
       break;
       }
       
    }

  arma_debug_check( (same_dim == false), "kmeans(): X has vectors with inconsistent dimensionality" );
  
  mog_diag_em_ml<eT> trainer(*this, in_X, in_n_iter, in_var_floor, in_weight_floor, in_verbose);
  trainer.run(*this);
  }



template<typename eT>
inline
void
mog_diag<eT>::compute_loglikelihood(eT& out_log_lhood, const Col<eT>& x)
  {
  arma_extra_debug_sigprint();

  out_log_lhood = eT(0);

  if(n_gaus < 1)  return;

  Col<eT> lhoods(n_gaus);

  for(u32 i=0; i<n_gaus; ++i)
    {
    //lhoods(i) = log_lhood_single(x, i);
    lhoods(i) = log_weights(i) + log_lhood_single(x, i);
    }

  eT   max_val = lhoods(0);
  eT   min_val = lhoods(0);

  u32  max_val_id = 0;
  u32  min_val_id = 0;

  for(u32 i=0; i<n_gaus; ++i)
    {
	const eT tmp_val = lhoods(i);

	if(tmp_val > max_val)
      {
	  max_val    = tmp_val;
	  max_val_id = i;
      }

	if(tmp_val < min_val)
	  {
      min_val    = tmp_val;
	  min_val_id = i;
	  }
    }

  out_log_lhood = max_val;
//  cout << "max out_log_lhood" << out_log_lhood << endl;
  if(isnan(out_log_lhood) == 1)
  {
	  cout << lhoods << endl;
	  cout << log_lhood_single(x, 0) << endl;
	  means.print("means = ");
	  cout << "in compute_loglikelihood function" << endl;
	  exit(-1);
  }
//  out_maxval_idx = max_val_id;

  }
