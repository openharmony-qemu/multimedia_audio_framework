/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <memory>
#include "audio_spatialization_service.h"
#include "audio_policy_server.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t LIMITSIZE = 4;
const uint32_t STREAMUSAGE_ENUM_NUM = 23;
const uint32_t NUM = 1;

static std::shared_ptr<AudioSpatializationService> g_audioSpatializationService =
    std::make_shared<AudioSpatializationService>();

void AudioSpatializationServiceFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioSpatialDeviceState audioSpatialDeviceState;
    g_audioSpatializationService->UpdateSpatialDeviceState(audioSpatialDeviceState);
    g_audioSpatializationService->Deinit();

    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    g_audioSpatializationService->SetHeadTrackingEnabled(selectedAudioDevice, true);
    g_audioSpatializationService->HandleHeadTrackingEnabledChange(selectedAudioDevice, true);

    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t streamUsageInt = *reinterpret_cast<const uint32_t*>(rawData);
    streamUsageInt = (streamUsageInt % STREAMUSAGE_ENUM_NUM) - NUM;
    StreamUsage streamUsage = static_cast<StreamUsage>(streamUsageInt);

    g_audioSpatializationService->GetSpatializationSceneType();
    g_audioSpatializationService->UpdateSpatializationState();
    g_audioSpatializationService->InitSpatializationState();
    g_audioSpatializationService->RemoveOldestDevice();
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioSpatializationServiceFuzzTest(rawData, size);
    return 0;
}