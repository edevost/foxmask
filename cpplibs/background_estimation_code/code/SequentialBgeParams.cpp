// Copyright (C) 2009 - 2012 NICTA
//
// Authors:
// - Vikas Reddy (vikas.reddy at ieee dot org)
//               (rvikas2333 at gmail dot com)
//
// This file is provided without any warranty of
// fitness for any purpose. You can redistribute
// this file and/or modify it under the terms of
// the GNU General Public License (GPL) as published
// by the Free Software Foundation, either version 3
// of the License or (at your option) any later version.
// (see http://www.opensource.org/licenses for more info)

#include "SequentialBgeParams.hpp"

bge_params::bge_params(const s32 &in_len, const bg_est_param_values &in_Tvals) :
	len(in_len), N(in_Tvals.N), ovlstep(in_Tvals.Ovlstep), pixel_diff(
			in_Tvals.Pixel_Diff), corr_coef(in_Tvals.Corr_Coef), eta(
			in_Tvals.Eta), min_frames(in_Tvals.MinFrames), iterations(
			in_Tvals.Iterations)
{
}

bge_params::~bge_params()
{
	// TODO Auto-generated destructor stub
}
