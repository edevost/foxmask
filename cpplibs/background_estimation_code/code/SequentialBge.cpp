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

#include "inc.hpp"
#include "SequentialBge.hpp"
//#include <cxtypes.h>
const int bg_est::N_MASKS = 1;

bg_est::bg_est(const cv::Mat &frame, const bge_params& in_bge_params)
{
	this->bge_params_obj = new bge_params(in_bge_params);

	frameNumber = 0;
	mod_B_frameNumber = 0;

	width = frame.cols;
	height = frame.rows;
	xmb = (width - this->bge_params_obj->N) / this->bge_params_obj->ovlstep;
	ymb = (height - this->bge_params_obj->N) / this->bge_params_obj->ovlstep;

	no_of_blks = (1 + ymb) * (1 + xmb);
	rep_blk_cnt.set_size((1 + ymb), (1 + xmb));
	block_info.set_size((1 + ymb), (1 + xmb));
	bg_frame_matrix.set_size((1 + ymb), (1 + xmb));
	channels = frame.channels();
	mask.set_size((1 + ymb), (1 + xmb));
	best_blk_id.set_size((1 + ymb), (1 + xmb));
	used_blks_pct.set_size((1 + ymb), (1 + xmb));

	used_blks_pct.fill(0);
	mask.fill(1);
	best_blk_id.fill(-1);

	for (int y = 0; y < (1 + ymb); ++y)
	{
		for (int x = 0; x < (1 + xmb); ++x)
		{

			bg_frame_matrix(y, x).set_size(this->bge_params_obj->N
					* this->bge_params_obj->N * channels);
			bg_frame_matrix(y, x).fill(0);
		}

	}

	dct_mtx = ones<mat> (this->bge_params_obj->N * 2, this->bge_params_obj->N
			* 2);

	cv::Size imsize(width, height);

	if (channels == 3)
	{
		estimated_bg_frame = cv::Mat(imsize, CV_8UC3, cv::Scalar(0, 0, 0));
	}
	else
	{

		estimated_bg_frame = cv::Mat::zeros(imsize, CV_8UC1);
	}

	cv::imshow("mainWin2", estimated_bg_frame);
	cv::waitKey(1000);

	DCT_coeffs.set_size(this->bge_params_obj->N * this->bge_params_obj->N * 4);
	create_dct_table(this->bge_params_obj->N * 2);

	//INITIALISE THE DCT MATRIX 8x8
	for (int i = 0; i < this->bge_params_obj->N * 2; i++)
	{
		for (int j = 0; j < this->bge_params_obj->N * 2; j++)
			dct_mtx(i, j) = DCT_coeffs[i * this->bge_params_obj->N * 2 + j];
	}

	//	Three neighbours offsets (default case)
	dst_offset.set_size(4, 6);
	src_offset.set_size(4, 6);
	curr_blk_offset.set_size(4, 2);

	//	One neighbour offsets
	src_offset_1N.set_size(4, 2);
	dst_offset_1N.set_size(4, 2);
	curr_blk_offset_1N.set_size(4, 2);

	//	 0  |  1
	//	____|_____
	//	 2  |  3
	//	    |

	//  0,1,2 & 3 are the possible positions of the unknown block. Each row in the
	//	table indicate the (x,y) locations of the destination buffer for 3 filled
	//	background neighbours in the clockwise direction.


	dst_offset
			= "\
	0 			 	16			16 				16  		16 			 	0;\
	0 		  	 	0 			16 				16  		16 			 	0;\
	0 		  	 	0 			0 			  	16  		16 			 	0;\
	0 		  	    0			0 			  	16  		16 				16";

	//	respective source indices of the filled neighbours in the same order.

	src_offset
			= " \
0		 1	    1		1 		1 		 0;\
0 		-1 		1 		0 	 	1 	    -1;\
-1 		-1 	   -1 		0 		0 		-1;\
-1 		 0 	   -1 		1 		0 		 1";

	//position of the unknown block, w.r.t to figure above,
	//which gets filled by candidates, one at a time.
	curr_blk_offset = " \
	 0		 0;\
	 0 		 16;\
	 16		 16;\
	 16 	 0";

	//Same table but with 1 neighbour at a time instead of 3
	dst_offset_1N = "\
	 0   0; \
	 0	 16; \
     16   0; \
     0   0";

	src_offset_1N = "\
	-1   0; \
	 0	 1; \
     1   0; \
     0  -1";

	curr_blk_offset_1N
			= " \
		 16		 0;\
		 0 		 0;\
		 0		 0;\
		 0 	 	 16";

	for (u16 i = 0; i < dst_offset.n_rows; ++i)
	{
		for (u16 j = 0; j < dst_offset.n_cols; ++j)
		{
			if (dst_offset(i, j) == 16)
				dst_offset(i, j) = this->bge_params_obj->N;
		}
	}

	for (u16 i = 0; i < curr_blk_offset.n_rows; ++i)
	{
		for (u16 j = 0; j < curr_blk_offset.n_cols; ++j)
		{
			if (curr_blk_offset(i, j) == 16)
				curr_blk_offset(i, j) = this->bge_params_obj->N;
		}
	}

	for (u16 i = 0; i < dst_offset_1N.n_rows; ++i)
	{
		for (u16 j = 0; j < dst_offset_1N.n_cols; ++j)
		{
			if (dst_offset_1N(i, j) == 16)
				dst_offset_1N(i, j) = this->bge_params_obj->N;
		}
	}

	for (u16 i = 0; i < curr_blk_offset_1N.n_rows; ++i)
	{
		for (u16 j = 0; j < curr_blk_offset_1N.n_cols; ++j)
		{
			if (curr_blk_offset_1N(i, j) == 16)
				curr_blk_offset_1N(i, j) = this->bge_params_obj->N;
		}
	}

}

