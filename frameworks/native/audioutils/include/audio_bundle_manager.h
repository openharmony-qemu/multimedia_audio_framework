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

#ifndef ST_AUDIO_BUNDLE_MANAGER_H
#define ST_AUDIO_BUNDLE_MANAGER_H

#include <shared_mutex>

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AudioStandard {
class AudioBundleManager {
public:
    static int32_t GetUidByBundleName(std::string bundleName, int userId);
    static std::string GetBundleName();
    static std::string GetBundleNameFromUid(int32_t callingUid);
    static std::string GetBundleNameFromUidCached(int32_t callingUid);
    static void RemoveBundleNameByUid(int32_t callingUid);
    static AppExecFwk::BundleInfo GetBundleInfo();
    static AppExecFwk::BundleInfo GetBundleInfoFromUid(int32_t callingUid);
    static void RemoveBundleInfoByUid(int32_t callingUid);
    static std::string GetBundleNameByToken(const uint32_t tokenIdNum);
    static int32_t IsBundleInstalled(
        const std::string &bundleName, int32_t userId, int32_t appIndex, bool &isInstalled);

private:
    static AudioBundleManager& GetInstance();

    AudioBundleManager() = default;
    ~AudioBundleManager() = default;
    AudioBundleManager(const AudioBundleManager&) = delete;
    AudioBundleManager& operator=(const AudioBundleManager&) = delete;
    AudioBundleManager(AudioBundleManager&&) = delete;
    AudioBundleManager& operator=(AudioBundleManager&&) = delete;

    int32_t GetUidByBundleNameImpl(std::string bundleName, int userId);
    std::string GetBundleNameImpl();
    std::string GetBundleNameFromUidImpl(int32_t callingUid);
    std::string GetBundleNameFromUidCachedImpl(int32_t callingUid);
    void RemoveBundleNameByUidImpl(int32_t callingUid);
    AppExecFwk::BundleInfo GetBundleInfoImpl();
    AppExecFwk::BundleInfo GetBundleInfoFromUidImpl(int32_t callingUid);
    void RemoveBundleInfoByUidImpl(int32_t callingUid);
    std::string GetBundleNameByTokenImpl(const uint32_t tokenIdNum);
    int32_t IsBundleInstalledImpl(
        const std::string &bundleName, int32_t userId, int32_t appIndex, bool &isInstalled);

    std::shared_mutex mapMutex_;
    std::mutex processMutex_;
    std::mutex bundleInfoMapMutex_;
    std::unordered_map<int32_t, std::string> bundleNameMap_;
    std::unordered_map<int32_t, AppExecFwk::BundleInfo> bundleInfoMap_;

    std::string FindBundleNameFromMap(int32_t uid);
    void InsertBundleNameToMap(int32_t uid, const std::string &name);
    void EraseBundleNameFromMap(int32_t uid);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_BUNDLE_MANAGER_H