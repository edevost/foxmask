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

#include"inc.hpp"
#include"SequentialBge.hpp"

void numtostr(int num, char *str);

string outpath = "../output/17_02_10/";
string gt_path = "../input/reference/";
string err_path = "../output/err_images/";

int main(int argc, char **argv)
{

	cout << "using Armadillo " << arma_version::major << "."
			<< arma_version::minor << "." << arma_version::patch << endl;

	wall_clock timer;




	double n_secs, acc_secs = 0;
	int same_as_src_flg = -1;
	//define constant
	bg_est_param_values T_vals;

        /*Parameters that to tune if required*/ 
	T_vals.N = 8;
	T_vals.Ovlstep = T_vals.N;
	/*for setting the MAD threshold*/        
	T_vals.Pixel_Diff = 15; //5 orig (15 works for small)
	/*for setting the correlation coefficient threshold*/        
	T_vals.Corr_Coef = 0.8;
	/*weight for spatial term*/        
	T_vals.Eta = 3;
	//T_vals.MinFrames = 100;
	T_vals.MinFrames = 5;
	
	T_vals.Iterations= 1;

	const s16 total_seqs = 12;

	char filename[200];
	string ref_frno, frno;
	string path, outfile, estimatedBG,respath;
	int len2;
	int lcnt;
	int ltemp;
	s16 ind;
	string outfileptr;



	if(argc < 2)
	{
		cout << "input sequence path is not specified; please set the input path" << endl;
		exit(-1);
	}
	else if(argc < 3)
	{
		cout << "input sequence name is not specified; setting it to 'bg'" << endl;
		//estimatedBG = "bg";
	}
	else
	{
		estimatedBG = argv[2];
	}


	path = argv[1];
	respath = argv[1];
	ind = 0;
	std::vector<std::string> result;
	size_t found;
	DIR *D;
	struct dirent *Dirp;
	lcnt = 0;
	n_secs = 0;



	cout << " BG Image name " <<  ": " << estimatedBG.c_str()
							<< endl;

	char lbuffer[10], *buffptr;
	string tmp_path;
	buffptr = lbuffer;

	//************************************************************************
	tmp_path.assign(path);

	D = opendir(path.c_str()); //"." Refers to the project location, any path can be placed here
	while ((Dirp = readdir(D)) != NULL)
	{
		std::string str(Dirp->d_name);
		found = str.find_last_of(".");
		if (str.substr(found + 1) == "bmp" || str.substr(found + 1)
				== "jpg"|| str.substr(found + 1)
				== "png" || str.substr(found + 1)
				== "tif")
		{
			result.push_back(str);
		}

	}

	closedir(D);
	std::sort(result.begin(), result.end());
	std::vector<std::string>::iterator iter;
	std::cerr << "N images: " << result.size() << std::endl;
	int length = result[0].length();
	result[0].copy(filename, length, 0);
	filename[length] = '\0';
	tmp_path.append(filename);
	cv::Mat img = cv::imread(tmp_path, same_as_src_flg);
	if (img.empty())
	{
		cout << "Unable to open the input sequence\n";
		exit(-1);
	}

	int training_frames = min((int)result.size(), 400);

	bge_params bge_param_obj(training_frames, T_vals);
	bg_est Obj(img, bge_param_obj);

	// get the image data
	int height = img.rows;
	int width = img.cols;
	int channels = img.channels();
	cv::Mat input_img_mtx;

	cvNamedWindow("mainWin1", CV_WINDOW_NORMAL);
	cvMoveWindow("mainWin1", 400, 40);
	cvNamedWindow("mainWin2", CV_WINDOW_NORMAL);
	cvMoveWindow("mainWin2", 900, 40);


	ltemp = atoi(frno.c_str());
	numtostr(ltemp, lbuffer);

	for (int i = 0; i < training_frames; i++)
	{
		length = result[i].length();
		result[i].copy(filename, length, 0);
		filename[length] = '\0';
		tmp_path.assign(path);
		tmp_path.append(filename);
		input_img_mtx = cv::imread(tmp_path.c_str(), same_as_src_flg);

		Obj.detectRaw(input_img_mtx);
		cv::imshow("mainWin1",input_img_mtx);
		cv::imshow("mainWin2", Obj.estimated_bg_frame);
		cvWaitKey(2);

	}
	vector<int> params;
	// get path for output dir
	//QString respath = QDir(path).filePath(estimatedBG);
	params.push_back(CV_IMWRITE_JPEG_QUALITY);
	params.push_back(99);
	estimatedBG.append("_0000.jpg");
	cout << "bg name : " << estimatedBG << endl;
	
	
	respath.append(estimatedBG);
	cout << "respath : " << respath  << endl;
	cv::imwrite(respath.c_str() ,Obj.estimated_bg_frame,params);
	// second saving
	respath = argv[1];
	estimatedBG = argv[2];
	estimatedBG.append("_0001.jpg");
	respath.append(estimatedBG);
	cout << "respath : " << respath  << endl;
	cv::imwrite(respath.c_str() ,Obj.estimated_bg_frame,params);
	// third saving
	respath = argv[1];
	estimatedBG = argv[2];
	estimatedBG.append("_0002.jpg");
	respath.append(estimatedBG);
	cout << "respath : " << respath  << endl;
	cv::imwrite(respath.c_str() ,Obj.estimated_bg_frame,params);
	// fourth saving
	respath = argv[1];
	estimatedBG = argv[2];
	estimatedBG.append("_0003.jpg");
	respath.append(estimatedBG);
	cout << "respath : " << respath  << endl;
	cv::imwrite(respath.c_str() ,Obj.estimated_bg_frame,params);

	
	//cv::imwrite(respath.c_str() ,Obj.estimated_bg_frame,params);
	

	cout << "Processing a " << width << " x " << height
			<< " image with " << channels << " channels" << endl;


	cout << endl << "Background estimation is complete" << endl;
	cout << "+++++++++++++++++++++++++++++++++++++" << endl;

	cvDestroyWindow("mainWin1");
	cvDestroyWindow("mainWin2");



	cout << "exit successfully" << endl;




	return 0;
}

void numtostr(int num, char *str)
{
	int i = 0;
	int temp = num;
	char arr[20];

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