bg_est::~bg_est()
{

}

void bg_est::detectRaw(const cv::Mat &frame)
{

	if (frameNumber == 0)
	{
		store_first_frame(frame);
		frameNumber++;
	}
	else
	{

		online_clustering(frame);
		frameNumber++;
		if (frameNumber == bge_params_obj->len)
		{
			partial_bg_construct(frame);
			bg_estimation_at_blocks();

			apply_ICM();

		}

	}

}

void bg_est::partial_bg_construct(const cv::Mat &frame)
{

	u16 N = bge_params_obj->N;
	u16 ovlstep = bge_params_obj->ovlstep;
	int x = 0, y = 0;
	int channels = frame.channels();

	for (int i = 0; i <= height - N; i = i + ovlstep)
	{

		for (int j = 0; j <= width - N; j = j + ovlstep)
		{

			/******************************************************************************/
			if (rep_blk_cnt(y, x) == 1)
			{
				cube res_mtx(N, N, channels);
				bg_frame_matrix(y, x) = block_info(y, x)[0].rep_blocks;
				u16 ofset = N * N;

				for (int ch = 0; ch < channels; ch++)
				{

					res_mtx.slice(ch) = reshape(bg_frame_matrix(y, x).rows(ch
							* ofset, (ch + 1) * ofset - 1), N, N);
				}

				mask(y, x) = 0;
				best_blk_id(y, x) = 0;

				for (int k = i, p = 0; k < (i + N); k++, p++)
				{
					for (int l = j, q = 0; l < (j + N); l++, q++)
					{

						if (channels == 3)
						{
							cv::Vec3b pix;

							for (int ch = 0; ch < channels; ch++)
							{
								pix[ch] = u8(res_mtx.slice(ch)(p, q));
							}

							estimated_bg_frame.at<cv::Vec3b> (k, l) = pix;
						}
						else
						{
							estimated_bg_frame.at<u8> (k, l) = u8(
									res_mtx.slice(0)(p, q));

						}

					}

				}
			}

			for (int id = 0; id < rep_blk_cnt(y, x); ++id)
			{
				used_blks_pct(y, x) += block_info(y, x)[id].wt;
			}

			x++;
		}
		x = 0;
		y++;
	}
	// CHECK IF THERE IS NO SEED IN THE PARTIALLY CONSTRUCTED BACKGROUND AND CREATE ONE IF NECESSARY
	if ((u32) sum(sum(mask)) == mask.n_elem)
	{

		cube res_mtx(N, N, channels);
		bg_frame_matrix(0, 0) = block_info(0, 0)[0].rep_blocks;
		u16 ofset = N * N;

		for (int ch = 0; ch < channels; ch++)
		{

			res_mtx.slice(ch) = reshape(bg_frame_matrix(0, 0).rows(ch * ofset,
					(ch + 1) * ofset - 1), N, N);
		}
		mask(0, 0) = 0;

		for (int k = 0, p = 0; k < (N); k++, p++)
		{
			for (int l = 0, q = 0; l < (N); l++, q++)
			{

				if (channels == 3)
				{
					cv::Vec3b pix;

					for (int ch = 0; ch < channels; ch++)
					{
						pix[ch] = u8(res_mtx.slice(ch)(p, q));
					}

					estimated_bg_frame.at<cv::Vec3b> (k, l) = pix;
				}
				else
				{
					estimated_bg_frame.at<u8> (k, l) = u8(
							res_mtx.slice(0)(p, q));

				}

			}

		}
	}
	used_blks_pct /= bge_params_obj->len;
	//	cout << used_blks_pct << endl;
	//	cout << "----------" << endl;
	cv::imshow("mainWin2", estimated_bg_frame);
	cvWaitKey(10);

}

void bg_est::show_bg_frame(void)
{

	u16 N = bge_params_obj->N;
	u16 ovlstep = bge_params_obj->ovlstep;
	int x = 0, y = 0;

	for (int i = 0; i <= height - N; i = i + ovlstep)
	{
		for (int j = 0; j <= width - N; j = j + ovlstep)
		{

			/******************************************************************************/
			if (mask(y, x) == 0)
			{
				cube res_mtx(N, N, channels);
				u16 ofset = N * N;

				for (int ch = 0; ch < channels; ch++)
				{

					res_mtx.slice(ch) = reshape(bg_frame_matrix(y, x).rows(ch
							* ofset, (ch + 1) * ofset - 1), N, N);
				}


				for (int k = i, p = 0; k < (i + N); k++, p++)
				{
					for (int l = j, q = 0; l < (j + N); l++, q++)
					{

						if (channels == 3)
						{
							cv::Vec3b pix;
							for (int ch = 0; ch < channels; ch++)
							{
								pix[ch] = u8(res_mtx.slice(ch)(p, q));
							}
							estimated_bg_frame.at<cv::Vec3b> (k, l) = pix;
						}
						else
						{

							estimated_bg_frame.at<u8> (k, l) = u8(
									res_mtx.slice(0)(p, q));

						}

					}

				}
			}

			x++;
		}
		x = 0;
		y++;
	}

	cv::imshow("mainWin2", estimated_bg_frame);
	cvWaitKey(2);

}

