/**
 * ============================================================================
 *
 * Copyright (C) 2018, Hisilicon Technologies Co., Ltd. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1 Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   2 Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   3 Neither the names of the copyright holders nor the names of the
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */

#include "hand_write_inference.h"
#include <vector>
#include "hiaiengine/log.h"
#include "hiaiengine/ai_types.h"
#include "hiaiengine/ai_model_parser.h"
#include "ascenddk/ascend_ezdvpp/dvpp_data_type.h"
#include "ascenddk/ascend_ezdvpp/dvpp_process.h"
#include "opencv2/opencv.hpp"
#include <math.h>
#include <sys/time.h>

using ascend::utils::DvppBasicVpcPara;
using ascend::utils::DvppProcess;
using ascend::utils::DvppVpcOutput;
using hiai::Engine;
using hiai::ImageData;
using hiai::IMAGEFORMAT;
using namespace cv;


namespace {
	// output port (engine port begin with 0)
	const uint32_t kSendDataPort = 0;

	// hand write model input width
	const uint32_t kModelWidth = 112;

	// hand write model input height
	const uint32_t kModelHeight = 112;

	// call dvpp success
	const uint32_t kDvppProcSuccess = 0;
	// level for call DVPP
	const int32_t kDvppToJpegLevel = 100;

	// vpc input image offset
	const uint32_t kImagePixelOffsetEven = 1;
	const uint32_t kImagePixelOffsetOdd = 2;
}

// register custom data type
HIAI_REGISTER_DATA_TYPE("ImageResults", ImageResults);
HIAI_REGISTER_DATA_TYPE("CRect", CRect);
HIAI_REGISTER_DATA_TYPE("CEngineTransT", CEngineTransT);
HIAI_REGISTER_DATA_TYPE("OutputT", OutputT);
HIAI_REGISTER_DATA_TYPE("ScaleInfoT", ScaleInfoT);
HIAI_REGISTER_DATA_TYPE("NewImageParaT", NewImageParaT);
HIAI_REGISTER_DATA_TYPE("BatchImageParaWithScaleT", BatchImageParaWithScaleT);

HandWriteInference::HandWriteInference() {
	ai_model_manager_ = nullptr;
}

HIAI_StatusT HandWriteInference::Init(
    const hiai::AIConfig& config,
    const std::vector<hiai::AIModelDescription>& model_desc) {
	HIAI_ENGINE_LOG("Start initialize!");

	// initialize aiModelManager
	if (ai_model_manager_ == nullptr) {
		ai_model_manager_ = std::make_shared<hiai::AIModelManager>();
	}

	// get parameters from graph.config
	// set model path and passcode to AI model description
	hiai::AIModelDescription fd_model_desc;
	for (int index = 0; index < config.items_size(); index++) {
		const ::hiai::AIConfigItem& item = config.items(index);
		// get model path
		if (item.name() == "model_path") {
			const char* model_path = item.value().data();
			fd_model_desc.set_path(model_path);
		}
	}

	// initialize model manager
	std::vector<hiai::AIModelDescription> model_desc_vec;
	model_desc_vec.push_back(fd_model_desc);
	hiai::AIStatus ret = ai_model_manager_->Init(config, model_desc_vec);
	// initialize AI model manager failed
	if (ret != hiai::SUCCESS) {
		HIAI_ENGINE_LOG(HIAI_GRAPH_INVALID_VALUE, "initialize AI model failed");
		return HIAI_ERROR;
	}

	HIAI_ENGINE_LOG("End initialize!");
	return HIAI_OK;
}


bool HandWriteInference::IsSupportFormat(hiai::IMAGEFORMAT format) {
	return format == hiai::YUV420SP;
}

