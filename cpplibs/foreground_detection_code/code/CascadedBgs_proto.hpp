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

#ifndef CASCADEDBGS_H_
#define CASCADEDBGS_H_

#include "mog_diag.hpp"
#include "CascadedBgsParams_proto.hpp"


template <typename eT>
class CascadedBgs
{

public:

	cv::Mat rawMask;
	double detection_percentage;
	string file_name;

	CascadedBgs() {};

	CascadedBgs(const cv::Mat &frame,
			const CascadedBgsParams& cascadedBgsParams);
	virtual ~CascadedBgs();

	void model_estimation(const s32 &len);
	void initialise_new_means();
	void initialise_new_dcovs();
	virtual void detectRaw(const cv::Mat &frame);

private:

	int frameNumber, mod_B_frameNumber;
	field<cv::Mat> imgarray;
	CascadedBgsParams *cascadedBgsParams;
	s16 xmb, ymb;
	u32 no_of_blks;
	field<mog_diag<eT> > gm_model_initial;
	field<mog_diag<eT> > gm_model_final;
	field<mog_diag<eT> > global_image_model;

	field< Col<eT> > Full_update_BG_model;
	field< Col<eT> > mu;
	field< Col<eT> > varience;
	field< Col<eT> > frame_fv;
	Mat<eT> dct_mtx, dct_mtx_trans;
	Mat<eT> fg_wt_mtx;
	Mat<eT> similarity_mtx;
	Mat<u32> fg_wts;
	Col<eT>  DCT_coeffs;
	Col<eT>  global_gmm_id;
	field<Mat<eT> > mask;
	mat fg_persistence;
	field< running_stat_vec<eT> >fg_model;
	cv::Mat bin_image1, bin_image2;
	cv::Mat mean_bg;
	cv::Mat tmp_img;
	cv::Mat threshold_img;
	double seq_threshold;


	u16 channels;
	u16 gauss_flg, toggle_flg;
	u16 frm_idx;
	bool model_B_train;
	cv::VideoWriter demofile;
	u16 width, height;


public:

	void fv_extraction(field<Col<eT> > &f_vec, int len1, const int &channel,
			const Cube<eT> &plane, const u32 &fv_type);

	void frame_fv_extraction(const field< Mat<eT> > &plane_vec, const u32 &fv_type, const u32 &dest_type);

	int detect_foreground(const cv::Mat &frame);
	void create_foreground_mask(const cv::Mat &frame);
	inline void cosinedist(const Col<eT>  &f_vec, const Col<eT>  &mu, eT &cosdistval);

	void choose_dominant_gaussian(s32 flag);

	void create_dct_table(int N);
	void idct_reconstruct(field<Mat <eT> > &res_mtx, u32 lcnt);
	void trace_blocks(const IplImage *frame, const int i, const int j);
	void gmm_reconstruct(field<Mat <eT> > &out_mtx,const u32  & x, const u32  &y);

	void show_mean_background_img(cv::Mat &get_mean_img);
	void img_mask_concatenate(const cv::Mat &input_frame, cv::Mat &concatenate_img);
	void numtostr(int num, char *str);

};

#endif /* CASCADEDBGS_H_ */
