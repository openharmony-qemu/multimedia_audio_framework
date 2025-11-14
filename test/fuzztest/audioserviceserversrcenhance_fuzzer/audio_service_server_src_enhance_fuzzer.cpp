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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_service.h"
#include "ipc_stream_stub.h"
#include "ipc_stream_in_server.h"
#include "none_mix_engine.h"
#include "securec.h"
#include "audio_errors.h"
#include "audio_service_log.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const uint64_t COMMON_LENGTH_NUM = 2;
const uint32_t OPERATION_ENUM_NUM = 13;
const uint32_t SOURCETYPE_ENUM_NUM = 4;
const uint32_t NUM = 1;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(g_data) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

void AudioServiceMoreFuzzTest()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;

    AudioService *audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, audioServicePtr);
    std::shared_ptr<AudioProcessInServer> audioProcessInServer =
        std::make_shared<AudioProcessInServer>(config, audioServicePtr);
    audioProcessInServer->Release(true);
    audioProcessInServer->Release(false);

    uint32_t sessionId = GetData<uint32_t>();
    AudioPlaybackCaptureConfig newConfig;
    audioServicePtr->UpdateMuteControlSet(sessionId, true);
    audioServicePtr->UpdateMuteControlSet(sessionId, false);
    audioServicePtr->EnableDualStream(sessionId, "Speaker");
    audioServicePtr->OnCapturerFilterChange(sessionId, newConfig, 1);
    audioServicePtr->OnCapturerFilterRemove(sessionId, 1);

    int32_t ret = GetData<int32_t>();
    audioServicePtr->workingConfigs_[1];
    audioServicePtr->GetIpcStream(config, ret);
#ifdef HAS_FEATURE_INNERCAPTURER
    audioServicePtr->ShouldBeInnerCap(config, 1);
    audioServicePtr->ShouldBeDualTone(config, "Speaker");

    audioServicePtr->OnInitInnerCapList(1);
    audioServicePtr->OnUpdateInnerCapList(1);
#endif
    uint32_t sourceTypeInt = GetData<uint32_t>();
    sourceTypeInt = (sourceTypeInt % SOURCETYPE_ENUM_NUM) - NUM;
    SourceType sourceType = static_cast<SourceType>(sourceTypeInt);
    audioServicePtr->UpdateSourceType(sourceType);
}

void AudioCapturerInServerMoreFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::weak_ptr<IStreamListener> innerListener;
    std::shared_ptr<CapturerInServer> capturerInServer = std::make_shared<CapturerInServer>(config, innerListener);
    if (capturerInServer == nullptr) {
        return;
    }

    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = static_cast<RestoreReason>(GetData<int32_t>());
    restoreInfo.targetStreamFlag = GetData<int32_t>();
    uint32_t operationInt = GetData<uint32_t>();
    operationInt = (operationInt % OPERATION_ENUM_NUM) - NUM;
    IOperation operation = static_cast<IOperation>(operationInt);
    capturerInServer->OnStatusUpdate(operation);

    size_t length = COMMON_LENGTH_NUM;
    capturerInServer->ReadData(length);
#ifdef HAS_FEATURE_INNERCAPTURER
    AudioPlaybackCaptureConfig captureconfig;
    capturerInServer->UpdatePlaybackCaptureConfig(captureconfig);
#endif
    capturerInServer->SetNonInterruptMute(true);
    if (capturerInServer->isInited_ == true) {
        capturerInServer->RestoreSession(restoreInfo);
    }
}

void AudioNoneMixEngineMoreFuzzTest()
{
    std::shared_ptr<NoneMixEngine> noneMixEngine = std::make_shared<NoneMixEngine>();
    noneMixEngine->isInit_ = true;
    AudioDeviceDescriptor type(AudioDeviceDescriptor::DEVICE_INFO);
    type.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    noneMixEngine->Init(type, true);
    noneMixEngine->Start();

    noneMixEngine->isStart_ = true;
    noneMixEngine->Stop();
    noneMixEngine->PauseAsync();

    noneMixEngine->MixStreams();
    noneMixEngine->IsPlaybackEngineRunning();
    noneMixEngine->StandbySleep();

    std::vector<AudioSamplingRate> audioSamplingRate = {
        SAMPLE_RATE_16000,
        SAMPLE_RATE_48000,
    };
    uint32_t sourceTypeInt = GetData<uint32_t>();
    sourceTypeInt = sourceTypeInt % audioSamplingRate.size();
    AudioSamplingRate samplingRate = audioSamplingRate[sourceTypeInt];
    noneMixEngine->GetDirectVoipSampleRate(samplingRate);
}

typedef void (*TestFuncs[3])();

TestFuncs g_testFuncs = {
    AudioServiceMoreFuzzTest,
    AudioCapturerInServerMoreFuzzTest,
    AudioNoneMixEngineMoreFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}