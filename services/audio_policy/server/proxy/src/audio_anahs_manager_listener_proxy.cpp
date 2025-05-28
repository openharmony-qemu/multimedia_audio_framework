/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioAnahsManagerListenerProxy"
#endif

#include "audio_anahs_manager_listener_proxy.h"
#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioAnahsManagerListenerProxy::AudioAnahsManagerListenerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStandardAudioAnahsManagerListener>(impl)
{
    AUDIO_DEBUG_LOG("Instances create");
}

AudioAnahsManagerListenerProxy::~AudioAnahsManagerListenerProxy()
{
    AUDIO_DEBUG_LOG("~AudioAnahsManagerListenerProxy: Instance destroy");
}


int32_t AudioAnahsManagerListenerProxy::OnExtPnpDeviceStatusChanged(std::string anahsStatus, std::string anahsShowType)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    CHECK_AND_RETURN_RET_LOG(data.WriteInterfaceToken(GetDescriptor()), ERROR,
        "OnAudioOutputDeviceRefined: WriteInterfaceToken failed");
    data.WriteString(anahsStatus);
    data.WriteString(anahsShowType);

    int error = Remote()->SendRequest(ON_AUDIO_ANAHS_DEVICE_CHANGE, data, reply, option);
    CHECK_AND_RETURN_RET_LOG(error == ERR_NONE, error, "OnExtPnpDeviceStatusChanged, error: %{public}d", error);

    int32_t result = reply.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result,
        "OnExtPnpDeviceStatusChanged callback failed, error %{public}d", result);

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
