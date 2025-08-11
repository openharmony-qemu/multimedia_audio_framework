/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <fstream>
#include <securec.h>

#include "audio_log.h"
#include "audio_collaborative_service.h"
#include "../fuzz_utils.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
static const std::string AUDIO_COLLABORATIVE_SERVICE_LABEL = "COLLABORATIVE";
static const std::string BLUETOOTH_EFFECT_CHAIN_NAME = "EFFECTCHAIN_COLLABORATIVE";
typedef void (*TestPtr)();

void AudioCollaborativeServiceIsCollaborativePlaybackSupportedFuzzTest()
{
    AudioCollaborativeService &audioCollaborativeService = AudioCollaborativeService::GetAudioCollaborativeService();
    std::vector<std::string> applyVec;
    EffectChain effectChain(BLUETOOTH_EFFECT_CHAIN_NAME, applyVec, AUDIO_COLLABORATIVE_SERVICE_LABEL);
    std::vector<EffectChain> effectChains;
    effectChains.push_back(effectChain);
    audioCollaborativeService.isCollaborativePlaybackSupported_ = g_fuzzUtils.GetData<bool>();
    audioCollaborativeService.Init(effectChains);
    audioCollaborativeService.IsCollaborativePlaybackSupported();
}

void AudioCollaborativeServiceUpdateCurrentDeviceFuzzTest()
{
    AudioCollaborativeService &audioCollaborativeService = AudioCollaborativeService::GetAudioCollaborativeService();
    AudioDeviceDescriptor selectedAudioDevice;
    selectedAudioDevice.macAddress_ = "8C-32-23-23-6C-12";
    audioCollaborativeService.curDeviceAddress_ = "test_address";
    selectedAudioDevice.deviceType_ = g_fuzzUtils.GetData<DeviceType>();
    bool isEnabled = g_fuzzUtils.GetData<bool>();
    if (isEnabled) {
        audioCollaborativeService.addressToCollaborativeEnabledMap_.insert(
            std::make_pair(selectedAudioDevice.macAddress_, g_fuzzUtils.GetData<bool>()));
    } else {
        audioCollaborativeService.curDeviceAddress_ = "";
        audioCollaborativeService.addressToCollaborativeEnabledMap_.clear();
        audioCollaborativeService.addressToCollaborativeMemoryMap_.insert(
            std::make_pair(selectedAudioDevice.macAddress_, g_fuzzUtils.GetData<bool>()));
    }
    audioCollaborativeService.UpdateCurrentDevice(selectedAudioDevice);
}

void AudioCollaborativeServiceSetCollaborativePlaybackEnabledForDeviceFuzzTest()
{
    AudioCollaborativeService &audioCollaborativeService = AudioCollaborativeService::GetAudioCollaborativeService();
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    bool enabled = g_fuzzUtils.GetData<bool>();
    audioCollaborativeService.SetCollaborativePlaybackEnabledForDevice(selectedAudioDevice, enabled);
}

void AudioCollaborativeServiceIsCollaborativePlaybackEnabledForDeviceFuzzTest()
{
    AudioCollaborativeService &audioCollaborativeService = AudioCollaborativeService::GetAudioCollaborativeService();
    std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    if (selectedAudioDevice == nullptr) {
        AUDIO_ERR_LOG("selectedAudioDevice is nullptr");
        return;
    }
    selectedAudioDevice->macAddress_ = "8C-32-23-23-6C-12";
    bool isEnabled = g_fuzzUtils.GetData<bool>();
    if (isEnabled) {
        audioCollaborativeService.addressToCollaborativeEnabledMap_.insert(
            std::make_pair(selectedAudioDevice->macAddress_, g_fuzzUtils.GetData<bool>()));
    } else {
        audioCollaborativeService.addressToCollaborativeEnabledMap_.clear();
    }
    audioCollaborativeService.IsCollaborativePlaybackEnabledForDevice(selectedAudioDevice);
}

void AudioCollaborativeServiceUpdateCollaborativeStateRealFuzzTest()
{
    AudioCollaborativeService &audioCollaborativeService = AudioCollaborativeService::GetAudioCollaborativeService();
    audioCollaborativeService.isCollaborativePlaybackSupported_ = g_fuzzUtils.GetData<bool>();
    audioCollaborativeService.isCollaborativeStateEnabled_ = g_fuzzUtils.GetData<bool>();
    audioCollaborativeService.curDeviceAddress_ = "8C-32-23-23-6C-12";
    bool isEnabled = g_fuzzUtils.GetData<bool>();
    if (isEnabled) {
        audioCollaborativeService.addressToCollaborativeEnabledMap_.insert(
            std::make_pair(audioCollaborativeService.curDeviceAddress_, g_fuzzUtils.GetData<bool>()));
    } else {
        audioCollaborativeService.addressToCollaborativeEnabledMap_.clear();
    }
    audioCollaborativeService.UpdateCollaborativeStateReal();
}

void AudioCollaborativeServiceIsCollaborativePlaybackSupportedSimpleFuzzTest()
{
    AudioCollaborativeService &audioCollaborativeService = AudioCollaborativeService::GetAudioCollaborativeService();
    audioCollaborativeService.isCollaborativePlaybackSupported_ = g_fuzzUtils.GetData<bool>();
    bool result = audioCollaborativeService.IsCollaborativePlaybackSupported();
}

vector<TestPtr> g_testPtrs = {
    AudioCollaborativeServiceIsCollaborativePlaybackSupportedFuzzTest,
    AudioCollaborativeServiceUpdateCurrentDeviceFuzzTest,
    AudioCollaborativeServiceSetCollaborativePlaybackEnabledForDeviceFuzzTest,
    AudioCollaborativeServiceIsCollaborativePlaybackEnabledForDeviceFuzzTest,
    AudioCollaborativeServiceUpdateCollaborativeStateRealFuzzTest,
    AudioCollaborativeServiceIsCollaborativePlaybackSupportedSimpleFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testPtrs);
    return 0;
}