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


#include "inc.hpp"
#include "CascadedBgs_proto.hpp"
#include "mog_diag.hpp"

template<typename eT>
CascadedBgs<eT>::CascadedBgs(const cv::Mat &frame,
		const CascadedBgsParams& cascadedBgsParams)
		{

	this->cascadedBgsParams = new CascadedBgsParams(cascadedBgsParams);
	width = frame.cols;
	height = frame.rows;

	channels = frame.channels();
	cv::Size imsize(width, height);
	rawMask.create(imsize, CV_8UC1);


	frameNumber = 0;
	mod_B_frameNumber = 0;
	model_B_train = false;
	imgarray.set_size(this->cascadedBgsParams->len);

	xmb = (width - this->cascadedBgsParams->N)
					/ this->cascadedBgsParams->ovlstep;
	ymb = (height - this->cascadedBgsParams->N)
					/ this->cascadedBgsParams->ovlstep;
	no_of_blks = (1 + ymb) * (1 + xmb);

	gm_model_initial.set_size((1 + ymb), (1 + xmb));
	gm_model_final.set_size((1 + ymb), (1 + xmb));
	global_image_model.set_size(1);
	global_gmm_id.set_size(no_of_blks);

	mu.set_size((1 + ymb), (1 + xmb));
	varience.set_size((1 + ymb), (1 + xmb));
	fg_wts.set_size(height, width);

	frame_fv.set_size(2, no_of_blks);
	Full_update_BG_model.set_size((1 + ymb), (1 + xmb));

	for (u32 j = 0; j < no_of_blks; ++j)
	{

		frame_fv(0, j).set_size(this->cascadedBgsParams->sub_mat_elem
				* channels);
		frame_fv(1, j).set_size(this->cascadedBgsParams->sub_mat_elem
				* channels);

	}
	mask.set_size(2);
	mask(0).set_size((1 + ymb), (1 + xmb));
	mask(1).set_size((1 + ymb), (1 + xmb));
	fg_persistence.set_size((1 + ymb), (1 + xmb));
	fg_persistence.fill(0);
	fg_model.set_size((1 + ymb), (1 + xmb));

	for (int y = 0; y < (1 + ymb); ++y)
	{
		for (int x = 0; x < (1 + xmb); ++x)
		{
			fg_model(y, x).reset();

		}
	}

	for (int y = 0; y < (1 + ymb); ++y)
	{
		for (int x = 0; x < (1 + xmb); ++x)
		{
			mu(y, x).set_size(this->cascadedBgsParams->sub_mat_elem * channels);
			varience(y, x).set_size(this->cascadedBgsParams->sub_mat_elem
					* channels);
			Full_update_BG_model(y, x).set_size(
					this->cascadedBgsParams->sub_mat_elem * channels);
			mask(0)(y, x) = 1;
			mask(1)(y, x) = 1;
		}

	}

	dct_mtx = ones<Mat<eT> > (this->cascadedBgsParams->N,
			this->cascadedBgsParams->N);

	dct_mtx_trans = ones<Mat<eT> > (this->cascadedBgsParams->N,
			this->cascadedBgsParams->N);
	s16 clip_N;
	fg_wt_mtx = zeros<Mat<eT> > (height, width);
	for (int i = 0; i <= height - this->cascadedBgsParams->N; i = i
	+ this->cascadedBgsParams->ovlstep)
	{
		for (int j = 0; j <= width - this->cascadedBgsParams->N; j = j
		+ this->cascadedBgsParams->ovlstep)
		{
			clip_N = min(height - i, (int) this->cascadedBgsParams->N);
			fg_wt_mtx.submat(i, j, i + clip_N - 1, j
					+ this->cascadedBgsParams->N - 1) += 1;
		}
	}

	similarity_mtx.set_size((1 + ymb), (1 + xmb));

	toggle_flg = 0;
	frm_idx = 0;

	imsize.width = width;
	imsize.height = height;
	bin_image1.create(imsize, CV_8UC1);
	bin_image2.create(imsize, CV_8UC3);

	if (channels == 3)
	{
		mean_bg.create(imsize, CV_8UC3);
	}
	else
	{
		mean_bg.create(imsize, CV_8UC1);
	}

	DCT_coeffs.set_size(this->cascadedBgsParams->N * this->cascadedBgsParams->N);
	create_dct_table(this->cascadedBgsParams->N);

	//INITIALISE THE DCT MATRIX 8x8
	for (int i = 0; i < this->cascadedBgsParams->N; i++)
	{
		for (int j = 0; j < this->cascadedBgsParams->N; j++)
			dct_mtx(i, j) = DCT_coeffs[i * this->cascadedBgsParams->N + j];
	}

	dct_mtx_trans = trans(dct_mtx);


		}

