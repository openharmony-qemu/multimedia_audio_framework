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

#ifndef AUDIO_FORMAT_CONVERTER_INTERFACE_H
#define AUDIO_FORMAT_CONVERTER_INTERFACE_H

#include <cstdint>
#include <memory>

#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// InnerAPI 数据状态枚举（与 CAPI 保持一致）
enum class InputDataStatus {
    HAVE_DATA = 1,          // 有可用数据
    NO_AVAILABLE_DATA = 2,  // 暂时无数据
    DATA_FINISHED = 3,      // 数据流结束
};

// 格式转换器数据请求回调抽象类
class FormatConverterDataCallback {
public:
    virtual ~FormatConverterDataCallback() = default;
    // 返回值: 数据大小（字节），负数表示错误
    // 输出参数: data - 数据指针，status - 数据状态
    virtual int32_t OnRequestData(const void** data, InputDataStatus* status) = 0;
};

// 格式转换器接口类（纯虚接口）
class IAudioFormatConverter {
public:
    virtual ~IAudioFormatConverter() = default;

    // 接口1：Create - 初始化转换器（带格式参数和格式校验）
    virtual int32_t Create(const PcmBufferFormat& inputFormat, const PcmBufferFormat& outputFormat) = 0;

    // 接口2：SetInputCallback - 设置数据请求回调
    virtual int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) = 0;

    // 接口3：Process - 处理音频数据
    virtual int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) = 0;

    // 接口4：Destroy - 销毁转换器
    virtual void Destroy() = 0;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_INTERFACE_H