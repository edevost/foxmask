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


#ifndef INPUT_PREPROCESSOR_HPP_
#define INPUT_PREPROCESSOR_HPP_

#include "inc.hpp"
#include "main.hpp"
#include "CascadedBgsParams_proto.hpp"



class InputPreprocessor
{
public:
	double n_secs, acc_secs;
	bgscascade_thresholds T_vals;
	string curr_filename;

	s16 N;
	s16 fvno;
	s16 ovlstep;
	u32 n_gaus;
	u32 n_gaus_final;
	u32 n_iter;
	u16 total_seqs;
	ifstream param_file;
	char filename[200];
	string ref_frno, frno;
	string path, path1, outfile, seqname;
	int lcnt;
	int ltemp;
	s16 ind;
	string outfileptr;
	cv::Mat first_img;
	cv::Mat img_sc, input_img_mtx, mask_mtx_us;
	cv::Mat padded_input_img;

	cv::Size tsize;
	float ratio;

	u32 true_height, true_width;
	u32  prow, pcol;

	u32 height, width, channels;
	u32 sequence_len;
	cv::VideoCapture vidfile;

	input_type in_data_format;


	InputPreprocessor(const rowvec &cur_param_vec, double ratio);
	void initialise_algorithm_parameters(const rowvec &cur_param_vec);
	void downscale_frame_and_pad_if_necessary();
	void fill_padded_region();
	void upscale_mask(cv::Mat &mask);
	void draw_bounding_boxes();
    void is_grey_scale_img(const rowvec &cur_param_vec);
    void  load_files_from_folder(std::vector<std::string> &out_result);

    void get_input_frame(const std::vector<std::string> result, u32 frm_idx);
    void get_first_frame(const std::vector<std::string> result);
	void numtostr(int num, char *str);
	void output_statistics();


	virtual ~InputPreprocessor();
};

#endif /* INPUT_PREPROCESSOR_HPP_ */