template<typename eT> CascadedBgs<eT>::~CascadedBgs()
{
	delete this->cascadedBgsParams;
}

template<typename eT> void CascadedBgs<eT>::detectRaw(const cv::Mat &frame)
		{

	int q = 0;
	cv::Scalar fg_percentage;

	detection_percentage = 0;

	if (frameNumber < this->cascadedBgsParams->len)
	{
		imgarray[frameNumber] = frame.clone();


		frameNumber++;
	}
	else if (frameNumber == this->cascadedBgsParams->len)
	{
		frameNumber++;

		cout << "Building the background model ..." << endl;
		model_estimation(this->cascadedBgsParams->len);
		choose_dominant_gaussian(0);

		cout << "Building complete" << endl;
	}
	else
	{
		frameNumber++;
		q = q + 1;
		frm_idx = frm_idx ^ 1;
		similarity_mtx = zeros<Mat<eT> > ((1 + ymb), (1 + xmb));
		fg_wts = zeros<Mat<u32> > (height, width);
		detect_foreground(frame);
		create_foreground_mask(frame);

		fg_percentage = mean(rawMask);
		fg_percentage[0] /= 0xFF;

		detection_percentage = fg_percentage[0];

		if (fg_percentage[0] > 0.7)
		{
			model_B_train = true;
			cout << fg_percentage[0] << "%" << endl;
			frame.copyTo(imgarray[mod_B_frameNumber]);

			mod_B_frameNumber++;
			cout << mod_B_frameNumber << " " << frameNumber << endl;


			if (mod_B_frameNumber >= 16)
			{
				model_B_train = false;
				model_estimation(17);
				choose_dominant_gaussian(1);
				mod_B_frameNumber = 0;
				detect_foreground(frame);
				cout << " ---------switch triggered -----------" << endl;

			}

		}
		else
		{
			mod_B_frameNumber = 0;
		}

	}

		}

template<typename eT> void CascadedBgs<eT>::model_estimation(
		const s32 &len1)
		{

	int clip_N;
	s16 N = cascadedBgsParams->N;
	s16 fvno = cascadedBgsParams->sub_mat_elem * channels;
	s16 ovlstep = cascadedBgsParams->ovlstep;
	s16 x, y, mbwt;
	u32 lcnt;
	Cube<eT> R = zeros<Cube<eT> > (N, N, len1);
	Cube<eT> G = zeros<Cube<eT> > (N, N, len1);
	Cube<eT> B = zeros<Cube<eT> > (N, N, len1);
	x = 0;
	y = 0;
	lcnt = 0;
	mbwt = width / N;



	field<Col<eT> > f_vec(len1);
	mog_diag<eT> tmp_model;
	s32 count = 0;

	Col<float> loglikelihood_vals(len1*no_of_blks);



	while (count < len1)
	{
		f_vec(count) = zeros<Col<eT> > (fvno);
		++count;
	}
	Row<eT> r_vec_tmp(fvno);
	Row<eT> cosine_vals(len1);

	for (int i = 0; i <= height - N; i = i + ovlstep)
	{
		for (int j = 0; j <= width - N; j = j + ovlstep)
		{
			clip_N = min(height - i, (int) N);

			/*******************************************************************************************/
			for (int f = 0; f < len1; f++)
			{
				for (int k = i, m = 0; k < (i + clip_N); k++, m++)
				{
					for (int l = j, n = 0; l < (j + N); l++, n++)
					{

						if (channels == 3)
						{
							cv::Vec3b pix;
							pix = imgarray[f].at<cv::Vec3b> (k, l);
							B(m, n, f) = (eT) pix[0];
							G(m, n, f) = (eT) pix[1];
							R(m, n, f) = (eT) pix[2];

						}
						else
						{
							arma::u8 pix;
							pix = imgarray[f].at<arma::u8> (k, l);
							R(m, n, f) = (eT) pix;

						}

					}
				}
			}

			fv_extraction(f_vec, len1, 1, R, cascadedBgsParams->fv_type);

			if (channels == 3)
			{
				fv_extraction(f_vec, len1, 2, G, cascadedBgsParams->fv_type);
				fv_extraction(f_vec, len1, 3, B, cascadedBgsParams->fv_type);
			}

			gm_model_initial(y, x).train_kmeans(f_vec,
					cascadedBgsParams->n_gaus, cascadedBgsParams->n_iter,
					cascadedBgsParams->trust, cascadedBgsParams->normalise,
					cascadedBgsParams->print_progress);

			if (cascadedBgsParams->n_gaus > 1)
			{

				if (abs(gm_model_initial(y, x).weights(0) - gm_model_initial(y,
						x).weights(1)) < 0.5)
				{

					tmp_model.train_kmeans(f_vec, 1, cascadedBgsParams->n_iter,
							cascadedBgsParams->trust,
							cascadedBgsParams->normalise,
							cascadedBgsParams->print_progress);

					gm_model_initial(y, x).means(0) = tmp_model.means(0);
					gm_model_initial(y, x).dcovs(0) = tmp_model.dcovs(0);
					gm_model_initial(y, x).weights(0) = tmp_model.weights(0);

				}
			}


			lcnt++;
			x = x + 1;

		}
		x = 0;
		y = y + 1;
	}

		}