void bg_est::store_first_frame(const cv::Mat &frame)
{

	s16 N = bge_params_obj->N;
	s16 ovlstep = bge_params_obj->ovlstep;

	s16 x, y, mbwt;
	u32 lcnt;

	blk_stats blk_info_init;

	x = 0;
	y = 0;
	lcnt = 0;
	mbwt = width / N;

	cube img_frame = zeros<cube> (height, width, channels);


	/*reading the frame into armadillo matrix*/
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			if (channels == 3)
			{

				cv::Vec3b pix = frame.at<cv::Vec3b> (i, j);
				for (int ch = 0; ch < channels; ch++)
				{
					img_frame.slice(ch)(i, j) = (double) pix[ch];
				}
			}
			else
			{
				img_frame.slice(0)(i, j) = frame.at<u8> (i, j);
			}

		}

	}

	blk_info_init.wt = 1;
	for (int i = 0; i <= height - N; i = i + ovlstep)
	{
		for (int j = 0; j <= width - N; j = j + ovlstep)
		{

			u16 ofset = N * N;
			blk_info_init.rep_blocks.set_size(ofset * channels);
			for (int ch = 0; ch < channels; ch++)
			{
				mat tmp = img_frame.slice(ch).submat(i, j, (i + N - 1), (j + N
						- 1));
				blk_info_init.rep_blocks.rows(ch * ofset, ch * ofset + ofset
						- 1) = reshape(tmp, N * N, 1);
			}

			rep_blk_cnt(y, x) = 1;
			block_info(y, x).assign(1, blk_info_init);
			x++;
		}
		y++;
		x = 0;
	}

}

void bg_est::online_clustering(const cv::Mat &frame)
{

	s16 N = bge_params_obj->N;
	s16 ovlstep = bge_params_obj->ovlstep;
	s16 x, y;
	x = 0;
	y = 0;
	mat tmp(N, N);
	vec cur_vec(N * N, 1);
	double dist, rr, corr_coef_threshold;
	u32 wt, blkcnt, L1_threshold;
	vec sf_cur_vec(N * N), sf_rep_vec(N * N);
	bool updateflg, addblkflg;

	cube img_frame = zeros<cube> (height, width, channels);

	blk_stats tmp_blk_val;
	//-----------------------------------------------------------
	/*reading the frame into armadillo matrix*/

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			if (channels == 3)
			{

				cv::Vec3b pix = frame.at<cv::Vec3b> (i, j);
				for (int ch = 0; ch < channels; ch++)
				{
					img_frame.slice(ch)(i, j) = (double) pix[ch];
				}
			}
			else
			{
				img_frame.slice(0)(i, j) = frame.at<u8> (i, j);
			}

		}

	}
	//-----------------------------------------------------------
	L1_threshold = (N * N * bge_params_obj->pixel_diff * channels);
	corr_coef_threshold = bge_params_obj->corr_coef;

	for (int i = 0; i <= height - N; i = i + ovlstep)
	{
		for (int j = 0; j <= width - N; j = j + ovlstep)
		{

			u16 ofset = N * N;
			cur_vec.set_size(ofset * channels);
			for (int ch = 0; ch < channels; ch++)
			{
				mat tmp = img_frame.slice(ch).submat(i, j, (i + N - 1), (j + N
						- 1));
				cur_vec.rows(ch * ofset, (ch + 1) * ofset - 1) = reshape(tmp, N
						* N, 1);
			}

			addblkflg = true;
			for (u16 k = 0; k < rep_blk_cnt(y, x); ++k)
			{

				updateflg = false;
				dist = measure_L1_distance(block_info(y, x)[k].rep_blocks,
						cur_vec);

				if (dist < L1_threshold)
				{

					rr = measure_correlation_coefficient(
							block_info(y, x)[k].rep_blocks, cur_vec);

					if (rr > corr_coef_threshold)
					{
						updateflg = true;
					}

					if (updateflg == true)
					{

						addblkflg = false;
						wt = block_info(y, x)[k].wt; //TODO: change the value
						block_info(y, x)[k].rep_blocks
								= block_info(y, x)[k].rep_blocks * wt + cur_vec;
						wt++;
						block_info(y, x)[k].rep_blocks /= wt;
						block_info(y, x)[k].wt = wt;
						break;

					}

				}

			}

			if (addblkflg == true)
			{
				tmp_blk_val.rep_blocks = cur_vec;
				tmp_blk_val.wt = 1;

				blkcnt = rep_blk_cnt(y, x);

				if (block_info(y, x)[blkcnt - 1].wt <= 2)
				{
					block_info(y, x).pop_back();
				}
				else
				{
					blkcnt++;
				}

				block_info(y, x).push_back(tmp_blk_val);
				rep_blk_cnt(y, x) = blkcnt;

			}
			x++;
		}
		y++;
		x = 0;
	}

}

