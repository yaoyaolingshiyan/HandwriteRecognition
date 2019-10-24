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
#ifndef HAND_WRITE_PARAMS_H_
#define HAND_WRITE_PARAMS_H_

#include "hiaiengine/data_type.h"
#include "opencv2/opencv.hpp"

/**
 * @brief: custom data type: ScaleInfo
 */
struct ScaleInfoT {
  float scale_width = 1;
  float scale_height = 1;
};

template<class Archive>
void serialize(Archive& ar, ScaleInfoT& data) {
  ar(data.scale_width, data.scale_height);
}

/**
 * @brief: custom data type: NewImagePara
 */
struct NewImageParaT {
  hiai::FrameInfo f_info;
  hiai::ImageData<u_int8_t> img;
  ScaleInfoT scale_info;
};

template<class Archive>
void serialize(Archive& ar, NewImageParaT& data) {
  ar(data.f_info, data.img, data.scale_info);
}

/**
 * @brief: custom data type: NewImagePara2
 */
struct NewImageParaT2 {
  hiai::FrameInfo f_info;
  hiai::ImageData<float> img;
  ScaleInfoT scale_info;
};

template<class Archive>
void serialize(Archive& ar, NewImageParaT2& data) {
  ar(data.f_info, data.img, data.scale_info);
}

/**
 * @brief: custom data type: BatchImageParaWithScale
 */
struct BatchImageParaWithScaleT {
  hiai::BatchInfo b_info;
  std::vector<NewImageParaT> v_img;
};

template<class Archive>
void serialize(Archive& ar, BatchImageParaWithScaleT& data) {
  ar(data.b_info, data.v_img);
}

/**
 * @brief: custom data type: ImageAll
 */
struct ImageAll {
  int width_org;
  int height_org;
  int channal_org;
  hiai::ImageData<float> image;
};

template<class Archive>
void serialize(Archive& ar, ImageAll& data) {
  ar(data.width_org, data.height_org, data.channal_org, data.image);
}

/**
 * @brief: custom data type: BatchImageParaScale
 */
struct BatchImageParaScale {
  hiai::BatchInfo b_info;             // batch信息
  std::vector<ImageAll> v_img;  // batch中的图像
};

template<class Archive>
void serialize(Archive& ar, BatchImageParaScale& data) {
  ar(data.b_info, data.v_img);
}

/**
 * @brief: custom data type: OutputT
 *         defined for results output data
 */
struct OutputT {
  int32_t size;
  std::shared_ptr<u_int8_t> data;
};

/**
 * @brief: serialize for OutputT
 *         engine uses it to transfer data between host and device
 */
template<class Archive>
void serialize(Archive& ar, OutputT& data) {
  ar(cereal::binary_data(data.data.get(), data.size * sizeof(u_int8_t)));
}

/**
 * @brief: character rectangle
 */
struct CRect {
  hiai::Point2D lt;  // left top
  hiai::Point2D rb;  // right bottom
};

/**
 * @brief: serialize for character rectangle
 *         engine uses it to transfer data between host and device
 */
template<class Archive>
void serialize(Archive& ar, CRect& data) {
  ar(data.lt, data.rb);
}

/**
 * @brief: custom data type: ImageResults
 *         detection and recognition results of an image
 */

struct ImageResults{
  int num;
  std::vector<OutputT> output_datas;
  //std::vector<NewImageParaT> imgs;
  std::vector<CRect> rects;
};

/**
 * @brief: serialize for ImageResults
 *         engine uses it to transfer data between host and device
 */
template<class Archive>
void serialize(Archive& ar, ImageResults& data) {
  ar(data.num, data.output_datas, data.rects);
}


struct CEngineTransT{
  bool status;
  std::string msg;  // error message
  hiai::BatchInfo b_info;
  std::vector<NewImageParaT> imgs;
  std::vector<ImageResults> results;

};

/**
 * @brief: serialize for CEngineTransT
 *         engine uses it to transfer data between host and device
 */
template<class Archive>
void serialize(Archive& ar, CEngineTransT& data) {
  ar(data.status, data.msg, data.b_info, data.imgs, data.results);
}




#endif /* HAND_WRITE_PARAMS_H_ */