template<typename eT> void CascadedBgs<eT>::fv_extraction(
		field<Col<eT> > &f_vec, int len1, const int &channel,
		const Cube<eT> &plane, const u32 &fv_type)
		{
	int N = cascadedBgsParams->N;
	Row<eT> R_row = zeros<Row<eT> > (1, N);
	Col<eT> R_col = zeros<Col<eT> > (N, 1);
	int N2 = N / 2, N3 = N + N2;
	Row<eT> temp(1, N2);
	Row<eT> res_vec(2 * N);
	int offset, idx;
	Mat<eT> tmp_mtx = zeros<Mat<eT> > (N, N);
	eT submat_size = sqrt(cascadedBgsParams->sub_mat_elem);
	Mat<eT> sub_mtx = zeros<Mat<eT> > ((u16) submat_size, (u16) submat_size);
	s16 fv_per_channel = cascadedBgsParams->sub_mat_elem;
	Col<eT> tmp_vec = zeros<Col<eT> > (fv_per_channel);
	eT ltempA, ltempB, ltempC, ltempD;
	Mat<eT> sub_mat(N2, N2);

	switch (channel)
	{
	case 1:
		offset = 0;
		break;
	case 2:
		offset = fv_per_channel;
		break;
	case 3:
		offset = fv_per_channel * 2;
		break;
	default:
		cout << "error" << endl;
		exit(-1);
	}

	switch (fv_type)

	{
	/*DCT BASED FEATURES*/
	case 0:
		for (int f = 0; f < len1; f++)
		{
			tmp_mtx = plane.slice(f) * dct_mtx_trans.submat(0, 0, N - 1,
					submat_size - 1);
			sub_mtx = dct_mtx.submat(0, 0, submat_size - 1, N - 1)
							* (tmp_mtx.submat(0, 0, N - 1, submat_size - 1));

			f_vec(f).rows(offset + 0, offset + fv_per_channel - 1) = reshape(
					sub_mtx, cascadedBgsParams->sub_mat_elem, 1);

		}
		break;
		/*HAAR-LIKE BASED FEATURES*/
	case 1:

		for (int f = 0; f < len1; f++)
		{
			idx = 0;
			R_row = mean(plane.slice(f), 0);
			R_col = mean(plane.slice(f), 1);
			res_vec.cols(0, N - 1) = R_row;
			res_vec.cols(N, 2 * N - 1) = trans(R_col);

			tmp_vec(idx) = mean(res_vec);

			temp.cols(0, N2 - 1) = res_vec.cols(0, N2 - 1);
			ltempA = mean(temp);
			temp.cols(0, N2 - 1) = res_vec.cols(N2, N - 1);
			idx++;
			tmp_vec(idx) = (mean(temp) - ltempA);

			temp.cols(0, N2 - 1) = res_vec.cols(N, N3 - 1);
			ltempC = mean(temp);

			temp.cols(0, N2 - 1) = res_vec.cols(N3, 2 * N - 1);
			idx++;
			ltempD = mean(temp);
			tmp_vec(idx) = (ltempD - ltempC);

			sub_mat = plane.slice(f).submat(0, 0, N2 - 1, N2 - 1);
			ltempA = mean(mean(sub_mat));
			sub_mat = plane.slice(f).submat(N2, N2, N - 1, N - 1);
			ltempB = mean(mean(sub_mat));

			ltempC = tmp_vec(0) - (ltempA + ltempB);

			idx++;
			tmp_vec(idx) = (ltempA + ltempB) - (ltempC);

			f_vec(f).rows(offset + 0, offset + fv_per_channel - 1) = tmp_vec;

		}
		break;

		/*MEAN OF SUB-BLOCKS BASED FEATURES*/
	case 2:

		for (int f = 0; f < len1; f++)
		{
			idx = 0;
			R_row = mean(plane.slice(f), 0);
			R_col = mean(plane.slice(f), 1);
			res_vec.cols(0, N - 1) = R_row;
			res_vec.cols(N, 2 * N - 1) = trans(R_col);
			temp.cols(0, N2 - 1) = res_vec.cols(0, N2 - 1);
			tmp_vec(0) = mean(temp);
			temp.cols(0, N2 - 1) = res_vec.cols(N2, N - 1);
			tmp_vec(1) = mean(temp);
			temp.cols(0, N2 - 1) = res_vec.cols(N, N3 - 1);
			tmp_vec(2) = mean(temp);
			temp.cols(0, N2 - 1) = res_vec.cols(N3, 2 * N - 1);
			tmp_vec(3) = mean(temp);
			f_vec(f).rows(offset + 0, offset + fv_per_channel - 1) = tmp_vec;

		}
		break;
		/*MEAN OF BLOCKS */
	case 3:

		for (int f = 0; f < len1; f++)
		{
			idx = 0;
			R_row = mean(plane.slice(f), 0);
			R_col = mean(plane.slice(f), 1);
			res_vec.cols(0, N - 1) = R_row;
			res_vec.cols(N, 2 * N - 1) = trans(R_col);
			f_vec(f).row(channel - 1) = mean(res_vec);

		}
		break;
	default:

		cout << "Invalid fv type " << endl;
		exit(-1);

	}

		}

