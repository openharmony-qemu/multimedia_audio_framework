/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>
#include "pro_audio_stream_manager_fuzzer.h"
#include "audio_errors.h"
#include "policy_handler.h"
#include "../fuzz_utils.h"
namespace OHOS {
namespace AudioStandard {
using namespace std;
const int32_t CAPTURER_FLAG = 10;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const int32_t NUM_1 = 1;
static int32_t NUM_5 = 5;
typedef void (*TestFuncs)();

static AudioProcessConfig GetConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.originalSessionId = g_fuzzUtils.GetData<uint32_t>();
    return config;
}

void ProAudioStreamManagerFuzzTest::ProAudioStreamManagerFuzz()
{
    audioStreamManager_ = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    AudioProcessConfig config = GetConfig();
    shared_ptr<IRendererStream> rendererStream = audioStreamManager_->CreateRendererStream(config);
    audioStreamManager_->CreateRender(config, rendererStream);
    Funcs_.clear();
    Funcs_.push_back([=, this]() { audioStreamManager_->StartRender(config.originalSessionId); });
    Funcs_.push_back([=, this]() { audioStreamManager_->StopRender(config.originalSessionId); });
    Funcs_.push_back([=, this]() { audioStreamManager_->PauseRender(config.originalSessionId); });
    Funcs_.push_back([=, this]() { audioStreamManager_->TriggerStartIfNecessary(); });
    Funcs_.push_back([=, this]() { audioStreamManager_->GetStreamCount(); });
    Funcs_.push_back([=, this]() { audioStreamManager_->GetLatency(); });
    Funcs_.push_back([=, this]() {
        std::vector<SinkInput> sinkInputs;
        audioStreamManager_->GetAllSinkInputs(sinkInputs);
    });
    for (size_t i = 0; i < NUM_5; i++) {
        size_t idx = g_fuzzUtils.GetData<size_t>() % Funcs_.size();
        Funcs_[idx]();
    }
    audioStreamManager_->ReleaseRender(config.originalSessionId);
}

void ProAudioStreamManagerFuzz()
{
    ProAudioStreamManagerFuzzTest t;
    t.ProAudioStreamManagerFuzz();
}

void StartRenderFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    AudioProcessConfig config = GetConfig();
    std::shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    if (rendererStream == nullptr) {
        return;
    }
    int32_t rendererStreamMap = NUM_1;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    audioStreamManager->StartRender(streamIndex);
}

void StopRenderFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    AudioProcessConfig config = GetConfig();
    std::shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    if (rendererStream == nullptr) {
        return;
    }
    int32_t rendererStreamMap = NUM_1;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    audioStreamManager->StopRender(streamIndex);
}

void PauseRenderFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    AudioProcessConfig config = GetConfig();
    std::shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    if (rendererStream == nullptr) {
        return;
    }
    int32_t rendererStreamMap = NUM_1;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    bool isStandby = g_fuzzUtils.GetData<bool>();
    audioStreamManager->PauseRender(streamIndex, isStandby);
}

void ReleaseRenderFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    AudioProcessConfig config = GetConfig();
    std::shared_ptr<IRendererStream> rendererStream = audioStreamManager->CreateRendererStream(config);
    if (rendererStream == nullptr) {
        return;
    }
    int32_t rendererStreamMap = NUM_1;
    audioStreamManager->rendererStreamMap_.emplace(rendererStreamMap, rendererStream);

    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    audioStreamManager->ReleaseRender(streamIndex);
}

void CreateCapturerFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    AudioProcessConfig config = GetConfig();
    std::shared_ptr<ICapturerStream> capturerStream = nullptr;
    audioStreamManager->CreateCapturer(config, capturerStream);
}

void ReleaseCapturerFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    audioStreamManager->ReleaseCapturer(streamIndex);
}

void AddUnprocessStreamFuzz()
{
    std::shared_ptr<ProAudioStreamManager> audioStreamManager = make_shared<ProAudioStreamManager>(DIRECT_PLAYBACK);
    if (audioStreamManager == nullptr) {
        return;
    }
    int32_t appUid = g_fuzzUtils.GetData<int32_t>();
    audioStreamManager->AddUnprocessStream(appUid);
}

vector<TestFuncs> g_testFuncs = {
    ProAudioStreamManagerFuzz,
    StartRenderFuzz,
    StopRenderFuzz,
    PauseRenderFuzz,
    ReleaseRenderFuzz,
    CreateCapturerFuzz,
    ReleaseCapturerFuzz,
    AddUnprocessStreamFuzz,
};
} // namespace AudioStandard
} // namesapce OHOS


/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
