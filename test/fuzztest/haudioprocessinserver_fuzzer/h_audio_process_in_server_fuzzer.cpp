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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "token_setproc.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "../fuzz_utils.h"

#include "audio_service.h"
#include "audio_process_in_server.h"
#include "audio_utils.h"
#include "record_privacy_manager.h"
using namespace std;
static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
constexpr int32_t INTELLIGENT_VOICE_UID = 1042;


FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appFullTokenId = 1;
    config.appInfo.appTokenId = 1;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}


void AudioProcessInServerExecute(FuzzedDataProvider &provider)
{
    AudioProcessConfig configRet = InitProcessConfig();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
    audioStreamInfo.channelLayout = CH_LAYOUT_STEREO;
    audioProcessInServer->ConfigProcessBuffer(totalSizeInFrame, spanSizeInFrame, audioStreamInfo);
    audioProcessInServer->Start();
    uint32_t sessionId =  provider.ConsumeIntegral<uint32_t>();
    audioProcessInServer->GetSessionId(sessionId);
    bool skipForce = provider.ConsumeBool();
    audioProcessInServer->SetDefaultOutputDevice(g_fuzzUtils.GetData<DeviceType>(), skipForce);
    int32_t tid = provider.ConsumeIntegral<int32_t>();
    std::string bundleName = provider.ConsumeRandomLengthString();
    int32_t method =  provider.ConsumeIntegralInRange<int32_t>(0, 2);
    audioProcessInServer->RegisterThreadPriority(tid, bundleName, method, THREAD_PRIORITY_QOS_7);
    bool on = provider.ConsumeBool();
    audioProcessInServer->SetSilentModeAndMixWithOthers(on);
    int64_t duration = provider.ConsumeIntegral<int64_t>();
    audioProcessInServer->SetSourceDuration(duration);
    uint32_t underrunCnt = provider.ConsumeIntegral<uint32_t>();
    audioProcessInServer->SetUnderrunCount(underrunCnt);
    float volume = provider.ConsumeFloatingPoint<float>();
    uint32_t sessionId1 = provider.ConsumeIntegral<uint32_t>();
    std::string adjustTime = provider.ConsumeRandomLengthString();
    uint32_t code = provider.ConsumeIntegral<uint32_t>();
    audioProcessInServer->SaveAdjustStreamVolumeInfo(volume, sessionId1, adjustTime, code);
    audioProcessInServer->SetRebuildFlag();
    bool keepRunning = provider.ConsumeBool();
    audioProcessInServer->GetServerKeepRunning(keepRunning);
    sptr<IRemoteObject> object = nullptr;
    audioProcessInServer->RegisterProcessCb(object);
    std::shared_ptr<OHAudioBufferBase> buffer;
    uint32_t reSpanSizeInFrame = provider.ConsumeIntegral<uint32_t>();
    audioProcessInServer->ResolveBufferBaseAndGetServerSpanSize(buffer, reSpanSizeInFrame);
    bool isFlush = provider.ConsumeBool();
    audioProcessInServer->Pause(isFlush);
    audioProcessInServer->Resume();
    int32_t stageIn = provider.ConsumeIntegralInRange<int32_t>(0, 1);
    audioProcessInServer->Stop(stageIn);
    audioProcessInServer->RequestHandleInfo();
    bool isSwitchStream = provider.ConsumeBool();
    audioProcessInServer->Release(isSwitchStream);
}

void AudioProcessInServerMicIndicatorExecute(FuzzedDataProvider &provider)
{
    AudioProcessConfig configRet = InitProcessConfig();
    configRet.callerUid = provider.ConsumeIntegral<int32_t>();
    constexpr SourceType sourceTypeList[] = {
        SOURCE_TYPE_MIC,
        SOURCE_TYPE_VOICE_COMMUNICATION,
        SOURCE_TYPE_PLAYBACK_CAPTURE,
        SOURCE_TYPE_REMOTE_CAST,
    };
    configRet.capturerInfo.sourceType = provider.PickValueInArray(sourceTypeList);
    configRet.appInfo.appTokenId = provider.ConsumeIntegral<uint32_t>();
    configRet.appInfo.appFullTokenId = provider.ConsumeIntegral<uint64_t>();
    AudioService *releaseCallbackRet = AudioService::GetInstance();
    sptr<AudioProcessInServer> audioProcessInServer = AudioProcessInServer::Create(configRet, releaseCallbackRet);
    if (audioProcessInServer == nullptr) {
        return;
    }

    (void)RecordPrivacyManager::GetInstance().NeedTurnOnMicIndicator(configRet.callerUid, false, SOURCE_TYPE_MIC);
    (void)RecordPrivacyManager::GetInstance().NeedTurnOnMicIndicator(INTELLIGENT_VOICE_UID, false,
        SOURCE_TYPE_MIC);
    (void)RecordPrivacyManager::GetInstance().NeedTurnOnMicIndicator(configRet.callerUid, false,
        SOURCE_TYPE_PLAYBACK_CAPTURE);
    (void)RecordPrivacyManager::GetInstance().NeedTurnOnMicIndicator(configRet.callerUid, false,
        SOURCE_TYPE_VOICE_CALL);
    (void)RecordPrivacyManager::GetInstance().NeedTurnOnMicIndicator(configRet.callerUid, true, SOURCE_TYPE_MIC);
    (void)RecordPrivacyManager::GetInstance().StartUsingMicrophone(configRet.appInfo.appTokenId);
    (void)RecordPrivacyManager::GetInstance().StartUsingMicrophone(configRet.appInfo.appTokenId);
    (void)RecordPrivacyManager::GetInstance().StopUsingMicrophone(configRet.appInfo.appTokenId);

    (void)audioProcessInServer->CheckBgRecordPermission(CAPTURER_RUNNING);

    SwitchStreamInfo info = {
        audioProcessInServer->GetSessionId(),
        configRet.callerUid,
        configRet.appInfo.appUid,
        configRet.appInfo.appPid,
        configRet.appInfo.appTokenId,
        CAPTURER_RUNNING,
    };
    (void)SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_WAITING);
    (void)SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_CREATED);
    (void)audioProcessInServer->CheckBgRecordPermission(CAPTURER_RUNNING);

    audioProcessInServer->isMicIndicatorOn_ = provider.ConsumeBool();
    (void)audioProcessInServer->TurnOnMicIndicator();
    (void)audioProcessInServer->TurnOffMicIndicator(CAPTURER_RELEASED);

    auto &manager = RecordPrivacyManager::GetInstance();
    manager.tokenIdRecordMap_.clear();
    AudioProcessConfig stopFailConfig = InitProcessConfig();
    stopFailConfig.appInfo.appTokenId = 0;
    sptr<AudioProcessInServer> stopFailProcess = AudioProcessInServer::Create(stopFailConfig, releaseCallbackRet);
    if (stopFailProcess != nullptr) {
        stopFailProcess->isMicIndicatorOn_ = true;
        manager.tokenIdRecordMap_[0] = {stopFailProcess->GetSessionId()};
        (void)stopFailProcess->TurnOffMicIndicator(CAPTURER_PAUSED);
        (void)stopFailProcess->TurnOnMicIndicator();
    }
    manager.tokenIdRecordMap_.clear();
}

void FuzzTest(FuzzedDataProvider &provider)
{
    auto func = provider.PickValueInArray({
        AudioProcessInServerExecute,
        AudioProcessInServerMicIndicatorExecute,
    });
    func(provider);
}
}
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    if (SetSelfTokenID(718336240uLL | (1uLL << NUM_32)) < 0) {
        return -1;
    }
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}