template<typename eT> void CascadedBgs<eT>::frame_fv_extraction(
		const field<Mat<eT> > &plane_vec, const u32 &fv_type,
		const u32 &dest_type)
		{
	int N = cascadedBgsParams->N;
	s16 ovlstep = cascadedBgsParams->ovlstep;
	Row<eT> R_row = zeros<Row<eT> > (1, N);
	Col<eT> R_col = zeros<Col<eT> > (N, 1);
	int N2 = N / 2, N3 = N + N2;
	Row<eT> temp(1, N2);
	Row<eT> res_vec(2 * N);
	int offset, idx;
	Mat<eT> tmp_mtx = zeros<Mat<eT> > (N, N);
	field<Col<eT> > f_vec(1);
	f_vec(0) = zeros<Col<eT> > (cascadedBgsParams->sub_mat_elem * channels);
	s16 fv_per_channel = cascadedBgsParams->sub_mat_elem;
	eT submat_size = sqrt(cascadedBgsParams->sub_mat_elem);
	Mat<eT> sub_mtx = zeros<Mat<eT> > ((u16) submat_size, (u16) submat_size);
	Col<eT> tmp_vec = zeros<Col<eT> > (fv_per_channel);
	eT ltempA, ltempB, ltempC, ltempD;
	Mat<eT> sub_mat(N2, N2);
	Mat<eT> plane(N, N);
	int clip_N, x, y;
	x = 0;
	y = 0;

	u32 lcnt = 0;
	for (int i = 0; i <= height - N; i = i + ovlstep)
	{

		clip_N = min(height - i, (int) N);

		for (int j = 0; j <= width - N; j = j + ovlstep)
		{

			for (int ch = 1; ch <= channels; ch++)
			{

				switch (ch)
				{
				case 1:
					offset = 0;
					break;
				case 2:
					offset = fv_per_channel;
					break;
				case 3:
					offset = fv_per_channel * 2;
					break;
				default:
					cout << "error" << endl;
					exit(-1);
				}

				switch (fv_type)

				{
				/*DCT BASED FEATURES*/
				case 0:

					tmp_mtx = plane_vec(ch - 1).submat(i, j, (i + clip_N - 1),
							(j + N - 1)) * dct_mtx_trans.submat(0, 0, N - 1,
									submat_size - 1);
					sub_mtx = dct_mtx.submat(0, 0, submat_size - 1, N - 1)
									* (tmp_mtx.submat(0, 0, N - 1, submat_size - 1));
					f_vec(0).rows(offset + 0, offset + fv_per_channel - 1)
									= reshape(sub_mtx, fv_per_channel, 1);

					break;
					/*HAAR-LIKE BASED FEATURES*/
				case 1:
					plane.set_size(clip_N, N);
					plane = plane_vec(ch - 1).submat(i, j, (i + clip_N - 1), (j
							+ N - 1));
					idx = 0;
					R_row = mean(plane_vec(ch - 1).submat(i, j,
							(i + clip_N - 1), (j + N - 1)), 0);
					R_col = mean(plane_vec(ch - 1).submat(i, j,
							(i + clip_N - 1), (j + N - 1)), 1);
					res_vec.cols(0, N - 1) = R_row;
					res_vec.cols(N, 2 * N - 1) = trans(R_col);

					tmp_vec(idx) = mean(res_vec);

					temp.cols(0, N2 - 1) = res_vec.cols(0, N2 - 1);
					ltempA = mean(temp);
					temp.cols(0, N2 - 1) = res_vec.cols(N2, N - 1);
					idx++;
					tmp_vec(idx) = (mean(temp) - ltempA);

					temp.cols(0, N2 - 1) = res_vec.cols(N, N3 - 1);
					ltempC = mean(temp);

					temp.cols(0, N2 - 1) = res_vec.cols(N3, 2 * N - 1);
					idx++;
					ltempD = mean(temp);
					tmp_vec(idx) = (ltempD - ltempC);

					sub_mat = plane.submat(0, 0, N2 - 1, N2 - 1);
					ltempA = mean(mean(sub_mat));
					sub_mat = plane.submat(N2, N2, N - 1, N - 1);
					ltempB = mean(mean(sub_mat));

					ltempC = tmp_vec(0) - (ltempA + ltempB);

					idx++;
					tmp_vec(idx) = (ltempA + ltempB) - (ltempC);

					f_vec(0).rows(offset + 0, offset + fv_per_channel - 1)
									= tmp_vec;

					break;

					/*MEAN OF SUB-BLOCKS BASED FEATURES*/
				case 2:

					idx = 0;
					R_row = mean(plane_vec(ch - 1).submat(i, j,
							(i + clip_N - 1), (j + N - 1)), 0);
					R_col = mean(plane_vec(ch - 1).submat(i, j,
							(i + clip_N - 1), (j + N - 1)), 1);
					res_vec.cols(0, N - 1) = R_row;
					res_vec.cols(N, 2 * N - 1) = trans(R_col);
					temp.cols(0, N2 - 1) = res_vec.cols(0, N2 - 1);
					tmp_vec(0) = mean(temp);
					temp.cols(0, N2 - 1) = res_vec.cols(N2, N - 1);
					tmp_vec(1) = mean(temp);
					temp.cols(0, N2 - 1) = res_vec.cols(N, N3 - 1);
					tmp_vec(2) = mean(temp);
					temp.cols(0, N2 - 1) = res_vec.cols(N3, 2 * N - 1);
					tmp_vec(3) = mean(temp);
					f_vec(0).rows(offset + 0, offset + fv_per_channel - 1)
									= tmp_vec;

					break;
					/*MEAN OF BLOCKS */
				case 3:

					idx = 0;
					R_row = mean(plane_vec(ch - 1).submat(i, j,
							(i + clip_N - 1), (j + N - 1)), 0);
					R_col = mean(plane_vec(ch - 1).submat(i, j,
							(i + clip_N - 1), (j + N - 1)), 1);
					res_vec.cols(0, N - 1) = R_row;
					res_vec.cols(N, 2 * N - 1) = trans(R_col);
					f_vec(0).row(ch - 1) = mean(res_vec);

					break;
				default:

					cout << "Invalid fv type " << endl;
					exit(-1);

				}

			}

			if (dest_type == 0)
			{
				frame_fv(frm_idx, lcnt) = f_vec(0);
			}
			else if (dest_type == 1)
			{
				gm_model_final(y, x).means(0) = f_vec(0);

			}

			lcnt++;
			x++;
		}
		x = 0;
		y++;
	}

		}

