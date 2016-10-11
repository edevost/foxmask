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

#include "CascadedBgsParams_proto.hpp"

CascadedBgsParams::CascadedBgsParams(const s32 in_len, const s16 in_N,
		const s16 in_ovlstep,
		const bgscascade_thresholds &in_T_vals)
	:

	len(in_len),
	N(in_N),
	sub_mat_elem(4),
	ovlstep(in_ovlstep),
	n_gaus(2),
	n_gaus_final(1),
	n_iter(5),
	trust(0.9),
	normalise(true),
	print_progress(false),
	rho(0.02),
	alpha(0.05),
	cosinedist_T(in_T_vals.cosinedist_T),
	tmprl_cosinedist_T(in_T_vals.cosinedist_T * 0.5),
	likelihood_ratio_T(0.9),
	fv_type(0)
{
}

CascadedBgsParams::~CascadedBgsParams()
{
	// TODO Auto-generated destructor stub
}