void bg_est::create_dct_table(int N)
{

	int k = 0;
	double scale_fac_i;

	for (double m = 0; m < N; m++)
	{

		for (double n = 0; n < N; n++)
		{

			scale_fac_i = (m == 0) ? sqrt(1.0 / double(N)) : sqrt(2.0
					/ double(N));
			DCT_coeffs(k++) = scale_fac_i * std::cos(double((math::pi() * m)
					/ (2 * N) * (2 * n + 1)));
		}
	}

}

inline double bg_est::measure_correlation_coefficient(const vec &a,
		const vec &b)
{

	double rep_mu, cur_mu, ccv, rr;
	double nmr1, nmr2, nmr_res;
	vec sf_cur_vec(bge_params_obj->N * bge_params_obj->N), sf_rep_vec(
			bge_params_obj->N * bge_params_obj->N);

	rep_mu = mean(a);
	cur_mu = mean(b);
	sf_cur_vec = b - cur_mu;
	sf_rep_vec = a - rep_mu;

	nmr1 = sqrt(sum(sum(square(sf_cur_vec))));
	nmr2 = sqrt(sum(sum(square(sf_rep_vec))));
	nmr_res = nmr1 * nmr2;

	if (nmr_res != 0)
	{
		ccv = dot(sf_cur_vec, sf_rep_vec);
		rr = abs(ccv / nmr_res);

	}
	else
	{
		rr = 1;
	}
	return rr;

}

inline double bg_est::measure_L1_distance(const vec &a, const vec &b)
{
	return sum(sum(abs(a - b)));
}

void bg_est::bg_estimation_at_blocks()
{

	int x, y;
	int interpval, fill_cnt;
	s32 best_blk_idx;
	mat cost(1, 1);

	while (sum(sum(mask)) != (s32) 0)
	{
		fill_cnt = 0;
		for (y = 0; y <= ymb; y++)
		{
			for (x = 0; x <= xmb; x++)
			{
				/******************************************************************************/
				if (mask(y, x) == 1)
				{

					interpval = find_atleast_3_filled_neighbours(x, y);
					cost.set_size(1, 1);
					cost(0, 0) = -1;

					best_blk_idx = unfilled_block_estimate_from_3_neighbours(
							cost, interpval, x, y);

					vec costvec = sum(cost, 1);
					uvec best_id = sort_index(costvec, 1);

					if (best_blk_idx < 0)
					{
						continue;
					}


					bg_frame_matrix(y, x)
							= block_info(y, x)[best_id(0)].rep_blocks;
					mask(y, x) = 0;
					best_blk_id(y, x) = best_id(0);
					show_bg_frame();
					fill_cnt++;

				}

			}

		}

		if (0 == fill_cnt)
		{
			fill_cnt = 0;

			for (y = 0; y <= ymb; y++)
			{
				for (x = 0; x <= xmb; x++)
				{

					/******************************************************************************/
					if (mask(y, x) == 1)
					{
						interpval = find_atleast_1_filled_neighbour(x, y);
						cost.set_size(1, 1);
						cost(0, 0) = -1;
						best_blk_idx
								= unfilled_block_estimate_from_1_neighbour(
										cost, interpval, x, y);
						vec costvec = sum(cost, 1);
						uvec best_id = sort_index(costvec, 1);
						if (best_blk_idx < 0)
						{
							continue;
						}
						bg_frame_matrix(y, x)
								= block_info(y, x)[best_id(0)].rep_blocks;
						mask(y, x) = 0;
						best_blk_id(y, x) = best_id(0);
						fill_cnt++;

						show_bg_frame();
						break;
					}

				}
				if (fill_cnt > 0)
				{
					break;
				}
			}

		}

	}
	show_bg_frame();

}
int bg_est::find_atleast_3_filled_neighbours(const int &x, const int &y)
{
	int intrpval = 0;
	int tr, lc, rc, br;
	int top, left, right, bottom;
	(y > 0) ? top = 1 : top = 0;
	(y < ymb) ? bottom = 1 : bottom = 0;

	(x > 0) ? left = 1 : left = 0;
	(x < xmb) ? right = 1 : right = 0;

	(top == 0) ? tr = 1 : tr = 0;
	(bottom == 0) ? br = 1 : br = 0;
	(right == 0) ? rc = 1 : rc = 0;
	(left == 0) ? lc = 1 : lc = 0;

	imat mask_win1 = mask.submat((y - 1 + tr), (x - 1 + lc), (y + 1 - br), (x
			+ 1 - rc));

	imat win_mask = ones<imat> (3, 3);

	if (tr == 1 && br == 0 && rc == 0 && lc == 0)
	{
		win_mask.submat(1, 0, 2, 2) = mask_win1;

	}

	if (tr == 0 && br == 1 && rc == 0 && lc == 0)
	{
		win_mask.submat(0, 0, 1, 2) = mask_win1;

	}

	if (tr == 0 && br == 0 && rc == 1 && lc == 0)
	{
		win_mask.submat(0, 0, 2, 1) = mask_win1;

	}

	if (tr == 0 && br == 0 && rc == 0 && lc == 1)
	{
		win_mask.submat(0, 1, 2, 2) = mask_win1;

	}

	if (tr == 1 && br == 0 && rc == 0 && lc == 1)
	{
		win_mask.submat(1, 1, 2, 2) = mask_win1;

	}

	if (tr == 1 && br == 0 && rc == 1 && lc == 0)
	{
		win_mask.submat(1, 0, 2, 1) = mask_win1;

	}

	if (tr == 0 && br == 1 && rc == 0 && lc == 1)
	{
		win_mask.submat(0, 1, 1, 2) = mask_win1;

	}

	if (tr == 0 && br == 1 && rc == 1 && lc == 0)
	{
		win_mask.submat(0, 0, 1, 1) = mask_win1;

	}

	if (tr == 0 && br == 0 && rc == 0 && lc == 0)
	{
		win_mask = mask_win1;
	}

	int lval = 0, pow2 = 1;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			imat tmp = win_mask.submat(i, j, (i + 1), (j + 1));

			if (sum(sum(tmp)) == 1)
				lval += pow2;
			pow2 *= 2;
		}
	}

	intrpval = lval;
	return intrpval;

}

