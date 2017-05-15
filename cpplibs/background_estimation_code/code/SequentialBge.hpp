// Copyright (C) 2009 - 2012 NICTA
// 
// Authors:
// - Vikas Reddy (vikas.reddy at ieee dot org)
//               (rvikas2333 at gmail dot com)
// 
// Please cite the following journal article when using this source code:
// 
//  V. Reddy, C. Sanderson,  B.C. Lovell.
//  A Low-Complexity Algorithm for Static Background Estimation from Cluttered Image Sequences in Surveillance Contexts.
//  Eurasip Journal on Image and Video Processing, 2011.
//   
//  http://dx.doi.org/10.1155/2011/164956
// 
// ---
// 
// This file is provided without any warranty of
// fitness for any purpose. You can redistribute
// this file and/or modify it under the terms of
// the GNU General Public License (GPL) as published
// by the Free Software Foundation, either version 3
// of the License or (at your option) any later version.
// (see http://www.opensource.org/licenses for more info)

#ifndef SEQUENTIALBGE_H_
#define SEQUENTIALBGE_H_

#include "inc.hpp"
#include "SequentialBgeParams.hpp"

typedef struct
{
	u32 wt;
	vec rep_blocks;
} blk_stats;




class bg_est
{

private:
	static const int N_MASKS;

	int frameNumber, mod_B_frameNumber;

	bge_params *bge_params_obj;
	s16 xmb, ymb;
	u32 no_of_blks;

	field<vector<blk_stats> > block_info;
	umat rep_blk_cnt;
	field<vec> bg_frame_matrix;
	field<vec> frame_fv;
	mat dct_mtx;
	vec DCT_coeffs;
	imat mask;
	imat best_blk_id;
	mat used_blks_pct;

	u16 width, height, channels;
	imat dst_offset, src_offset, curr_blk_offset;
	imat dst_offset_1N, src_offset_1N, curr_blk_offset_1N;

public:

	bg_est(const cv::Mat &frame, const bge_params& cascadedBgsParams);

	virtual ~bg_est();

	void store_first_frame(const cv::Mat &frame);
	void online_clustering(const cv::Mat &frame);
	inline double measure_correlation_coefficient(const vec &a, const vec &b);
	inline double measure_L1_distance(const vec &a, const vec &b);
	void partial_bg_construct(const cv::Mat &frame);
	void bg_estimation_at_blocks();
	int find_atleast_3_filled_neighbours(const int &x, const int &y);
	s32 unfilled_block_estimate_from_3_neighbours(mat &out_cost,
			const int &interpval, const int & x, const int & y);
	int
	find_atleast_1_filled_neighbour(const int &x, const int &y);
	s32 unfilled_block_estimate_from_1_neighbour(mat &out_cost,
			const int &interpval, const int & x, const int &y);

	void show_bg_frame(void);

	void create_dct_table(int N);
	void view_candidate(const vector<blk_stats> &block_info, int blk_n);
	void view_local_region(const mat &local_region);
	void apply_ICM();
	void mark_8_connected_neighbours(imat &matrix);
	virtual void detectRaw(const cv::Mat &frame);
	cv::Mat estimated_bg_frame;

};

#endif /* SEQUENTIALBGE_H_ */
