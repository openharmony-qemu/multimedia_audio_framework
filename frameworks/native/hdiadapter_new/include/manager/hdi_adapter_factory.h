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

#ifndef HDI_ADAPTER_FACTORY_H
#define HDI_ADAPTER_FACTORY_H

#include <iostream>
#include <cstring>
#include <memory>
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "adapter/i_device_manager.h"

namespace OHOS {
namespace AudioStandard {
class HdiAdapterFactory {
public:
    static HdiAdapterFactory &GetInstance(void);
    std::shared_ptr<IAudioRenderSink> CreateRenderSink(uint32_t renderId);
    std::shared_ptr<IAudioCaptureSource> CreateCaptureSource(uint32_t captureId);
    std::shared_ptr<IDeviceManager> CreateDeviceManager(uint32_t type);

private:
    HdiAdapterFactory() = default;
    ~HdiAdapterFactory() = default;
    HdiAdapterFactory(const HdiAdapterFactory &) = delete;
    HdiAdapterFactory &operator=(const HdiAdapterFactory &) = delete;
    HdiAdapterFactory(HdiAdapterFactory &&) = delete;
    HdiAdapterFactory &operator=(HdiAdapterFactory &&) = delete;

    std::shared_ptr<IAudioRenderSink> CreatePrimaryRenderSink(const uint32_t renderId, const std::string &info);
    std::shared_ptr<IAudioRenderSink> CreateBluetoothRenderSink(const std::string &info);
    std::shared_ptr<IAudioRenderSink> CreateRemoteRenderSink(const std::string &info);
    std::shared_ptr<IAudioRenderSink> CreateRemoteFastRenderSink(const std::string &info);
    std::shared_ptr<IAudioCaptureSource> CreatePrimaryCaptureSource(const uint32_t captureId, const std::string &info);
    std::shared_ptr<IAudioCaptureSource> CreateRemoteCaptureSource(const std::string &info);
    std::shared_ptr<IAudioCaptureSource> CreateRemoteFastCaptureSource(const std::string &info);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_ADAPTER_FACTORY_H
