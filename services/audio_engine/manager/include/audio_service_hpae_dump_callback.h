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

#ifndef AUDIO_SERVICE_HPAE_DUMP_CALLBACK_H
#define AUDIO_SERVICE_HPAE_DUMP_CALLBACK_H
#include <string>
#include "audio_info.h"
#include "hpae_info.h"
namespace OHOS {
namespace AudioStandard {
typedef struct {
    std::string deviceName;
    std::string config;
} HpaeSinkSourceInfo;

typedef struct {
    uint32_t sessionId;
    std::string deviceName;
    int32_t uid;
    int32_t pid;
    uint32_t tokenId;
    bool offloadEnable;
    AudioPrivacyType privacyType;
    std::string config;
    HPAE::HpaeSessionState state;
    uint64_t startTime;
} HpaeInputOutputInfo;

typedef struct {
    std::vector<HpaeSinkSourceInfo> sinkInfos;
    std::vector<HpaeSinkSourceInfo> sourceInfos;
} HpaeDeviceInfo;

class AudioServiceHpaeDumpCallback {
public:
    virtual void OnDumpSinkInfoCb(std::string& dumpStr, int32_t result) = 0;
    virtual void OnDumpSourceInfoCb(std::string &dumpStr, int32_t result) = 0;
    virtual void OnDumpAllAvailableDeviceCb(int32_t result) = 0;
    virtual void OnDumpSinkInputsInfoCb(std::vector<HpaeInputOutputInfo> &sinkInputs, int32_t result) = 0;
    virtual void OnDumpSourceOutputsInfoCb(std::vector<HpaeInputOutputInfo> &sourceOutputs, int32_t result) = 0;
    virtual ~AudioServiceHpaeDumpCallback()
    {}
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_SERVICE_HPAE_DUMP_CALLBACK_H