HIAI_StatusT HandWriteInference::ConvertImage(NewImageParaT& img) {
	hiai::IMAGEFORMAT format = img.img.format;
	if (!IsSupportFormat(format)){
		HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
			            "Format %d is not supported!", format);
		return HIAI_ERROR;
	}

	uint32_t width = img.img.width;
	uint32_t height = img.img.height;
	uint32_t img_size = img.img.size;

	// parameter
	ascend::utils::DvppToJpgPara dvpp_to_jpeg_para;
	dvpp_to_jpeg_para.format = JPGENC_FORMAT_NV12;
	dvpp_to_jpeg_para.level = kDvppToJpegLevel;
	dvpp_to_jpeg_para.resolution.height = height;
	dvpp_to_jpeg_para.resolution.width = width;
	ascend::utils::DvppProcess dvpp_to_jpeg(dvpp_to_jpeg_para);
  

	// call DVPP
	ascend::utils::DvppOutput dvpp_output;
	int32_t ret = dvpp_to_jpeg.DvppOperationProc(reinterpret_cast<char*>(img.img.data.get()),
												img_size, &dvpp_output);

	// failed, no need to send to presenter
	if (ret != kDvppProcSuccess) {
		HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
						"Failed to convert YUV420SP to JPEG, skip it.");
		return HIAI_ERROR;
	}

	// reset the data in img_vec
	img.img.data.reset(dvpp_output.buffer, default_delete<uint8_t[]>());
	img.img.size = dvpp_output.size;

	return HIAI_OK;
}

cv::Mat HandWriteInference::PreProcess(cv::Mat src)
{
	resize(src,src,cv::Size(90,90));
	cv::Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	gray.convertTo(gray,CV_8UC3);

	//Binaryzation by using THRESH method
	bitwise_not(gray, gray);
	cv::Mat thresh;
	threshold(gray, thresh, 0, 255, THRESH_TOZERO | THRESH_OTSU);
	bitwise_not(thresh, thresh);
	int width = src.cols;
	int height = src.rows;
	cv::Mat resize_img;

	//Resize and padding the images to 112*112
	float x = 0.0;
	if ((height > width) && (height < 224))
	{
		x = 224.0 / height;
		resize(thresh, thresh, Size(int(width * x), 224));
		int padding_left = int((224 - int(width * x)) / 2);
		int padding_right = 224 - padding_left - int(width * x);
		copyMakeBorder(thresh, thresh, 0, 0, padding_left, padding_right, BORDER_CONSTANT, Scalar(255));
			
	}
	else
	{
		x = 224.0 / width;
		resize(thresh, thresh, Size(224, int(height * x)));
		int padding_top = int((224 - int(height * x)) / 2);
		int padding_bottom = 224 - padding_top - int(height * x);
		copyMakeBorder(thresh, thresh, padding_top, padding_bottom, 0, 0, BORDER_CONSTANT, Scalar(255));
	}
	resize(thresh, resize_img, Size(112, 112));

	/*Enhance the images if their images' meanvalue are less than110*/
	int rows = resize_img.rows;
	int cols = resize_img.cols;
	bitwise_not(resize_img, resize_img);
	float meanvalue = 0.0;
	float stdvalue = 0.0;
	int cnt = 0;

	//Calculate the meanvalue
	for (int i = 0; i < cols; i++)
	{
		for (int j = 0; j < rows; j++)
		{
			if (resize_img.at<uchar>(j, i) > 0)
			{
				stdvalue += (resize_img.at<uchar>(j, i) - meanvalue) * (resize_img.at<uchar>(j, i) - meanvalue);
			}
		}
	}
	meanvalue /= (cnt + 0.0);
	int num = 0;

	//Enhance the images
	if (meanvalue < 110)
	{
		GaussianBlur(resize_img, resize_img, Size(3, 3), 0, 0);
		stdvalue /= (cnt + 0.0);
		stdvalue = sqrt(stdvalue);
		float power = log(185.0 / (185.0 + 40.0)) / log(meanvalue / (meanvalue + 2 * stdvalue));
		float alpha = 185.0 / pow(meanvalue, power);
		for (int i = 0; i < cols; i++)
		{
			for (int j = 0; j < rows; j++)
			{
				int grayvalue = resize_img.at<uchar>(j, i);
				if (grayvalue > 0)
				{
					resize_img.at<uchar>(j, i) = min(255.0, alpha * pow(grayvalue, power));
				}
			}
		}
	}
	bitwise_not(resize_img, resize_img);
	//Convert the images to RGB
	cvtColor(resize_img,resize_img,COLOR_GRAY2BGR);
	return resize_img;
}

