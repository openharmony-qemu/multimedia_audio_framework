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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_spatialization_service.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
typedef void (*TestFuncs)();

void AudioInterruptZoneManagerGetAudioFocusInfoListFuzzTest()
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    if (audioInterruptZoneManager == nullptr) {
        return;
    }
    AudioInterruptService service;
    audioInterruptZoneManager->service_ = &service;
    if (audioInterruptZoneManager->service_ == nullptr) {
        return;
    }
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    audioInterruptZoneManager->service_->zonesMap_.insert({zoneId, audioInterruptZone});
    std::string deviceTag = "0";
    AudioFocusList focusInfoList;
    audioInterruptZoneManager->GetAudioFocusInfoList(zoneId, deviceTag, focusInfoList);
}

void AudioInterruptZoneManagerForceStopAudioFocusInZoneFuzzTest()
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    if (audioInterruptZoneManager == nullptr) {
        return;
    }
    AudioInterruptService service;
    audioInterruptZoneManager->service_ = &service;
    if (audioInterruptZoneManager->service_ == nullptr) {
        return;
    }

    audioInterruptZoneManager->service_->handler_ = std::make_shared<AudioPolicyServerHandler>();
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    AudioInterrupt interrupt;
    interrupt.streamId = g_fuzzUtils.GetData<uint32_t>();
    interrupt.pid = g_fuzzUtils.GetData<int32_t>();
    audioInterruptZoneManager->ForceStopAudioFocusInZone(zoneId, interrupt);
}

void AudioInterruptZoneManagerForceStopAllAudioFocusInZoneFuzzTest()
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    if (audioInterruptZoneManager == nullptr || zone == nullptr) {
        return;
    }
    AudioInterruptService service;
    audioInterruptZoneManager->service_ = &service;
    AudioInterrupt interrupt;
    AudioFocuState state = ACTIVE;
    AudioFocuState stateByGetData = g_fuzzUtils.GetData<AudioFocuState>();
    zone->audioFocusInfoList.push_back(std::make_pair(interrupt, state));
    zone->audioFocusInfoList.push_back(std::make_pair(interrupt, stateByGetData));

    audioInterruptZoneManager->ForceStopAllAudioFocusInZone(zone);
}

void AudioInterruptZoneManagerInjectInterruptToAudioZoneFuzzTest()
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    if (audioInterruptZoneManager == nullptr) {
        return;
    }
    AudioInterruptService service;
    audioInterruptZoneManager->service_ = &service;
    if (audioInterruptZoneManager->service_ == nullptr) {
        return;
    }
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    audioInterruptZoneManager->service_->zonesMap_.insert({zoneId, std::make_shared<AudioInterruptZone>()});
    std::string deviceTag = "test_device_tag";
    AudioInterrupt interrupt;
    AudioFocuState stateByGetData = g_fuzzUtils.GetData<AudioFocuState>();
    AudioFocusList interrupts;
    interrupts.push_back(std::make_pair(interrupt, stateByGetData));
    audioInterruptZoneManager->InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
}

void AudioInterruptZoneManagerQueryAudioFocusFromZoneFuzzTest()
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    if (audioInterruptZoneManager == nullptr || audioInterruptZone == nullptr) {
        return;
    }
    AudioInterruptService service;
    audioInterruptZoneManager->service_ = &service;
    if (audioInterruptZoneManager->service_ == nullptr) {
        return;
    }
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    std::string deviceTag = "test_device_tag";
    AudioInterrupt interrupt;
    interrupt.deviceTag = deviceTag;
    AudioFocuState stateByGetData = g_fuzzUtils.GetData<AudioFocuState>();
    audioInterruptZone->audioFocusInfoList.push_back(std::make_pair(interrupt, stateByGetData));
    audioInterruptZoneManager->service_->zonesMap_.insert({zoneId, audioInterruptZone});

    audioInterruptZoneManager->QueryAudioFocusFromZone(zoneId, deviceTag);
}

void AudioInterruptZoneManagerTryActiveAudioFocusForZoneFuzzTest()
{
    auto audioInterruptZoneManager = std::make_shared<AudioInterruptZoneManager>();
    shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    if (audioInterruptZoneManager == nullptr || audioInterruptZone == nullptr) {
        return;
    }
    AudioInterruptService service;
    audioInterruptZoneManager->service_ = &service;
    if (audioInterruptZoneManager->service_ == nullptr) {
        return;
    }
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    AudioInterrupt interrupt;
    AudioFocuState stateByGetData = g_fuzzUtils.GetData<AudioFocuState>();
    audioInterruptZone->audioFocusInfoList.push_back(std::make_pair(interrupt, stateByGetData));
    audioInterruptZoneManager->service_->zonesMap_.insert({zoneId, audioInterruptZone});
    AudioFocusList activeFocusList;
    activeFocusList.push_back(std::make_pair(interrupt, stateByGetData));
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        activeFocusList.clear();
    }

    audioInterruptZoneManager->TryActiveAudioFocusForZone(zoneId, activeFocusList);
}

vector<TestFuncs> g_testFuncs = {
    AudioInterruptZoneManagerGetAudioFocusInfoListFuzzTest,
    AudioInterruptZoneManagerForceStopAudioFocusInZoneFuzzTest,
    AudioInterruptZoneManagerForceStopAllAudioFocusInZoneFuzzTest,
    AudioInterruptZoneManagerInjectInterruptToAudioZoneFuzzTest,
    AudioInterruptZoneManagerQueryAudioFocusFromZoneFuzzTest,
    AudioInterruptZoneManagerTryActiveAudioFocusForZoneFuzzTest,
};
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