template<typename eT> int CascadedBgs<eT>::detect_foreground(
		const cv::Mat &frame)
		{

	int clip_N;
	s16 N = cascadedBgsParams->N;
	s16 ovlstep = cascadedBgsParams->ovlstep;
	s16 fvno = cascadedBgsParams->sub_mat_elem * channels;
	s16 x, y, mbwt;
	u32 lcnt;
	eT cur_cosineval_out, tmp_cosineval_out, log_liklihood_val;

	Mat<eT> buffer(fvno, 2);
	buffer.col(0).fill(0.5);
	u32 max_val_id;
	const eT alpha = cascadedBgsParams->alpha;
	const eT rho_fixed = cascadedBgsParams->rho;

	field<Mat<eT> > img_frame(channels);

	img_frame(0).set_size(height, width);
	if (channels == 3)
	{
		img_frame(1).set_size(height, width);
		img_frame(2).set_size(height, width);
	}
	x = 0;
	y = 0;
	lcnt = 0;

	mbwt = width / N;
	field<Col<eT> > f_vec(1);
	field<Col<eT> > tmp_mu, tmp_var;
	tmp_mu.set_size(fvno);
	tmp_var.set_size(fvno);
	f_vec(0) = zeros<Col<eT> > (fvno);
	Col<eT> temp_diff(fvno);
	Row<eT> dev(1, fvno);

	/*reading the frame into armadillo matrix*/
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{

			if (channels == 3)
			{
				cv::Vec3b pix;
				pix = frame.at<cv::Vec3b> (i, j);
				img_frame(2)(i, j) = (eT) pix[0];
				img_frame(1)(i, j) = (eT) pix[1];
				img_frame(0)(i, j) = (eT) pix[2];
			}
			else
			{
				img_frame(0)(i, j) = (eT) frame.at<arma::u8> (i, j);

			}

		}

	}

	frame_fv_extraction(img_frame, cascadedBgsParams->fv_type, 0);

	for (int i = 0; i <= height - N; i = i + ovlstep)
	{
		clip_N = min(height - i, (int) N);

		for (int j = 0; j <= width - N; j = j + ovlstep)
		{

			bool matchflg = false;
			Col<eT> thr = gm_model_final(y, x).means(0) + 2.0 * sqrt(gm_model_final(y, x).dcovs(0));

			const eT threshold = gm_model_final(y, x).log_lhood(thr);
			gm_model_final(y, x).adapt(matchflg, log_liklihood_val, max_val_id,
					frame_fv(frm_idx, lcnt), threshold, alpha, rho_fixed);

			mask(frm_idx)(y, x) = 1;

			if (matchflg == true)
			{
				mask(frm_idx)(y, x) = 0;

			}
			else
			{
				cosinedist(frame_fv(frm_idx, lcnt),
						gm_model_final(y, x).means(max_val_id),
						cur_cosineval_out);

				if ((cur_cosineval_out < cascadedBgsParams->cosinedist_T))
				{
					mask(frm_idx)(y, x) = 0;
				}
				else if ((mask(frm_idx ^ 1)(y, x) == 0))
				{
					cosinedist(frame_fv(frm_idx, lcnt), frame_fv(frm_idx
							^ 1, lcnt), tmp_cosineval_out);
					if ((tmp_cosineval_out
							< cascadedBgsParams->tmprl_cosinedist_T))
					{
						mask(frm_idx)(y, x) = 0;
					}

				}

			}

			if (mask(frm_idx)(y, x) == 1)
			{
				fg_wts.submat(i, j, i + clip_N - 1, j + N - 1) += 1;
				fg_persistence(y, x) += 1;
				fg_model(y, x)(frame_fv(frm_idx, lcnt));

				if (fg_persistence(y, x) > 36000)
				{
					field<Col<eT> > tmp_mu, tmp_var;
					Col<eT> tmp_wt(1);
					tmp_mu.set_size(1);
					tmp_var.set_size(1);
					tmp_mu(0) = fg_model(y, x).mean();
					tmp_var(0) = fg_model(y, x).var();
					tmp_wt(0) = 1.0;
					mog_diag<eT> tmp_model(tmp_mu, tmp_var, tmp_wt);
					gm_model_final(y, x) = tmp_model;
				}
			}
			else
			{
				fg_persistence(y, x) = 0;
				fg_model(y, x).reset();
			}

			lcnt++;
			x = x + 1;

		}
		x = 0;
		y = y + 1;

	}

	return (0);
		}
