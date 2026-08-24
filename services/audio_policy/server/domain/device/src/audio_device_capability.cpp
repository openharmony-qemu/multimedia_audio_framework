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

#include "audio_device_capability.h"
#include "audio_errors.h"
#include "audio_log.h"
#include "cJSON.h"

namespace OHOS {
namespace AudioStandard {

void RemoteDeviceCapability::SetSupportDynamicPipes(const std::vector<AudioFlag> &flags)
{
    supportDynamicPipes_ = AUDIO_FLAG_NONE;
    for (const auto &flag : flags) {
        supportDynamicPipes_ |= static_cast<uint32_t>(flag);
    }
}

bool RemoteDeviceCapability::IsSupportDynamicPipe(AudioFlag flag) const
{
    return (supportDynamicPipes_ & static_cast<uint32_t>(flag)) != AUDIO_FLAG_NONE;
}

std::string RemoteDeviceCapability::GetJsonString() const
{
    cJSON *interface = cJSON_CreateObject();
    std::string deviceStreamString = DeviceStreamInfo::SerializeList(streamInfoList_);
    cJSON_AddStringToObject(interface, "stream_info", deviceStreamString.c_str());
    cJSON_AddBoolToObject(interface, "support_remote_volume", isSupportRemoteVolume_);
    cJSON_AddNumberToObject(interface, "init_volume", initVolume_);
    cJSON_AddBoolToObject(interface, "init_mute_status", initMuteStatus_);
    cJSON_AddBoolToObject(interface, "isSupportHiLinkControl", isSupportHiLinkControl_);
    cJSON_AddStringToObject(interface, "device_name", deviceName_.c_str());
    cJSON_AddNumberToObject(interface, "protocol", protocol_);
    cJSON_AddNumberToObject(interface, "hd_play_mode", hdPlaybackMode_);
    cJSON_AddNumberToObject(interface, "tokenId", deviceTokenId_);
    cJSON_AddNumberToObject(interface, "support_dynamic_pipes", supportDynamicPipes_);
    char *pChar = cJSON_PrintUnformatted(interface);
    CHECK_AND_RETURN_RET_LOG(pChar != nullptr, "", "pChar is null");
    cJSON_Delete(interface);
    std::string str = pChar;
    cJSON_free(pChar);
    return str;
}

void RemoteDeviceCapability::FromJsonStringExt(const std::string &jsonString)
{
    cJSON *root = cJSON_Parse(jsonString.c_str());
    CHECK_AND_RETURN_LOG(root != nullptr, "root is null");

    cJSON *supportDynamicPipesObj = cJSON_GetObjectItem(root, "support_dynamic_pipes");
    if (supportDynamicPipesObj && cJSON_IsNumber(supportDynamicPipesObj)) {
        supportDynamicPipes_ = static_cast<uint32_t>(supportDynamicPipesObj->valueint);
    }
    cJSON_Delete(root);
}

void RemoteDeviceCapability::FromJsonString(const std::string &jsonString)
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

    cJSON *supportHilinkControlObj = cJSON_GetObjectItem(root, "support_hilink_control");
    if (supportHilinkControlObj && cJSON_IsNumber(supportHilinkControlObj)) {
        isSupportHiLinkControl_ = static_cast<int32_t>(supportHilinkControlObj->valueint) == 1;
    }

    cJSON *muteObj = cJSON_GetObjectItem(root, "init_mute_status");
    if (muteObj && cJSON_IsNumber(muteObj)) {
        initMuteStatus_ = cJSON_IsTrue(muteObj);
    }

    cJSON *deviceNameObj = cJSON_GetObjectItem(root, "device_name");
    if (deviceNameObj && cJSON_IsString(deviceNameObj)) {
        deviceName_ = deviceNameObj->valuestring;
    }
    cJSON *protocolObj = cJSON_GetObjectItem(root, "protocol");
    if (protocolObj && cJSON_IsNumber(protocolObj)) {
        protocol_ = static_cast<int32_t>(protocolObj->valueint);
    }

    cJSON *hdPlaybackModeObj = cJSON_GetObjectItem(root, "hd_play_mode");
    if (hdPlaybackModeObj && cJSON_IsNumber(hdPlaybackModeObj)) {
        hdPlaybackMode_ = static_cast<int32_t>(hdPlaybackModeObj->valueint);
    }

    cJSON *tokenIdObj = cJSON_GetObjectItem(root, "tokenId");
    if (tokenIdObj && cJSON_IsNumber(tokenIdObj)) {
        deviceTokenId_ = static_cast<uint32_t>(tokenIdObj->valueint);
    }

    FromJsonStringExt(jsonString);

    cJSON_Delete(root);
}
}
}