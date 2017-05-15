// Copyright (C) 2009 - 2012 NICTA
//
// Authors:
// - Vikas Reddy (vikas.reddy at ieee dot org)
//
// This file is provided without any warranty of
// fitness for any purpose. You can redistribute
// this file and/or modify it under the terms of
// the GNU General Public License (GPL) as published
// by the Free Software Foundation, either version 3
// of the License or (at your option) any later version.
// (see http://www.opensource.org/licenses for more info)

#ifndef CASCADEDBGSPARAMS_H_
#define CASCADEDBGSPARAMS_H_

#include "inc.hpp"

typedef struct
{

	 double cosinedist_T;

} bgscascade_thresholds;



class CascadedBgsParams
{
public:
	const s32 len;
	const s16 N;
	const s16 sub_mat_elem;
	const s16 ovlstep;
	const u32 n_gaus;
	const u32 n_gaus_final;
	const u32 n_iter;
	const double trust;
	const bool normalise;
	const bool print_progress;
	const double rho;
	const double alpha;
	const double cosinedist_T;
	const double likelihood_ratio_T;
	const double tmprl_cosinedist_T;
	const u32 fv_type;

	CascadedBgsParams(const s32 in_len,const s16 in_N, const s16 in_ovlstep, const bgscascade_thresholds  &T_vals);

	virtual ~CascadedBgsParams();
};


#endif /* CASCADEDBGSPARAMS_H_ */