template<typename eT> void CascadedBgs<eT>::create_foreground_mask(
		const cv::Mat &frame)
		{

	Mat<unsigned char> mask1, mask2;

	Mat<eT> tmp_mask = this->cascadedBgsParams->likelihood_ratio_T * fg_wt_mtx;
	Mat<u32> tmp_mask1 = conv_to<Mat<u32> >::from(tmp_mask);
	// GET THE FOREGROUND MASK
	Mat<u32> mask = fg_wts > tmp_mask1;


	//CONVERT THE MASK TO U8
	mask1 = conv_to<Mat<unsigned char> >::from(mask);

	//TRANSPOSE THE MASK TO MAKE IN COMPATIBLE WITH OPENCV FORMAT FOR A MEMCPY
	mask2 = trans(mask1 * 255);

	std::memcpy(rawMask.data, mask2.memptr(), mask2.n_elem);

		}


template<typename eT> inline void CascadedBgs<eT>::cosinedist(
		const Col<eT> &f_vec, const Col<eT> &mu, eT &cosdist)
		{

	cosdist = 1 - norm_dot(f_vec, mu);

		}

template<typename eT> void CascadedBgs<eT>::choose_dominant_gaussian(
		s32 flag)
		{
	s16 fvno = cascadedBgsParams->sub_mat_elem * channels;
	s16 x, y;
	u32 bestidx;
	Col<eT> temp_mu(fvno), temp_var(fvno);
	Col<eT> tmp_wt;

	field<Col<eT> > tmp_mu, tmp_var;
	Col<eT> tmp_diff(fvno);
	tmp_wt.set_size(cascadedBgsParams->n_gaus_final);
	tmp_mu.set_size(cascadedBgsParams->n_gaus_final);
	tmp_var.set_size(cascadedBgsParams->n_gaus_final);

	if (flag == 0)
	{

		for (y = 0; y <= ymb; y++)
		{

			for (x = 0; x <= xmb; x++)
			{

				ucolvec indices = sort_index(gm_model_initial(y, x).weights, 1);
				for (u32 i = 0; i < cascadedBgsParams->n_gaus_final; ++i)
				{

					bestidx = indices(i);
					tmp_mu(i) = gm_model_initial(y, x).means(bestidx);
					tmp_var(i) = gm_model_initial(y, x).dcovs(bestidx);
					tmp_wt(i) = gm_model_initial(y, x).weights(bestidx);

				}

				mog_diag<eT> tmp_model(tmp_mu, tmp_var, tmp_wt);

				gm_model_final(y, x) = tmp_model;
				Full_update_BG_model(y, x) = gm_model_final(y, x).means(0);

			}

		}
	}
	else
	{

		for (y = 0; y <= ymb; y++)
		{

			for (x = 0; x <= xmb; x++)
			{

				ucolvec indices = sort_index(gm_model_initial(y, x).weights, 1);
				for (u32 i = 0; i < cascadedBgsParams->n_gaus_final; ++i)
				{

					bestidx = indices(i);
					tmp_mu(i) = gm_model_initial(y, x).means(bestidx);
					tmp_var(i) = gm_model_initial(y, x).dcovs(bestidx);
					tmp_wt(i) = gm_model_initial(y, x).weights(bestidx);

				}

				mog_diag<eT> tmp_model(tmp_mu, tmp_var, tmp_wt);
				gm_model_final(y, x).means(0) = tmp_model.means(0);

			}

		}

	}
		}

