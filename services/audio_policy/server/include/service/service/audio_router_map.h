
/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ROUTER_MAP_H
#define ST_AUDIO_ROUTER_MAP_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioRouteMap {
public:
    static AudioRouteMap& GetInstance()
    {
        static AudioRouteMap instance;
        return instance;
    }
    std::string GetDeviceInfoByUidAndPid(int32_t uid, int32_t pid);
    bool DelRouteMapInfoByKey(int32_t uid);
    void AddRouteMapInfo(int32_t uid, std::string device, int32_t pid);
    int32_t AddFastRouteMapInfo(int32_t uid, std::string device, DeviceRole role);
    void RemoveDeviceInRouterMap(std::string networkId);
    void RemoveDeviceInFastRouterMap(std::string networkId);
    void GetNetworkIDInFastRouterMap(int32_t uid, DeviceRole role, std::string& newworkId);
private:
    AudioRouteMap() {}
    ~AudioRouteMap() {}
private:
    std::mutex fastRouterMapMutex_; // unordered_map is not concurrently-secure
    std::mutex routerMapMutex_; // unordered_map is not concurrently-secure
    std::unordered_map<int32_t, std::pair<std::string, int32_t>> routerMap_;
    std::unordered_map<int32_t, std::pair<std::string, DeviceRole>> fastRouterMap_; // key:uid value:<netWorkId, Role>
};
}
}
#endif