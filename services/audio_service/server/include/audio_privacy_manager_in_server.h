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

#ifndef AUDIO_PRIVACY_MANAGER_IN_SERVER_H
#define AUDIO_PRIVACY_MANAGER_IN_SERVER_H

#include <cstdint>
#include <string>
#include <mutex>

namespace OHOS {
namespace AudioStandard {
class AudioPrivacyManagerInServer {
public:
    static AudioPrivacyManagerInServer &GetInstance();

    bool StartUsingMicrophone(uint32_t targetTokenId);
    bool StopUsingMicrophone(uint32_t targetTokenId);
    bool StartUsingMicrophone(const std::string &ancoPkgName);
    bool StopUsingMicrophone(const std::string &ancoPkgName);
    bool UpdateActiveAncoCapturer(const std::vector<std::string> &bundleNameList);
    bool GetStartMicResult() const {return startUsingMicFlag_;};

private:
    AudioPrivacyManagerInServer() = default;
    ~AudioPrivacyManagerInServer() = default;
    AudioPrivacyManagerInServer(const AudioPrivacyManagerInServer &) = delete;
    AudioPrivacyManagerInServer &operator=(const AudioPrivacyManagerInServer &) = delete;

    std::mutex activeCaptureMutex_;
    std::set<std::string> lastAncoBundlenameList_;
    bool startUsingMicFlag_ = false;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_PRIVACY_MANAGER_IN_SERVER_H
