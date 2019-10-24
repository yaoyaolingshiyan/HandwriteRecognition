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
#ifndef HAND_WRITE_POST_PROCESS_H_
#define HAND_WRITE_POST_PROCESS_H_
#include "hand_write_params.h"
#include "hiaiengine/api.h"
#include "hiaiengine/ai_types.h"
#include "hiaiengine/data_type.h"
#include "hiaiengine/data_type_reg.h"
#include "hiaiengine/engine.h"
#include "ascenddk/presenter/agent/presenter_channel.h"


#define INPUT_SIZE 1
#define OUTPUT_SIZE 1

// hand write configuration
struct HandWritePostConfig {
  float confidence;  // confidence
  std::string presenter_ip;  // presenter server IP
  int32_t presenter_port;  // presenter server port for agent
  std::string channel_name;  // channel name
};

/**
 * @brief: hand write post process
 */
class HandWritePostProcess : public hiai::Engine {
public:
  /**
   * @brief: construction function
   */
  HandWritePostProcess();

  /**
   * @brief: the destruction function
   */
  ~HandWritePostProcess() = default;

  /**
   * @brief: hand write post process engine initialize
   * @param [in]: engine's parameters which configured in graph.config
   * @param [in]: model description
   * @return: HIAI_StatusT
   */
  HIAI_StatusT Init(const hiai::AIConfig& config,
                    const std::vector<hiai::AIModelDescription>& model_desc);

  /**
   * @brief: engine processor
   *         1. dealing results
   *         2. call OSD to draw box and label if needed
   *         3. call DVPP to change YUV420SP to JPEG
   *         4. call presenter agent to send JPEG to server
   * @param [in]: input size
   * @param [in]: output size
   */
  HIAI_DEFINE_PROCESS(INPUT_SIZE, OUTPUT_SIZE);

private:
  // configuration
  std::shared_ptr<HandWritePostConfig> fd_post_process_config_;

  // presenter channel
  std::shared_ptr<ascend::presenter::Channel> presenter_channel_;

  /**
   * @brief: handle original image
   * @param [in]: EngineTransT format data which inference engine send
   * @return: HIAI_StatusT
   */
  HIAI_StatusT HandleOriginalImage(
      const std::shared_ptr<CEngineTransT> &inference_res);

  /**
   * @brief: handle results
   * @param [in]: EngineTransT format data which inference engine send
   * @return: HIAI_StatusT
   */
  HIAI_StatusT HandleResults(
      const std::shared_ptr<CEngineTransT> &inference_res);

  /**
   * @brief: validate IP address
   * @param [in]: IP address
   * @return: true: invalid
   *          false: valid
   */
  bool IsInValidIp(const std::string &ip);

  /**
   * @brief: validate port
   * @param [in]: port
   * @return: true: invalid
   *          false: valid
   */
  bool IsInValidPort(int32_t port);

  /**
   * @brief: validate channel name
   * @param [in]: channel name
   * @return: true: invalid
   *          false: valid
   */
  bool IsInValidChannelName(const std::string &channel__name);

  /**
   * @brief: validate confidence
   * @param [in]: confidence
   * @return: true: invalid
   *          false: valid
   */
  bool IsInvalidConfidence(float confidence);


  /**
   * @brief: validate results
   * @param [in]: attribute
   * @param [in]: score
   * @param [in]: left top anchor
   * @param [in]: right bottom anchor
   * @return: true: invalid
   *          false: valid
   */
  bool IsInvalidResults(float attr, float score,
                        const ascend::presenter::Point &point_lt,
                        const ascend::presenter::Point &point_rb);

  /**
   * @brief: convert YUV420SP to JPEG, and then send to presenter
   * @param [in]: image height
   * @param [in]: image width
   * @param [in]: image size
   * @param [in]: image data
   * @return: FD_FUN_FAILED or FD_FUN_SUCCESS
   */
  int32_t SendImage(uint32_t height, uint32_t width, uint32_t size,
                    u_int8_t *data, vector<ascend::presenter::DetectionResult>& detection_results);

  /**
   * @brief: find Chinese characters from the index table
   * @param [in]: The nth line
   * @return: the corresponding Chinese character
   */ 
  string GetCharacterByLine(int n);
};

#endif /* HAND_WRITE_POST_PROCESS_H_ */
