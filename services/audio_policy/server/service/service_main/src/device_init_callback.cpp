/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "DeviceStatusCallbackImpl"
#endif

#include "device_init_callback.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

#ifdef FEATURE_DEVICE_MANAGER

static string GetExtraDataField(const string &src, const string &field)
{
    auto pos = src.find(field);
    CHECK_AND_RETURN_RET(pos != string::npos, "");
    pos = src.find(':', pos + field.length());
    CHECK_AND_RETURN_RET(pos != string::npos, "");
    auto end = ++pos;
    for (; end < src.length(); end++) {
        if (src[end] == '}' || src[end] == ',') {
            break;
        }
    }
    auto value = end == src.length() ? src.substr(pos) : src.substr(pos, end - pos);
    for (pos = 0; pos < value.length(); pos++) {
        auto ch = value[pos];
        if (ch != '\\' && ch != '"') {
            break;
        }
    }
    for (end = value.length() - 1; end >= 0; end--) {
        auto ch = value[end];
        if (ch != '\\' && ch != '"') {
            break;
        }
    }
    return value.substr(pos, end - pos + 1);
}

static DmDevice ParseDmDevice(const DistributedHardware::DmDeviceInfo &dmDeviceInfo)
{
    string carBrand = GetExtraDataField(dmDeviceInfo.extraData, "\"CAR_BRAND\\");
    CHECK_AND_RETURN_RET_LOG(!carBrand.empty(), {}, "Can not find field: CAR_BRAND");
    return {
        carBrand,
        dmDeviceInfo.networkId,
        DistributedHardware::DEVICE_TYPE_CAR,
    };
}

DeviceStatusCallbackImpl::DeviceStatusCallbackImpl()
    : audioPolicyService_(AudioPolicyService::GetAudioPolicyService())
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
}

void DeviceStatusCallbackImpl::OnDeviceChanged(const DistributedHardware::DmDeviceBasicInfo &dmDeviceBasicInfo)
{
    std::string strDeviceName(dmDeviceBasicInfo.deviceName);
    AUDIO_INFO_LOG("OnDeviceChanged:remote name [%{public}s]", Hide(strDeviceName).c_str());

    //OnDeviceChanged listeren did not report networkId information
    AudioConnectedDevice::GetInstance().SetDisplayName(strDeviceName, false);
}

void DeviceStatusCallbackImpl::OnDeviceChanged(const DistributedHardware::DmDeviceInfo &dmDeviceInfo)
{
    AUDIO_INFO_LOG("Entry. deviceName=%{public}s, dmDeviceType=%{public}d, networkId=%{public}s",
        Hide(dmDeviceInfo.deviceName).c_str(), dmDeviceInfo.deviceTypeId, Hide(dmDeviceInfo.networkId).c_str());
    auto dmDev = ParseDmDevice(dmDeviceInfo);
    if (!dmDev.deviceName_.empty()) {
        AudioConnectedDevice::GetInstance().UpdateDmDeviceMap(std::move(dmDev), true);
    }
}

void DeviceStatusCallbackImpl::OnDeviceOnline(const DistributedHardware::DmDeviceInfo &dmDeviceInfo)
{
    AUDIO_INFO_LOG("Entry. deviceName=%{public}s, dmDeviceType=%{public}d, networkId=%{public}s",
        Hide(dmDeviceInfo.deviceName).c_str(), dmDeviceInfo.deviceTypeId, Hide(dmDeviceInfo.networkId).c_str());
    auto dmDev = ParseDmDevice(dmDeviceInfo);
    if (!dmDev.deviceName_.empty()) {
        AudioConnectedDevice::GetInstance().UpdateDmDeviceMap(std::move(dmDev), true);
    }
}

void DeviceStatusCallbackImpl::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &dmDeviceInfo)
{
    AUDIO_INFO_LOG("Entry. deviceName=%{public}s, dmDeviceType=%{public}d, networkId=%{public}s",
        Hide(dmDeviceInfo.deviceName).c_str(), dmDeviceInfo.deviceTypeId, Hide(dmDeviceInfo.networkId).c_str());
    AudioConnectedDevice::GetInstance().UpdateDmDeviceMap({ .networkId_ = dmDeviceInfo.networkId }, false);
}
#endif
} // namespace AudioStandard
} // namespace OHOS