template<typename eT> void CascadedBgs<eT>::create_dct_table(int N)
		{

	int k = 0;
	eT scale_fac_i;

	for (eT m = 0; m < N; m++)
	{

		for (eT n = 0; n < N; n++)
		{

			scale_fac_i = (m == 0) ? sqrt(1.0 / eT(N)) : sqrt(2.0 / eT(N));
			DCT_coeffs(k++) = scale_fac_i * std::cos(eT((math::pi() * m) / (2
					* N) * (2 * n + 1)));
		}
	}

		}

template<typename eT> void CascadedBgs<eT>::idct_reconstruct(
		field<Mat<eT> > &out_mtx, u32 lcnt)
		{

	Mat<eT> tmp_mtx = zeros<Mat<eT> > (cascadedBgsParams->N,
			cascadedBgsParams->N);
	Col<eT> f = frame_fv(frm_idx, lcnt);
	u32 ind = 0;
	for (int i = 0; i < cascadedBgsParams->sub_mat_elem * channels; i = i + 4)
	{

		tmp_mtx.submat(0, 0, 1, 1) = reshape(f.rows(i, i + 3), 2, 2);
		out_mtx(ind) = trans(dct_mtx) * tmp_mtx * dct_mtx;
		ind++;
		tmp_mtx.fill(0);

	}

		}



