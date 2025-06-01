/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRouteMap"
#endif

#include "audio_router_map.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_policy_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
static const size_t FAST_ROUTE_LIMIT = 1024;

std::string AudioRouteMap::GetDeviceInfoByUidAndPid(int32_t uid, int32_t pid)
{
    std::string selectedDevice = "";
    std::lock_guard<std::mutex> lock(routerMapMutex_);
    if (!routerMap_.count(uid)) {
        AUDIO_INFO_LOG("GetSelectedDeviceInfo no such uid[%{public}d]", uid);
        return "";
    }
    if (routerMap_[uid].second == pid) {
        selectedDevice = routerMap_[uid].first;
    } else if (routerMap_[uid].second == -1) {
        routerMap_[uid].second = pid;
        selectedDevice = routerMap_[uid].first;
    } else {
        AUDIO_INFO_LOG("GetSelectedDeviceInfo: uid[%{public}d] changed pid, get local as defalut", uid);
        routerMap_.erase(uid);
        selectedDevice = LOCAL_NETWORK_ID;
    }
    return selectedDevice;
}

bool AudioRouteMap::DelRouteMapInfoByKey(int32_t uid)
{
    std::lock_guard<std::mutex> lock(routerMapMutex_);
    return routerMap_.erase(uid);
}

void AudioRouteMap::AddRouteMapInfo(int32_t uid, std::string device, int32_t pid)
{
    std::lock_guard<std::mutex> lock(routerMapMutex_);
    routerMap_[uid] = std::pair(device, pid);
}

int32_t AudioRouteMap::AddFastRouteMapInfo(int32_t uid, std::string device, DeviceRole role)
{
    std::lock_guard<std::mutex> lock(fastRouterMapMutex_);
    if (fastRouterMap_.size() > FAST_ROUTE_LIMIT) {
        return ERROR;
    }
    fastRouterMap_[uid] = std::make_pair(device, role);
    return SUCCESS;
}

void AudioRouteMap::RemoveDeviceInRouterMap(std::string networkId)
{
    std::lock_guard<std::mutex> lock(routerMapMutex_);
    std::unordered_map<int32_t, std::pair<std::string, int32_t>>::iterator it;
    for (it = routerMap_.begin();it != routerMap_.end();) {
        if (it->second.first == networkId) {
            it = routerMap_.erase(it);
        } else {
            it++;
        }
    }
}

void AudioRouteMap::RemoveDeviceInFastRouterMap(std::string networkId)
{
    std::lock_guard<std::mutex> lock(fastRouterMapMutex_);
    std::unordered_map<int32_t, std::pair<std::string, DeviceRole>>::iterator it;
    for (it = fastRouterMap_.begin();it != fastRouterMap_.end();) {
        if (it->second.first == networkId) {
            it = fastRouterMap_.erase(it);
        } else {
            it++;
        }
    }
}

void AudioRouteMap::GetNetworkIDInFastRouterMap(int32_t uid, DeviceRole role, std::string& newworkId)
{
    std::lock_guard<std::mutex> lock(fastRouterMapMutex_);
    if (fastRouterMap_.count(uid) &&
        fastRouterMap_[uid].second == role) {
        newworkId = fastRouterMap_[uid].first;
        AUDIO_INFO_LOG("use networkid in fastRouterMap_ :%{public}s ", GetEncryptStr(newworkId).c_str());
    }
}

}
}
