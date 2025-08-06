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

#ifndef HPAE_RENDERER_MANAGER_ADD_FUZZER_H
#define HPAE_RENDERER_MANAGER_ADD_FUZZER_H

#include "audio_info.h"
#include "audio_stream_info.h"
#include "i_capturer_stream.h"
#include "i_renderer_stream.h"

namespace OHOS {
namespace AudioStandard {
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

class ReadDataCb : public ICapturerStreamCallback, public std::enable_shared_from_this<ReadDataCb> {
public:
    explicit ReadDataCb(const std::string &fileName);
    virtual ~ReadDataCb();
    int32_t OnStreamData(AudioCallBackCapturerStreamInfo &callBackStreamInfo) override;
private:
    FILE *testFile_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // HPAE_RENDERER_MANAGER_ADD_FUZZER_H
