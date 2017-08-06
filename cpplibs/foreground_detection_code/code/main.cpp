// 
// Please cite the following journal article when using this source code:
//   
//  V. Reddy, C. Sanderson, B.C. Lovell.
//  Improved Foreground Detection via Block-based Classifier Cascade with Probabilistic Decision Integration.
//  IEEE Transactions on Circuits and Systems for Video Technology, Vol. 23, No. 1, pp. 83-93, 2013.
//   
//  DOI: 10.1109/TCSVT.2012.2203199
//   
// You can obtain a copy of this article via:
// http://dx.doi.org/10.1109/TCSVT.2012.2203199


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
#include "CascadedBgsParams_proto.hpp"
#include "CascadedBgsParams_meat.hpp"
#include "CascadedBgs_proto.hpp"
#include "CascadedBgs_meat.hpp"
#include "input_preprocessor.hpp"
#include "main.hpp"
#include <fstream>
#include <iostream>


//writes the foreground masks to the below location if WRITEMASK is defined in main.hpp
const string mask_save_path = "";

/* read params file*/;

float getpar()
{
float datas;
     ifstream infile;
//float datas;
     infile.open("/tmp/params.txt");
     infile >> datas;
     cout << "data" << endl;
     cout << datas << endl;
     infile.close();
return datas;
//return datas;

     /*threshold for Cosine distance based classifier (2nd classifier)*/
     //tmp(2) = 0.001; (0.002 works well for small impg)
     //tmp(2) = data; //0.003 more false positive more separation
}

void numtostr(int num, char *str);
rowvec initialiseParameters();




int main(int argc, char **argv)
{




	cout << "using Armadillo " << arma_version::major << "."
			<< arma_version::minor << "." << arma_version::patch << endl;

	wall_clock timer;

	// initialise the parameters in a row vector
	rowvec cur_param_vec = initialiseParameters();
	cur_param_vec.print("parameters");


	//create a instance system configuration to do the front end processing
	InputPreprocessor config_obj(cur_param_vec, DS_RATIO);

	//read first two parameters
	const s16 N = cur_param_vec(0);
	const s16 ovlstep = cur_param_vec(1);


	string seqname;

	if(argc < 2)
        {
        	cout << "input sequence path is not specified; please set the input path" << endl;
		exit(-1);    
	}
        else if(argc < 3)
	{
		cout << "input sequence name is not specified; setting it to 'test'" << endl;
		seqname = "test";  
	}
        else
	{
		seqname = argv[2];
	}



#ifdef WRITEMASK

		string foldername = "output";
		if (mkdir(foldername.c_str(), 0777) == -1)
		{
			std::cerr << "warning: folder exists"<< endl;
		}

#endif

		

		std::vector<std::string> result;
		config_obj.n_secs = 0;

		config_obj.path = argv[1];

		cout << "Sequence number " << seqname << endl;
		cout << "Input Sequence path " << config_obj.path << endl;


		u32 training_frames = 10;

#ifdef WRITEMASK

		int found;
		foldername.assign(mask_save_path);
		foldername.append(seqname.c_str());
		if (mkdir(foldername.c_str(), 0777) == -1)
		{
			std::cerr << "warning: folder exists"<< endl;
		}

#endif


		//************************************************************************
		// print the input file path
		cout << config_obj.path.c_str() << endl;

		//load files from the folder pointed by member variable 'path'
		config_obj.load_files_from_folder(result);

		// load the first image into member variable 'first_img'
		config_obj.get_first_frame(result);



		// check if the total frames in the sequence is less than specified number of training frames
		training_frames = min((u32) config_obj.sequence_len,training_frames) - 1;

		// downsize 'first_img' depending in DS_RATIO and pad to make it multiple of 'N'
		config_obj.downscale_frame_and_pad_if_necessary();


		//change bgs_cascade paramters if input is grey scale
		config_obj.is_grey_scale_img(cur_param_vec);


		CascadedBgsParams cascadedBgsParams(training_frames, N,
				ovlstep, config_obj.T_vals);
		CascadedBgs<double> Obj(config_obj.padded_input_img,
				cascadedBgsParams);


		for (u32 i = 0; i < training_frames + 1; i++)
		{
			//read frame into 'input_mtx' and then store into 'padded_input_img'
			config_obj.get_input_frame(result, i);

			cv::imshow("mainWin1", config_obj.padded_input_img);
			cvWaitKey(2);
			//object segmentation training
			Obj.detectRaw(config_obj.padded_input_img);

			cout << i << endl;

		}


		cout << "--------" << endl;



		for (u32 i = 0; i < config_obj.sequence_len; i++)
		{
			//read frame into 'input_mtx' and then store into 'padded_input_img'
			config_obj.get_input_frame(result, i);

			timer.tic();
			//perform object segmentation
			Obj.detectRaw(config_obj.padded_input_img);
			config_obj.n_secs += timer.toc();


			// upscale mask output in 'mask_mtx_us' depending on DS_RATIO
			
		   	config_obj.upscale_mask(Obj.rawMask);
     			cv::imshow("mainWin2", config_obj.mask_mtx_us);
			
						

#ifdef WRITEMASK
			string write_path;
			write_path.assign(foldername.c_str());
			write_path.append("/");
			write_path.append(config_obj.curr_filename);
			found = write_path.find_last_of(".");
			write_path.replace(found + 1, 3, "png");
			cout << "path is: " << write_path.c_str() << endl;
			cv::imwrite(write_path.c_str(), config_obj.mask_mtx_us);

#endif


			cv::imshow("mainWin1", config_obj.input_img_mtx);
			cvWaitKey(2);
			cout << " test frame: " << i << endl;

		}

		// output the dimensions of the image, frame rate info etc.
		config_obj.output_statistics();

	




	return 0;
}



//Configuring the algorithm's parameters
rowvec initialiseParameters()
{
  //float param3;
  //getpar();
  float j = getpar();
  cout << j << endl;
        rowvec tmp;
	tmp.set_size(3);//orig = 3
	/*set block size N*/
	tmp(0) = 8;//orig8
	/*set pixel advancement (smaller advancement translates to higher overlap) */
	tmp(1) = 2;
	//tmp(2) = j;
        tmp(2) = j;
	return tmp;
}


/*Function to convert an integer to string*/
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


