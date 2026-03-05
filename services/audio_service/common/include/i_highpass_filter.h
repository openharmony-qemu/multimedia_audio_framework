/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef I_HIGHPASS_FILTER
#define I_HIGHPASS_FILTER
#include <cstdint>
#include <cstddef>
#include <vector>

namespace OHOS {
namespace AudioStandard {
class IHighPassFilter {
/**
 * 4阶Butterworth高通滤波器，滤除截止频率18000以下低频信号，保留高频超声信号
 * 输入数据格式：采样率48000，stereo，双通道交错，16位PCM
 * 输出数据格式：采样率48000，stereo，双通道交错，16位PCM
 */
public:
    virtual ~IHighPassFilter() = default;
    virtual int32_t Init(const int32_t channels) = 0;
    virtual int32_t Apply(const std::vector<float> &samples, std::vector<float> &result) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
typedef int32_t AudioHighpassFilterClassCreateFunc(OHOS::AudioStandard::IHighPassFilter **filter);
#endif