s32 bg_est::unfilled_block_estimate_from_3_neighbours(mat &out_cost,
		const int &interpval, const int & x, const int & y)
{
	u16 N = bge_params_obj->N, est_n = 0;
	vec blk_arr(4);

	mat F(2 * N, 2 * N), res1(2 * N, 2 * N), res2(2 * N, 2 * N);
	umat W_inv(2 * N, 2 * N), W_512(2 * N, 2 * N);

	switch (interpval)
	{

	case 1:
		//            %             LEFT-TOP ONLY
		blk_arr(0) = 2;
		est_n = 1;
		break;

	case 2:
		//            %             RIGHT-TOP ONLY
		blk_arr(0) = 3;
		est_n = 1;
		break;

	case 3:
		//            %             RIGHT-TOP & LEFT-TOP ONLY
		blk_arr(0) = 3;
		blk_arr(1) = 2;
		est_n = 2;
		break;

	case 4:
		//            %              LEFT-BOTTOM ONLY

		blk_arr(0) = 1;
		est_n = 1;
		break;

	case 5:
		//            %             LEFT-TOP & LEFT-BOTTOM ONLY
		blk_arr(0) = 1;
		blk_arr(1) = 2;
		est_n = 2;
		break;

	case 6:
		//            %           LEFT-BOTTOM & RIGHT-TOP ONLY
		blk_arr(0) = 2;
		blk_arr(1) = 3;
		est_n = 2;
		break;

	case 7:
		//            %             LEFT-TOP, LEFT-BOTTOM & RIGHT-TOP ONLY
		blk_arr(0) = 2;
		blk_arr(1) = 1;
		blk_arr(2) = 3;
		est_n = 3;
		break;

	case 8:
		//            %             RIGHT-BOTTOM ONLY
		blk_arr(0) = 0;
		est_n = 1;
		break;

	case 9:
		//            %				RIGHT-BOTTOM & LEFT-TOP,
		blk_arr(0) = 0;
		blk_arr(1) = 2;
		est_n = 2;
		break;
	case 10:
		//            %             RIGHT-TOP & RIGHT-BOTTOM ONLY
		blk_arr(0) = 3;
		blk_arr(1) = 0;
		est_n = 2;
		break;
	case 11:
		//            %  			RIGHT-TOP, LEFT-TOP & RIGHT-BOTTOM ONLY
		blk_arr(0) = 3;
		blk_arr(1) = 2;
		blk_arr(2) = 0;
		est_n = 3;
		break;
	case 12:
		//            %            RIGHT-BOTTOM & LEFT-BOTTOM ONLY
		blk_arr(0) = 0;
		blk_arr(1) = 1;
		est_n = 2;
		break;

	case 13:
		//						  RIGHT-BOTTOM, LEFT-TOP &  RIGHT-TOP
		blk_arr(0) = 0;
		blk_arr(1) = 2;
		blk_arr(2) = 3;
		est_n = 3;
		break;

	case 14:
		//						  RIGHT-BOTTOM, LEFT-BOTTOM &  RIGHT-TOP
		blk_arr(0) = 0;
		blk_arr(1) = 1;
		blk_arr(2) = 3;
		est_n = 3;
		break;

	case 15:
		//            %             ALL NEIGHBOURS
		blk_arr(0) = 0;
		blk_arr(1) = 1;
		blk_arr(2) = 2;
		blk_arr(3) = 3;
		est_n = 4;
		break;

	default:
		return -1;

	}

	mat local_region = zeros<mat> (2 * N, 2 * N);

	out_cost.set_size(rep_blk_cnt(y, x), est_n);

	//GET THE LIKELIHOODS
	vec lhood = zeros<vec> (rep_blk_cnt(y, x));
	for (u16 i = 0; i < rep_blk_cnt(y, x); ++i)
	{
		lhood(i) = block_info(y, x)[i].wt;
		lhood(i) = min((int) bge_params_obj->min_frames, (int) lhood(i));
	}
	//		NORMALISE LIKLIHOODS
	lhood = lhood / sum(lhood);

	u16 ofset = N * N;

//	view_candidate(block_info(y, x), (rep_blk_cnt(y, x)));

	for (int k = 0; k < est_n; ++k)
	{
		vec prior = zeros<vec> (rep_blk_cnt(y, x));
		for (int ch = 0; ch < channels; ++ch)
		{

			local_region.submat(dst_offset(blk_arr(k), 0), dst_offset(
					blk_arr(k), 1), dst_offset(blk_arr(k), 0) + N - 1,
					dst_offset(blk_arr(k), 1) + N - 1) = reshape(
					bg_frame_matrix(y + src_offset(blk_arr(k), 0), x
							+ src_offset(blk_arr(k), 1)).rows(ch * ofset, (ch
							+ 1) * ofset - 1), N, N);

			local_region.submat(dst_offset(blk_arr(k), 2), dst_offset(
					blk_arr(k), 3), dst_offset(blk_arr(k), 2) + N - 1,
					dst_offset(blk_arr(k), 3) + N - 1) = reshape(
					bg_frame_matrix(y + src_offset(blk_arr(k), 2), x
							+ src_offset(blk_arr(k), 3)).rows(ch * ofset, (ch
							+ 1) * ofset - 1), N, N);

			local_region.submat(dst_offset(blk_arr(k), 4), dst_offset(
					blk_arr(k), 5), dst_offset(blk_arr(k), 4) + N - 1,
					dst_offset(blk_arr(k), 5) + N - 1) = reshape(
					bg_frame_matrix(y + src_offset(blk_arr(k), 4), x
							+ src_offset(blk_arr(k), 5)).rows(ch * ofset, (ch
							+ 1) * ofset - 1), N, N);

			for (u16 i = 0; i < rep_blk_cnt(y, x); ++i)
			{

				local_region.submat(curr_blk_offset(blk_arr(k), 0),
						curr_blk_offset(blk_arr(k), 1), curr_blk_offset(
								blk_arr(k), 0) + N - 1, curr_blk_offset(
								blk_arr(k), 1) + N - 1) = reshape(
						block_info(y, x)[i].rep_blocks.rows(ch * ofset,
								(ch + 1) * ofset - 1), N, N);

				//				view_local_region(local_region);

				res2 = dct_mtx * local_region * trans(dct_mtx);
				res2(0, 0) = 0;
				prior(i) += sum(sum(abs(res2)));

			}

		}

		//		NORMALISE PRIORS
		prior = prior / sum(prior);

		for (u16 i = 0; i < rep_blk_cnt(y, x); ++i)
		{
			prior(i) = exp(-prior(i) * rep_blk_cnt(y, x));
		}
		//		NORMALISE PRIORS AGAIN
		prior = prior / sum(prior);

		out_cost.col(k) = bge_params_obj->eta * log(prior) + log(lhood);

	}
	return 0;

}