/**
	This function is responsible for extracting the text regions from the input image
	input: cv::Mat img,  type: BGR£¬CV_32FC3
	output: vector<cv::Rect> result_rects
*/
std::vector<cv::Rect> HandWriteInference::detect(cv::Mat img)
{   
	// convert the image type from BGR to RGB
	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	img.convertTo(img,CV_8UC3);
	std::vector<cv::Rect> result_rects;
	float benchmark_len = img.rows;
	float bench_area = img.rows*img.cols;
	
	// extract red area contours from image
	cv::Mat hsv;
	cv::cvtColor(img.clone(), hsv, cv::COLOR_BGR2HSV);
	cv::Mat mask;
	cv::inRange(hsv, cv::Scalar(170, 100, 100), cv::Scalar(180, 255, 255), mask);
	cv::Mat element1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));
	cv::Mat dilate1;
	cv::dilate(mask, dilate1, element1);
	std::vector<cv::Rect> rects;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	std::vector <cv::Point> center_point;
	cv::findContours(dilate1, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	if (contours.size() == 0)
	{
		return result_rects;
	}

	// get the Horizontal outer rectangular boxs of this contours
	for (int i = 0; i < contours.size(); i++)
	{
		cv::Rect rect = cv::boundingRect(contours[i]);
		rects.push_back(rect);
	}

	// Calculate the center distances of all rectangular boxes
	// set a threshold, If the relative distance is less than the threshold, it is considered to belong to the same region
	std::vector<std::vector<int> > dist_list(rects.size());
	for (int i = 0; i < rects.size(); i++)
	{
		dist_list[i].push_back(i);
		for (int j = i + 1; j < rects.size(); j++)
		{
			float two_dist = pow((rects[i].x + 0.5*rects[i].width) - (rects[j].x + 0.5*rects[j].width), 2)
				+ pow((rects[i].y + 0.5*rects[i].height) - (rects[j].y + 0.5*rects[j].height), 2);
			float w_scale = two_dist / benchmark_len;
			if (w_scale < 1.5)
			{
				dist_list[i].push_back(j);
			}
		}
	}

	// use Union-Find method to Merged rectangle that belong to the same region
	std::vector <int> dist_result(dist_list.size(), -1);
	std::vector<int> dist;
	for (int m = 0; m < dist_list.size(); m++)
	{
		dist = dist_list[m];
		for (int j = 0; j < dist.size(); j++)
		{
			if (dist_result[m] == -1)
			{
				dist_result[m] = dist[j];
			}
			else
			{
				dist_result[dist[j]] = dist_result[m];
			}
		}
	}

	std::vector<std::vector<int> > un_list(dist_result.size());
	std::vector<int> un_dist;
	std::vector<cv::Rect> inte_area;

	for (int i = 0; i < dist_result.size(); i++)
	{
		un_list[dist_result[i]].push_back(i);
	}

	// Get the text area outer rectangle box
	for (int m = 0; m < un_list.size(); m++)
	{
		if (un_list[m].empty())
		{
			continue;
		}
		un_dist = un_list[m];
		int xmin = rects[un_dist[0]].x;
		int ymin = rects[un_dist[0]].y;
		int xmax = rects[un_dist[0]].x + rects[un_dist[0]].width;
		int ymax = rects[un_dist[0]].y + rects[un_dist[0]].height;
		for (int j = 1; j < un_dist.size(); j++)
		{
			if (xmin > rects[un_dist[j]].x)
			{
				xmin = rects[un_dist[j]].x;
			}
			if (ymin > rects[un_dist[j]].y)
			{
				ymin = rects[un_dist[j]].y;
			}
			if (xmax < rects[un_dist[j]].x + rects[un_dist[j]].width)
			{
				xmax = rects[un_dist[j]].x + rects[un_dist[j]].width;
			}
			if (ymax < rects[un_dist[j]].y + rects[un_dist[j]].height)
			{
				ymax = rects[un_dist[j]].y + rects[un_dist[j]].height;
			}
		}

		// Calculate relative area
		// set a threshold, If the relative area is larger than the threshold, add it, else ignored
		double area = (xmax - xmin)*(ymax - ymin) / bench_area;
		if (area>0.0015 && area<0.03)
		{
			cv::Rect roi = cv::Rect(xmin, ymin, xmax - xmin, ymax - ymin);
			result_rects.push_back(roi);
		}
	}
	return result_rects;
}


