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

#ifndef VA_DEVICE_BROKER_STUB_H
#define VA_DEVICE_BROKER_STUB_H

#include <iremote_broker.h>

#include "virtual_audio_interface.h"
#include "va_device_broker_stub.h"

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_device_descriptor.h"

#include "va_device.h"
#include "iv_a_device_controller.h"

namespace OHOS {
namespace AudioStandard {

class VADeviceBrokerStubImpl : public VADeviceBrokerStub {
public:
    static sptr<VADeviceBrokerStubImpl> Create();

    VADeviceBrokerStubImpl();
    virtual ~VADeviceBrokerStubImpl();

    int32_t OnDevicesConnected(const VADevice& device, const sptr<IRemoteObject>& controller)override;
    int32_t OnDevicesDisconnected(const VADevice& device)override;
};
}  //namespace AudioStandard
}  //namespace OHOS
#endif //VA_DEVICE_BROKER_STUB_H