int bg_est::find_atleast_1_filled_neighbour(const int &x, const int &y)
{
	int intrpval = -1;
	int tr, lc, rc, br;
	int top, left, right, bottom;

	(y > 0) ? top = 1 : top = 0;
	(y < ymb - 1) ? bottom = 1 : bottom = 0;

	(x > 0) ? left = 1 : left = 0;
	(x < xmb - 1) ? right = 1 : right = 0;

	(top == 0) ? tr = 1 : tr = 0;
	(bottom == 0) ? br = 1 : br = 0;
	(right == 0) ? rc = 1 : rc = 0;
	(left == 0) ? lc = 1 : lc = 0;

	imat mask_win1 = mask.submat((y - 1 + tr), (x - 1 + lc), (y + 1 - br), (x
			+ 1 - rc));

	imat win_mask = ones<imat> (3, 3);

	if (tr == 1 && br == 0 && rc == 0 && lc == 0)
	{
		win_mask.submat(1, 0, 2, 2) = mask_win1;

	}

	if (tr == 0 && br == 1 && rc == 0 && lc == 0)
	{
		win_mask.submat(0, 0, 1, 2) = mask_win1;

	}

	if (tr == 0 && br == 0 && rc == 1 && lc == 0)
	{
		win_mask.submat(0, 0, 2, 1) = mask_win1;

	}

	if (tr == 0 && br == 0 && rc == 0 && lc == 1)
	{
		win_mask.submat(0, 1, 2, 2) = mask_win1;

	}

	if (tr == 1 && br == 0 && rc == 0 && lc == 1)
	{
		win_mask.submat(1, 1, 2, 2) = mask_win1;

	}

	if (tr == 1 && br == 0 && rc == 1 && lc == 0)
	{
		win_mask.submat(1, 0, 2, 1) = mask_win1;

	}

	if (tr == 0 && br == 1 && rc == 0 && lc == 1)
	{
		win_mask.submat(0, 1, 1, 2) = mask_win1;

	}

	if (tr == 0 && br == 1 && rc == 1 && lc == 0)
	{
		win_mask.submat(0, 0, 1, 1) = mask_win1;

	}

	if (tr == 0 && br == 0 && rc == 0 && lc == 0)
	{
		win_mask = mask_win1;
	}

	int lval = 0, pow2 = 1;
	//CHECK TOP BLOCK
	if (win_mask(0, 1) == 0)
	{
		lval += pow2;
	}
	pow2 *= 2;
	//CHECK RIGHT BLOCK
	if (win_mask(1, 2) == 0)
	{
		lval += pow2;

	}
	pow2 *= 2;
	//CHECK BOTTOM BLOCK
	if (win_mask(2, 1) == 0)
	{
		lval += pow2;

	}
	pow2 *= 2;
	//CHECK LEFT BLOCK
	if (win_mask(1, 0) == 0)
	{
		lval += pow2;

	}

	switch (lval)
	{

	case 1:
		//		TOP
		intrpval = 0;
		break;
	case 2:
		//		RIGHT
		intrpval = 1;
		break;
	case 3:
		//		TOP,RIGHT
		intrpval = rand() % 2;
		break;
	case 4:
		//		BOTTOM
		intrpval = 2;
		break;
	case 5:
		//		TOP, BOTTOM
		intrpval = (rand() % 3) & 0x2;
		break;
	case 6:
		//		RIGHT,BOTTOM
		intrpval = (rand() % 2 + 1);
		break;
	case 7:
		//		TOP,RIGHT,BOTTOM
		intrpval = (rand() % 3);
		break;
	case 8:
		//		LEFT
		intrpval = 3;
		break;
	case 9:
		//		TOP,LEFT
		intrpval = 0;
		break;
	case 10:
		//		RIGHT,LEFT
		intrpval = 3;
		break;
	case 11:
		//		TOP,RIGHT,LEFT
		intrpval = 1;
		break;
	case 12:
		//		BOTTOM,LEFT
		intrpval = 2;
		break;
	case 13:
		//		TOP,BOTTOM,LEFT
		intrpval = 0;
		break;
	case 14:
		//		RIGHT,BOTTOM,LEFT
		intrpval = 3;
		break;
	case 15:
		//		ALL NEIGHBOURS
		intrpval = (rand() % 4);
		break;
	default:
		intrpval = -1;
	}

	return intrpval;

}

