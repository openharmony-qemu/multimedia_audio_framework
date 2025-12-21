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

#ifndef AUDIO_DEVICE_CAPABILITY_H
#define AUDIO_DEVICE_CAPABILITY_H

#include <audio_device_stream_info.h>
#include "cJSON.h"

#define REMOTE_DEFAULT_VOLUME 100

namespace OHOS {
namespace AudioStandard {
class RemoteDeviceCapability {

public:
    std::list<DeviceStreamInfo> streamInfoList_;
    bool isSupportRemoteVolume_ = false;
    int32_t initVolume_ = REMOTE_DEFAULT_VOLUME;
    bool initMuteStatus_ = false;

    std::string GetJsonString() const
    {
        cJSON *interface = cJSON_CreateObject();
        std::string deviceStreamString = DeviceStreamInfo::SerializeList(streamInfoList_);
        cJSON_AddStringToObject(interface, "stream_info", deviceStreamString.c_str());
        cJSON_AddBoolToObject(interface, "support_remote_volume", isSupportRemoteVolume_);
        cJSON_AddNumberToObject(interface, "init_volume", initVolume_);
        cJSON_AddBoolToObject(interface, "init_mute_status", initMuteStatus_);
        char *pChar = cJSON_PrintUnformatted(interface);
        CHECK_AND_RETURN_RET_LOG(pChar != nullptr, "", "pChar is null");
        cJSON_Delete(interface);
        std::string str = pChar;
        cJSON_free(pChar);
        return str;
    }

    void FromJsonString(const std::string &jsonString)
    {
        cJSON *root = cJSON_Parse(jsonString.c_str());
        CHECK_AND_RETURN_LOG(root != nullptr, "root is null");

        cJSON *streamInfoObj = cJSON_GetObjectItem(root, "stream_info");
        if (streamInfoObj && cJSON_IsString(streamInfoObj)) {
            streamInfoList_ = DeviceStreamInfo::DeserializeList(streamInfoObj->valuestring);
        }

        cJSON *volumeSupportObj = cJSON_GetObjectItem(root, "support_remote_volume");
        if (volumeSupportObj && cJSON_IsBool(volumeSupportObj)) {
            isSupportRemoteVolume_ = cJSON_IsTrue(volumeSupportObj);
        }

        cJSON *volumeObj = cJSON_GetObjectItem(root, "init_volume");
        if (volumeObj && cJSON_IsNumber(volumeObj)) {
            initVolume_ = static_cast<int32_t>(volumeObj->valueint);
        }
        cJSON *muteObj = cJSON_GetObjectItem(root, "init_mute_status");
        if (muteObj && cJSON_IsNumber(muteObj)) {
            initMuteStatus_ = cJSON_IsTrue(muteObj);
        }
        cJSON_Delete(root);
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEVICE_CAPABILITY_H