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
#ifndef DOWNLOAD_UT_TEST_COMMON_H
#define DOWNLOAD_UT_TEST_COMMON_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "audio_errors.h"
#include "audio_suite_sa_types.h"

constexpr int32_t MAX_RETRY_COUNT = 3;
constexpr int32_t DOWNLOAD_COMPLETE = 100;
constexpr int32_t CLOUD_ROM_NETTYPE = 2 | 4;
constexpr int32_t CALLBACK_REGISTER_TIMEOUT_MS = 1000;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

inline StatusInfo CreateStatusInfo(const std::string &version, CloudRomStatus status,
    int32_t progress = 0, const std::string &installPath = "")
{
    return {{version, ""}, version, status, installPath, progress, {"", 0}};
}

inline CloudRomCallback CreateCloudRomCallback(const std::string &version,
    CloudRomStatus status, int32_t progress = 0)
{
    CloudRomCallback callbackData;
    callbackData.statusInfos.push_back(CreateStatusInfo(version, status, progress));
    return callbackData;
}

inline CloudRomNewVersionInfo CreateNewVersionInfo(const std::string &version,
    int64_t size = 1024000)
{
    return {{version, ""}, version, CloudRomStatus::DOWNLOAD_SUCCESS, size,
        SecurityLevel::DEFAULT, "", "", {"", 0}};
}

inline CloudRomCheckResult CreateCheckResult(const std::string &version, int64_t size = 1024000)
{
    CloudRomCheckResult result;
    result.newVersionInfos.push_back(CreateNewVersionInfo(version, size));
    result.errorInfo.errorNum = 0;
    result.errorInfo.message = "";
    return result;
}

inline StatusInfoResult CreateStatusInfoResult(const std::string &version,
    CloudRomStatus status = CloudRomStatus::INSTALL_SUCCESS, int32_t progress = 100,
    const std::string &installPath = "/data/rom")
{
    StatusInfoResult result;
    result.statusInfos.push_back(CreateStatusInfo(version, status, progress, installPath));
    result.errorInfo.errorNum = 0;
    result.errorInfo.message = "";
    return result;
}

inline FeatureInfo CreateFeatureInfo(const std::string &featureName)
{
    FeatureInfo info;
    info.featureName = featureName;
    info.info = "";
    return info;
}

inline CheckCondition CreateCheckCondition()
{
    CheckCondition condition;
    condition.language = "china";
    condition.condition = "";
    return condition;
}

inline DownloadCondition CreateDownloadCondition(bool isBackground = false)
{
    DownloadCondition condition;
    condition.allowedNetType = CLOUD_ROM_NETTYPE;
    condition.condition = "";
    condition.isBackground = isBackground;
    return condition;
}

inline PauseCondition CreatePauseCondition()
{
    PauseCondition condition;
    condition.reason = "";
    condition.autoResume = false;
    condition.condition = "";
    return condition;
}

inline BusinessError CreateBusinessError(int32_t errorNum = 0, const std::string &message = "")
{
    BusinessError error;
    error.errorNum = errorNum;
    error.message = message;
    return error;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // DOWNLOAD_UT_TEST_COMMON_H
