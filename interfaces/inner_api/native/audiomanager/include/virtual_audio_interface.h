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

#ifndef ST_VIRTUAL_AUDIO_INTERFACE_H
#define ST_VIRTUAL_AUDIO_INTERFACE_H

#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "va_device.h"
#endif

namespace OHOS {
namespace AudioStandard {

#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
class VAStreamCallback {
public:
    virtual int32_t Start() = 0;
    virtual int32_t Stop() = 0;
    virtual int32_t Close() = 0;
    virtual int32_t GetStreamProperty(VAAudioStreamProperty& streamProp) = 0;
    virtual int32_t RequestSharedMem(const VASharedMemInfo& memInfo) = 0;
};

class VAInputStreamCallback : public VAStreamCallback {
public:
    virtual int32_t GetCapturePosition(uint64_t& attr_1, uint64_t& attr_2) = 0;
};

class VADeviceControllerCallback {
public:
    virtual int32_t OpenInputStream(const VAAudioStreamProperty &prop, const VAInputStreamAttribute &attr,
                                    std::shared_ptr<VAInputStreamCallback> &inputStream) = 0;
    virtual int32_t GetParameters(const std::string& key, std::string& value) = 0;

    virtual int32_t SetParameters(const std::string& key, const std::string& value) = 0;
};

class VADeviceBrokerWrapper {
public:
    virtual int32_t OnDevicesConnected(const VADevice &device,
                                       const std::shared_ptr<VADeviceControllerCallback> &controllerCallback) = 0;
    virtual int32_t OnDevicesDisconnected(const VADevice &device) = 0;
};
#endif
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_VIRTUAL_AUDIO_INTERFACE_H