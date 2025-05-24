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

#include "app_mgr_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioBundleManager {
public:
    static int32_t GetUidByBundleName(std::string bundleName, int userId);
    static std::string GetBundleName();
    static std::string GetBundleNameFromUid(int32_t callingUid);
    static AppExecFwk::BundleInfo GetBundleInfo();
    static AppExecFwk::BundleInfo GetBundleInfoFromUid(int32_t callingUid);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_BUNDLE_MANAGER_H