s32 bg_est::unfilled_block_estimate_from_1_neighbour(mat &out_cost,
		const int &interpval, const int & x, const int & y)
{
	u16 N = bge_params_obj->N, blk_id;
	u16 vertical_flg;

	mat F, res1, res2;
	umat W_inv(2 * N, 2 * N), W_512(2 * N, 2 * N), W;
	mat local_region;
	switch (interpval)
	{

	case 0:
		//            %             TOP ONLY
		blk_id = 0;
		break;

	case 1:
		//            %             RIGHT ONLY
		blk_id = 1;
		break;

	case 2:
		//            %           BOTTOM ONLY
		blk_id = 2;
		break;

	case 3:
		//            %             LEFT ONLY
		blk_id = 3;
		break;

	default:
		return -1;

	}

	if (blk_id == 0 || blk_id == 2)
	{
		local_region = zeros<mat> (2 * N, N);
		vertical_flg = 1;
		res2.set_size(2 * N, N);
	}
	else
	{
		local_region = zeros<mat> (N, 2 * N);
		vertical_flg = 0;
		res2.set_size(N, 2 * N);

	}
	out_cost.set_size(rep_blk_cnt(y, x), 1);

	//GET THE LIKELIHOODS
	vec lhood = zeros<vec> (rep_blk_cnt(y, x));
	for (u16 i = 0; i < rep_blk_cnt(y, x); ++i)
	{
		lhood(i) = block_info(y, x)[i].wt;
		lhood(i) = min((int) bge_params_obj->min_frames, (int) lhood(i));
	}
	//		NORMALISE LIKLIHOODS
	lhood = lhood / sum(lhood);

	u16 ofset = N * N;
	vec prior = zeros<vec> (rep_blk_cnt(y, x));
	for (int ch = 0; ch < channels; ++ch)
	{

		local_region.submat(dst_offset_1N(blk_id, 0), dst_offset_1N(blk_id, 1),
				dst_offset_1N(blk_id, 0) + N - 1, dst_offset_1N(blk_id, 1) + N
						- 1) = reshape(bg_frame_matrix(y + src_offset_1N(
				blk_id, 0), x + src_offset_1N(blk_id, 1)).rows(ch * ofset, (ch
				+ 1) * ofset - 1), N, N);

		for (u16 i = 0; i < rep_blk_cnt(y, x); ++i)
		{

			local_region.submat(curr_blk_offset_1N(blk_id, 0),
					curr_blk_offset_1N(blk_id, 1),
					curr_blk_offset_1N(blk_id, 0) + N - 1, curr_blk_offset_1N(
							blk_id, 1) + N - 1) = reshape(
					block_info(y, x)[i].rep_blocks.rows(ch * ofset, (ch + 1)
							* ofset - 1), N, N);

			if (vertical_flg)
			{
				res2 = dct_mtx * local_region;
				res2.row(0) = zeros<rowvec> (N);
			}
			else
			{
				res2 = local_region * trans(dct_mtx);
				res2.col(0) = zeros<vec> (N);
			}

			prior(i) += sum(sum(abs(res2)));

		}

	}

	//	NORMALISE  PRIORS

	prior = prior / sum(prior);

	for (u16 i = 0; i < rep_blk_cnt(y, x); ++i)
	{
		prior(i) = exp(-prior(i) * rep_blk_cnt(y, x));
	}
	//		NORMALISE PRIORS AGAIN
	prior = prior / sum(prior);
	out_cost.col(0) = log(prior) + log(lhood);

	return 0;
}