HIAI_IMPL_ENGINE_PROCESS("hand_write_inference",
    HandWriteInference, INPUT_SIZE) {
	HIAI_ENGINE_LOG("Start process!");

	// check arg0 is null or not
	if (arg0 == nullptr) {
		HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
					"Failed to process invalid message.");
		return HIAI_ERROR;
	}

	std::shared_ptr<BatchImageParaWithScaleT> image_handle =
		std::static_pointer_cast<BatchImageParaWithScaleT>(arg0);

	std::shared_ptr<CEngineTransT> ctrans_data = std::make_shared<CEngineTransT>();
	ctrans_data->b_info = image_handle->b_info;

	//extract characters
	vector<ImageResults> b_results;
	for (uint32_t k = 0; k < image_handle->b_info.batch_size; k++) {
    
		ImageData<u_int8_t> img = image_handle->v_img[k].img;
		int img_height = img.height;
		int img_width = img.width;
		//mat src:yuv type
		Mat src(img_height * 3 / 2, img_width, CV_8UC1);

		int copy_size = img_width * img_height * 3 / 2;
		int destination_size = src.cols * src.rows * src.elemSize();
		int ret = memcpy_s(src.data, destination_size, img.data.get(),
							copy_size);
		//yuv to bgr
		Mat dst_temp;
		cvtColor(src, dst_temp, COLOR_YUV420sp2BGR);
		dst_temp.convertTo(dst_temp,CV_32FC3);
	
		//Count the time it takes to deal an image.
		struct timeval start,end;
		double start_t, end_t,tdiff;
		//get all the rects in an image
		std::vector<cv::Rect> rects = detect(dst_temp);

		//for each character
		ImageResults results;
		results.num = 0;
	
		gettimeofday(&start, NULL);
		for(int j=0; j<rects.size(); j++)
		{           
      		cv::Mat roi = dst_temp(rects[j]);
      		roi = PreProcess(roi);
      		cv::resize(roi,roi,Size(112,112));

			uint32_t size2 = roi.total() * roi.channels();
			u_int8_t *image_buf_ptr = new (std::nothrow) u_int8_t[size2];
			memcpy_s(image_buf_ptr, size2, roi.ptr<u_int8_t>(),
								roi.total() * roi.channels());
			ImageData<u_int8_t> temp_img; 
			temp_img.size = size2;    
			temp_img.data.reset(image_buf_ptr,std::default_delete<u_int8_t[]>());

  			//inference for each rect
  			uint32_t input_size = temp_img.size * sizeof(uint8_t);
			// 1.copy image data
			std::shared_ptr<uint8_t> temp = std::shared_ptr<uint8_t>(
				new uint8_t[input_size], std::default_delete<uint8_t[]>());
			// copy memory according to each size
			uint32_t each_size = temp_img.size * sizeof(uint8_t);
			HIAI_ENGINE_LOG("each input image size: %u", each_size);
			errno_t mem_ret = memcpy_s(temp.get(),
								input_size,
								temp_img.data.get(),
								each_size);
			// memory copy failed, no need to inference, send original image
			if (mem_ret != EOK) {
    			HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
      				"prepare image data: memcpy_s() error=%d", mem_ret);
	  			ctrans_data->status = false;
	  			ctrans_data->msg = "HiAIInference Engine memcpy_s image data failed";
	  			// send data to engine output port 0
	  			SendData(kSendDataPort, "CEngineTransT",
    			std::static_pointer_cast<void>(ctrans_data));

	  			return HIAI_ERROR;
			}

			// 2.neural buffer
			std::shared_ptr<hiai::AINeuralNetworkBuffer> neural_buf = std::shared_ptr<
			hiai::AINeuralNetworkBuffer>(
	      		new hiai::AINeuralNetworkBuffer(),
	      		std::default_delete<hiai::AINeuralNetworkBuffer>());
			neural_buf->SetBuffer((void*) temp.get(), input_size);
	
  			// 3.input data
			std::shared_ptr<hiai::IAITensor> input_data = std::static_pointer_cast<
	  		hiai::IAITensor>(neural_buf);
			std::vector<std::shared_ptr<hiai::IAITensor>> input_data_vec;
	  		input_data_vec.push_back(input_data);

         
			// 4.Call Process
			// 1. create output tensor
			hiai::AIContext ai_context;
			std::vector<std::shared_ptr<hiai::IAITensor>> output_data_vector;
			hiai::AIStatus ret = ai_model_manager_->CreateOutputTensor(
	      		input_data_vec, output_data_vector);
			// create failed, also need to send data to post process
			if (ret != hiai::SUCCESS) {
	  			HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
							"failed to create output tensor");
	  			ctrans_data->status = false;
	  			ctrans_data->msg = "HiAIInference Engine CreateOutputTensor failed";
	  			// send data to engine output port 0
	  			SendData(kSendDataPort, "CEngineTransT",
					std::static_pointer_cast<void>(ctrans_data));

	  			return HIAI_ERROR;
	  		}

			// 2. process
			//HIAI_ENGINE_LOG("aiModelManager->Process start!");
			ret = ai_model_manager_->Process(ai_context, input_data_vec,
										output_data_vector,
										AI_MODEL_PROCESS_TIMEOUT);

			// process failed, also need to send data to post process
			if (ret != hiai::SUCCESS) {
	  			HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
						"failed to process ai_model");
	  			ctrans_data->status = false;
	  			ctrans_data->msg = "HiAIInference Engine Process failed";
	  			// send data to engine output port 0
	  			SendData(kSendDataPort, "CEngineTransT",
		    		std::static_pointer_cast<void>(ctrans_data));

	  			return HIAI_ERROR;
	  		}

			// 5.generate output data
			ctrans_data->status = true;  
			std::shared_ptr<hiai::AISimpleTensor> result_tensor =
				std::static_pointer_cast<hiai::AISimpleTensor>(output_data_vector[0]);
			OutputT out;
			out.size = result_tensor->GetSize();
			out.data = std::shared_ptr<uint8_t>(new uint8_t[out.size],
											std::default_delete<uint8_t[]>());
			mem_ret = memcpy_s(out.data.get(), out.size,
									result_tensor->GetBuffer(),
									result_tensor->GetSize());
			// memory copy failed, skip this result
			if (mem_ret != EOK) {
    			HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
								"dealing results: memcpy_s() error=%d", mem_ret);
							continue;
			}
            
    		// add OutputT data to results
    		results.output_datas.push_back(out);
    		// add CRect data to results
    		CRect rect;
    		rect.lt.x = rects[j].x;
    		rect.lt.y = rects[j].y;
    		rect.rb.x = rects[j].x+rects[j].width;
    		rect.rb.y = rects[j].y+rects[j].height;
	  		results.rects.push_back(rect);
    		// add num to results
    		results.num++;
		}

		gettimeofday(&end, NULL);

		start_t = start.tv_sec+double(start.tv_usec)/1e6;

		end_t = end.tv_sec+double(end.tv_usec)/1e6;

		tdiff = end_t-start_t;
  
		HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
							"recognition time %f of %d chars", tdiff,rects.size());


		b_results.push_back(results);

	}//batch end

	// convert the orginal image to JPEG
	for (uint32_t index = 0; index < image_handle->b_info.batch_size; index++){
		HIAI_StatusT convert_ret = ConvertImage(image_handle->v_img[index]);
		if (convert_ret != HIAI_OK) {
      		HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
						"Convert YUV Image to Jpeg failed!");
      		return HIAI_ERROR;
		}  
	}

	ctrans_data->imgs = image_handle->v_img;

	ctrans_data->results = b_results;

	//send results and original image data to post process (port 0)
	HIAI_StatusT hiai_ret = SendData(kSendDataPort, "CEngineTransT",
								std::static_pointer_cast<void>(ctrans_data));

	HIAI_ENGINE_LOG(HIAI_ENGINE_RUN_ARGS_NOT_RIGHT,
					"inference success");
	return hiai_ret;
}
