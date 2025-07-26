/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEST_CASE_COMMON_H
#define TEST_CASE_COMMON_H
#include <cstdint>
#include "i_renderer_stream.h"
#include "i_capturer_stream.h"
#include "audio_errors.h"
#include "hpae_info.h"
#include "hpae_msg_channel.h"

namespace OHOS {
namespace AudioStandard {
constexpr int OFFSET_BIT_24 = 3;
constexpr int BIT_DEPTH_TWO = 2;
constexpr int BIT_16 = 16;
constexpr int BIT_8 = 8;
constexpr float TEST_VALUE_PRESION = 0.001;
constexpr int TEST_FREAME_LEN = 125;
constexpr int TEST_SUB_FREAME_LEN = 50;
constexpr int TEST_LEN_LT_FOUR = 3;

#define DEFAULT_TEST_SINK_NAME "hdi_output"
#define DEFAULT_TEST_AUDIO_DEVICE_NAME "Speaker"
#define DEFAULT_TEST_DEVICE_CLASS "file_io"
#define DEFAULT_TEST_DEVICE_NETWORKID "LocalDevice"

class WriteFixedDataCb : public IStreamCallback, public std::enable_shared_from_this<WriteFixedDataCb> {
public:
    int32_t OnStreamData(AudioCallBackStreamInfo& callBackStremInfo) override;

    explicit WriteFixedDataCb(AudioSampleFormat format) : format_(format)
    {}
    virtual ~WriteFixedDataCb()
    {}

private:
    int32_t writeNum_ = 0;
    AudioSampleFormat format_ = SAMPLE_F32LE;
};

class WriteFixedValueCb : public IStreamCallback, public std::enable_shared_from_this<WriteFixedValueCb> {
public:
    int32_t OnStreamData(AudioCallBackStreamInfo& callBackStremInfo) override;
    WriteFixedValueCb(AudioSampleFormat format, int32_t fixedValue) : format_(format), fixValue_(fixedValue)
    {}
    virtual ~WriteFixedValueCb()
    {}

private:
    AudioSampleFormat format_ = SAMPLE_F32LE;
    int32_t fixValue_ = 0;
};

class WriteIncDataCb : public IStreamCallback, public std::enable_shared_from_this<WriteIncDataCb> {
public:
    int32_t OnStreamData(AudioCallBackStreamInfo& callBackStremInfo) override;
    explicit WriteIncDataCb(AudioSampleFormat format) : format_(format)
    {}
    virtual ~WriteIncDataCb()
    {}

private:
    int32_t writeNum_ = 0;
    AudioSampleFormat format_ = SAMPLE_F32LE;
};

class StatusChangeCb : public IStreamStatusCallback, public std::enable_shared_from_this<StatusChangeCb> {
public:
    void OnStatusUpdate(IOperation operation, uint32_t streamIndex) override;
    IStatus GetStatus();
    virtual ~StatusChangeCb() = default;
private:
    IStatus status_;
};

class ReadDataCb : public ICapturerStreamCallback, public std::enable_shared_from_this<ReadDataCb> {
public:
    explicit ReadDataCb(const std::string &fileName);
    virtual ~ReadDataCb();
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override;
private:
    FILE *testFile_ = nullptr;
};

class NodeStatusCallback : public HPAE::INodeCallback, public std::enable_shared_from_this<NodeStatusCallback> {
public:
    virtual ~NodeStatusCallback() = default;
};

void TestCapturerSourceFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes);
} // namespace AudioStandard
} // namespace OHOS
#endif