template<typename eT> void CascadedBgs<eT>::trace_blocks(
		const IplImage *frame, const int i, const int j)
		{

	//	 FOR DEBUGGING PURPOSE ONLY+++++++++++++++++++++++++++++++++++++++++++++++++++++
	IplImage *temp = cvCloneImage((const IplImage*) frame);
	cvRectangle(temp, cvPoint(j, i), cvPoint(j + cascadedBgsParams->N, i
			+ cascadedBgsParams->N), cvScalar(0, 0, 255), 1, 8, 0);
	cvShowImage("mainWin1", temp);
	cvWaitKey(10);
	cvReleaseImage(&temp);
		}
//	 FOR DEBUGGING PURPOSE ONLY+++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename eT> void CascadedBgs<eT>::gmm_reconstruct(
		field<Mat<eT> > &out_mtx, const u32 & x, const u32 &y)
		{

	Mat<eT> tmp_mtx = zeros<Mat<eT> > (cascadedBgsParams->N,
			cascadedBgsParams->N);
	Col<eT> f = Full_update_BG_model(y, x);
	eT submat_size = sqrt(cascadedBgsParams->sub_mat_elem);
	u32 ind = 0;
	for (int i = 0; i < cascadedBgsParams->sub_mat_elem * channels; i = i
	+ cascadedBgsParams->sub_mat_elem)
	{

		tmp_mtx.submat(0, 0, submat_size - 1, submat_size - 1) = reshape(
				f.rows(i, i + cascadedBgsParams->sub_mat_elem - 1),
				submat_size, submat_size);
		out_mtx(ind) = trans(dct_mtx) * tmp_mtx * dct_mtx;
		ind++;
		tmp_mtx.fill(0);

	}

		}

template<typename eT> void CascadedBgs<eT>::show_mean_background_img(
		cv::Mat &get_mean_img)
		{

	field<Mat<eT> > blk_mtx(channels);
	blk_mtx(0).set_size(cascadedBgsParams->N, cascadedBgsParams->N);

	cv::Vec3f pix;
	if (channels == 3)
	{

		blk_mtx(1).set_size(cascadedBgsParams->N, cascadedBgsParams->N);
		blk_mtx(2).set_size(cascadedBgsParams->N, cascadedBgsParams->N);
	}

	for (int y = 0; y <= ymb; ++y)
	{
		for (int x = 0; x <= xmb; ++x)
		{
			gmm_reconstruct(blk_mtx, x, y);

			for (int p = 0; p < cascadedBgsParams->N; ++p)
			{
				for (int q = 0; q < cascadedBgsParams->N; ++q)
				{
					if (channels == 3)
					{
						pix[2] = std::min(std::max(blk_mtx(0)(p, q), 0.0),
								255.0);
						pix[1] = std::min(std::max(blk_mtx(1)(p, q), 0.0),
								255.0);
						pix[0] = std::min(std::max(blk_mtx(2)(p, q), 0.0),
								255.0);
						mean_bg.at<cv::Vec3b> (y * cascadedBgsParams->ovlstep
								+ p, x * cascadedBgsParams->ovlstep + q)
								= (cv::Vec3b) pix;
					}
					else
					{

						mean_bg.at<arma::u8> (y * cascadedBgsParams->ovlstep
								+ p, x * cascadedBgsParams->ovlstep + q)
								= (arma::u8) std::min(std::max(
										blk_mtx(0)(p, q), 0.0), 255.0);

					}
				}
			}

		}
	}

	get_mean_img = mean_bg.clone();

		}



template<typename eT> void CascadedBgs<eT>::numtostr(int num, char *str)
		{
	int i = 0;
	int temp = num;
	char arr[128];

	if (temp == 0)
	{
		str[i] = 0x30;
		str[i + 1] = '\0';
		return;
	}

	while (temp)
	{
		int q = temp % 10;
		temp = temp / 10;
		arr[i] = (unsigned char) ((q) + 0x30);
		i++;
	}

	arr[i] = '\0';
	int len = strlen(arr);
	i = 0;
	for (int var = len - 1; var >= 0; var--)
	{
		str[i] = arr[var];
		i++;
	}
	str[i] = '\0';

		}
