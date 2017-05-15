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


#if !defined(MOG_DIAG_HPP)

  #include <omp.h>

  #include "mog_diag_base_proto.hpp"
  #include "mog_diag_kmeans_parallel_proto.hpp"
  #include "mog_diag_em_ml_parallel_proto.hpp"
  
  #include "mog_diag_base_meat.hpp"
  #include "mog_diag_kmeans_parallel_meat.hpp"
  #include "mog_diag_em_ml_parallel_meat.hpp"
  
  #define MOG_DIAG_HPP

#endif