void bg_est::view_candidate(const vector<blk_stats> &block_info, int blk_n)
{
	u16 N = bge_params_obj->N;
	cube res_mtx(N, N * blk_n, channels);
	u16 ofset = N * N;
	cv::Mat img;
	//	mat blks_data(N, N * blk_n);
	for (int i = 0; i < blk_n; ++i)
	{

		for (int ch = 0; ch < channels; ch++)
		{

			res_mtx.slice(ch).submat(0, (N * i), N - 1, (i * N + N - 1))
					= reshape(block_info[i].rep_blocks.rows(ch * ofset,
							(ch + 1) * ofset - 1), N, N);
		}


	}
	cv::Size imsize(N * blk_n, N);
	if (channels == 3)
	{
		img.create(imsize, CV_8UC3);
	}
	else
	{
		img.create(imsize, CV_8UC1);
	}

	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < (N * blk_n); ++j)
		{

			if (channels == 3)
			{
				cv::Vec3b pix;

				for (int ch = 0; ch < channels; ch++)
				{
					pix[ch] = u8(res_mtx.slice(ch)(i, j));
				}

				img.at<cv::Vec3b> (i, j) = pix;
			}
			else
			{
				img.at<u8> (i, j) = u8(res_mtx.slice(0)(i, j));

			}

		}
	}
	cv::imshow("mainWin1", img);
	cv::waitKey(2);
}

void bg_est::view_local_region(const mat &lblk)
{

	cv::Mat img = cv::Mat::zeros(lblk.n_rows, lblk.n_cols, CV_8U);

	for (u16 i = 0; i < lblk.n_rows; ++i)
	{
		for (u16 j = 0; j < lblk.n_cols; ++j)
		{
			img.at<u8> (i, j) = (u8) lblk(i, j);

		}
	}

	cv::imshow("mainWin1", img);
	cv::waitKey(2);
}

void bg_est::apply_ICM()
{

	int x, y, fill_cnt;
	int interpval;
	s32 best_blk_idx;
	mat cost(1, 1);

	if (sum(sum(mask)) != 0)
	{
		cout << " shd not come here" << endl;
		exit(-23);
	}
	imat initial_mask, tmp;
	initial_mask.copy_size(mask);
	initial_mask.fill(1);

	tmp.copy_size(mask);
	tmp.fill(0);

	bool first_entry_flg = true;

	for (u32 lcnt = 0; lcnt < bge_params_obj->iterations; ++lcnt)
	{

		while ((sum(sum(mask)) != (s32) 0) || (first_entry_flg == true))
		{
			first_entry_flg = false;

			for (y = 0; y <= ymb; y++)
			{
				for (x = 0; x <= xmb; x++)
				{

					mask(y, x) = (lcnt == 0) ? 1 : mask(y, x);

					if (mask(y, x) == 1)
					{

						interpval = find_atleast_3_filled_neighbours(x, y);
						cost.set_size(1, 1);
						cost(0, 0) = -1;

						best_blk_idx
								= unfilled_block_estimate_from_3_neighbours(
										cost, interpval, x, y);

						vec costvec = sum(cost, 1);
						uvec best_id = sort_index(costvec, 1);

						if (best_blk_idx < 0)
						{

							interpval = find_atleast_1_filled_neighbour(x, y);

							cost.set_size(1, 1);
							cost(0, 0) = -1;
							best_blk_idx
									= unfilled_block_estimate_from_1_neighbour(
											cost, interpval, x, y);
							vec costvec = sum(cost, 1);
							uvec best_id = sort_index(costvec, 1);
							if (best_blk_idx < 0)
							{
								cout << " shd not have come here" << endl;
								continue;
							}
						}

						bg_frame_matrix(y, x)
								= block_info(y, x)[best_id(0)].rep_blocks;
						mask(y, x) = 0;

						//						STORE THE CHANGES IN BEST BLOCK IDS
						if (best_blk_id(y, x) != (s16) best_id(0))
						{
							tmp(y, x) = -1;
							best_blk_id(y, x) = best_id(0);

						}
						fill_cnt++;
					}

				}

			}
		}
		mark_8_connected_neighbours(tmp);
		mask = tmp;
		tmp.fill(0);

	}

}

void bg_est::mark_8_connected_neighbours(imat &matrix)
{
	int tr, lc, rc, br;
	int top, left, right, bottom;

	for (u16 y = 0; y < matrix.n_rows; y++)
	{
		for (u16 x = 0; x < matrix.n_cols; x++)
		{

			if (matrix(y, x) == -1)
			{

				(y > 0) ? top = 1 : top = 0;
				(y < ymb) ? bottom = 1 : bottom = 0;

				(x > 0) ? left = 1 : left = 0;
				(x < xmb) ? right = 1 : right = 0;

				(top == 0) ? tr = 1 : tr = 0;
				(bottom == 0) ? br = 1 : br = 0;
				(right == 0) ? rc = 1 : rc = 0;
				(left == 0) ? lc = 1 : lc = 0;

				//				READ THE 8-NEIGHBOURS INTO A MATRIX
				imat mask_win = matrix.submat((y - 1 + tr), (x - 1 + lc), (y
						+ 1 - br), (x + 1 - rc));

				for (u16 j = 0; j < mask_win.n_rows; j++)
				{
					for (u16 i = 0; i < mask_win.n_cols; i++)
					{

						if (mask_win(j, i) == 0)
						{
							mask_win(j, i) = 1;
						}

					}
				}
				//				WRITE BACK THE MATRIX AFTER SETTING THE 8-NEIGHBOURS, IF THEY ARE 0
				matrix.submat((y - 1 + tr), (x - 1 + lc), (y + 1 - br), (x + 1
						- rc)) = mask_win;
				matrix(y, x) = 0;
			}
		}
	}

}
