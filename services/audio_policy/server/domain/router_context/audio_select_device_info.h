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

#ifndef AUDIO_SELECT_DEVICE_INFO_H
#define AUDIO_SELECT_DEVICE_INFO_H

#include <memory>
#include "audio_device_simple_descriptor.h"

namespace OHOS {
namespace AudioStandard {

enum class SelectDeviceType {
    MEDIA_INPUT,
    MEDIA_OUTPUT,
    CALL_INPUT,
    CALL_OUTPUT,
    INVALID_TYPE,
};

struct AudioSelectDeviceInfo {
    std::shared_ptr<AudioDeviceSimpleDescriptor> device_;
    int64_t selectTime_ = 0;

    AudioSelectDeviceInfo() = default;
    AudioSelectDeviceInfo(std::shared_ptr<AudioDeviceSimpleDescriptor> device, int64_t selectTime)
        : device_(device), selectTime_(selectTime) {}
    ~AudioSelectDeviceInfo() = default;
};

struct AudioAppSelectDevice {
    AudioSelectDeviceInfo mediaInput_;
    AudioSelectDeviceInfo mediaOutput_;
    AudioSelectDeviceInfo callInput_;
    AudioSelectDeviceInfo callOutput_;
    BluetoothAndNearlinkPreferredRecordCategory preferredInputCategory_ =
        BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;

    AudioAppSelectDevice() = default;
    ~AudioAppSelectDevice() = default;
};

}
}
#endif
