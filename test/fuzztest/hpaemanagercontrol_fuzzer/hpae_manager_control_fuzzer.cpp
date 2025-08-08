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
#include "hpae_manager_control_fuzzer.h"

#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include "audio_errors.h"
#include "test_case_common.h"
#include "hpae_audio_service_dump_callback_unit_test.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
using namespace OHOS::AudioStandard::HPAE;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 9;
static int32_t NUM_2 = 2;
static std::string g_rootPath = "/data/";
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;

typedef void (*TestFuncs)();

vector<AudioSpatializationSceneType> AudioSpatializationSceneTypeVec {
    SPATIALIZATION_SCENE_TYPE_DEFAULT,
    SPATIALIZATION_SCENE_TYPE_MUSIC,
    SPATIALIZATION_SCENE_TYPE_MOVIE,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
    SPATIALIZATION_SCENE_TYPE_MAX,
};

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

vector<HpaeStreamClassType> HpaeStreamClassTypeVec = {
    HPAE_STREAM_CLASS_TYPE_INVALID,
    HPAE_STREAM_CLASS_TYPE_PLAY,
    HPAE_STREAM_CLASS_TYPE_RECORD,
};

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

AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

void WaitForMsgProcessing(std::shared_ptr<HPAE::HpaeManager> &hpaeManager)
{
    while (hpaeManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
}

HPAE::HpaeStreamInfo GetRenderStreamInfo()
{
    HPAE::HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

AudioModuleInfo GetSourceAudioModeInfo(std::string name = "mic")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + "source_" + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_FILE_SOURCE);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

HPAE::HpaeStreamInfo GetCaptureStreamInfo()
{
    HPAE::HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_RECORD;
    return streamInfo;
}

void HpaeRenderManagerReloadTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();

    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();

    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    callback->GetCloseAudioPortResult();

    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeRenderManagerReloadTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();

    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();

    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    callback->GetCloseAudioPortResult();

    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerGetSinkAndSourceInfoTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    HpaeSinkInfo sinkInfo;
    HpaeSourceInfo sourceInfo;
    int32_t ret = -1;
    hpaeManager_->GetSinkInfoByIdx(0, [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
        sinkInfo = sinkInfoRet;
        ret = result;
    });
    WaitForMsgProcessing(hpaeManager_);

    ret = -1;
    hpaeManager_->GetSourceInfoByIdx(0, [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
        sourceInfo  = sourceInfoRet;
        ret = result;
    });

    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();
    ret = -1;
    hpaeManager_->GetSinkInfoByIdx(portId, [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
        sinkInfo = sinkInfoRet;
        ret = result;
    });
    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo2 = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo2);
    WaitForMsgProcessing(hpaeManager_);
    portId = callback->GetPortId();
    ret = -1;
    hpaeManager_->GetSourceInfoByIdx(portId, [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
        sourceInfo  = sourceInfoRet;
        ret = result;
    });
    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerGetSinkAndSourceInfoTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    HpaeSinkInfo sinkInfo;
    HpaeSourceInfo sourceInfo;
    int32_t ret = -1;
    hpaeManager_->GetSinkInfoByIdx(0, [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
        sinkInfo = sinkInfoRet;
        ret = result;
    });
    WaitForMsgProcessing(hpaeManager_);

    ret = -1;
    hpaeManager_->GetSourceInfoByIdx(0, [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
        sourceInfo  = sourceInfoRet;
        ret = result;
    });

    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = GetData<int32_t>();
    ret = -1;
    hpaeManager_->GetSinkInfoByIdx(portId, [&sinkInfo, &ret](const HpaeSinkInfo &sinkInfoRet, int32_t result) {
        sinkInfo = sinkInfoRet;
        ret = result;
    });
    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);

    AudioModuleInfo audioModuleInfo2 = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo2);
    WaitForMsgProcessing(hpaeManager_);
    portId = GetData<int32_t>();
    ret = -1;
    hpaeManager_->GetSourceInfoByIdx(portId, [&sourceInfo, &ret](const HpaeSourceInfo &sourceInfoRet, int32_t result) {
        sourceInfo  = sourceInfoRet;
        ret = result;
    });
    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerEffectLiveTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->NotifySettingsDataReady();
    hpaeManager_->NotifyAccountsChanged();
    hpaeManager_->IsAcousticEchoCancelerSupported(SOURCE_TYPE_LIVE);
    std::vector<std::string> subKeys;
    std::vector<std::pair<std::string, std::string>> result;
    subKeys.push_back("live_effect_supported");
    hpaeManager_->GetEffectLiveParameter(subKeys, result);
    std::vector<std::pair<std::string, std::string>> params;
    params.push_back({"live_effect_enable", "NRON"});
    hpaeManager_->SetEffectLiveParameter(params);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerEffectTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    HpaeStreamInfo streamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeManager_);
   
    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->SetRate(streamInfo.sessionId, RENDER_RATE_DOUBLE);

    int32_t effectMode = 0;
    hpaeManager_->GetAudioEffectMode(streamInfo.sessionId, effectMode);
    int32_t privacyType = 0;
    hpaeManager_->SetPrivacyType(streamInfo.sessionId, privacyType);
    hpaeManager_->GetPrivacyType(streamInfo.sessionId, privacyType);
    hpaeManager_->GetWritableSize(streamInfo.sessionId);
    hpaeManager_->UpdateSpatializationState(streamInfo.sessionId + 1, true, false);
    hpaeManager_->UpdateSpatializationState(streamInfo.sessionId, true, false);
    hpaeManager_->UpdateMaxLength(streamInfo.sessionId, TEST_SLEEP_TIME_20);
    hpaeManager_->SetOffloadRenderCallbackType(streamInfo.sessionId, CB_FLUSH_COMPLETED);

    hpaeManager_->Release(streamInfo.streamClassType, streamInfo.sessionId);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->GetSessionInfo(streamInfo.streamClassType, streamInfo.sessionId, sessionInfo);
    hpaeManager_->CloseOutAudioPort("Speaker_File1");
    hpaeManager_->CloseOutAudioPort("Speaker_File");
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerEffectTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    hpaeManager_->OpenAudioPort(audioModuleInfo1);

    AudioSpatializationState stateInfo;
    stateInfo.headTrackingEnabled = false;
    stateInfo.spatializationEnabled = false;
    hpaeManager_->UpdateSpatializationState(stateInfo);
    hpaeManager_->UpdateSpatialDeviceType(EARPHONE_TYPE_INEAR);
    
    AudioEffectPropertyArrayV3 propertyV3;
    hpaeManager_->GetAudioEffectProperty(propertyV3);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->SetAudioEffectProperty(propertyV3);
    
    AudioEffectPropertyArray property;
    hpaeManager_->GetAudioEffectProperty(property);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->UpdateEffectBtOffloadSupported(true);
    hpaeManager_->SetOutputDevice(TEST_STREAM_SESSION_ID, DEVICE_TYPE_SPEAKER);

    hpaeManager_->SetMicrophoneMuteInfo(false);

    hpaeManager_->GetAudioEnhanceProperty(propertyV3, DEVICE_TYPE_SPEAKER);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->SetAudioEnhanceProperty(propertyV3, DEVICE_TYPE_SPEAKER);
    
    AudioEnhancePropertyArray propertyEn;
    hpaeManager_->GetAudioEnhanceProperty(propertyEn, DEVICE_TYPE_SPEAKER);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->SetAudioEnhanceProperty(propertyEn, DEVICE_TYPE_SPEAKER);
    
    hpaeManager_->UpdateExtraSceneType("123", "456", "789");
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void GetAllSinksFuzzTest()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->ReloadRenderManager(audioModuleInfo);
    hpaeManager_->DumpSinkInfo(audioModuleInfo.name);
    hpaeManager_->DumpSinkInfo("virtual1");
    WaitForMsgProcessing(hpaeManager_);

    uint32_t sinkSourceIndex = GetData<uint32_t>();
    hpaeManager_->OpenVirtualAudioPort(audioModuleInfo, sinkSourceIndex);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo("Speaker_File1");
    hpaeManager_->OpenVirtualAudioPort(audioModuleInfo1, sinkSourceIndex);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->GetAllSinks();

    audioModuleInfo1 = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo1);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->DumpSourceInfo(audioModuleInfo1.name);
    hpaeManager_->DumpSourceInfo("virtual1");

    HpaeDeviceInfo devicesInfo_;
    hpaeManager_->DumpAllAvailableDevice(devicesInfo_);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

void GetAllSinksFuzzTest2()
{
    std::shared_ptr<HPAE::HpaeManager> hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    hpaeManager_->SetDefaultSink(audioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->ReloadRenderManager(audioModuleInfo);
    hpaeManager_->DumpSinkInfo(audioModuleInfo.name);
    hpaeManager_->DumpSinkInfo("virtual1");
    WaitForMsgProcessing(hpaeManager_);
    uint32_t sinkSourceIndex = GetData<uint32_t>();
    hpaeManager_->OpenVirtualAudioPort(audioModuleInfo, sinkSourceIndex);
    AudioModuleInfo audioModuleInfo1 = GetSinkAudioModeInfo();
    hpaeManager_->OpenVirtualAudioPort(audioModuleInfo1, sinkSourceIndex);
    WaitForMsgProcessing(hpaeManager_);

    hpaeManager_->GetAllSinks();
    WaitForMsgProcessing(hpaeManager_);

    audioModuleInfo1 = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo1);

    hpaeManager_->DumpSourceInfo(audioModuleInfo1.name);
    hpaeManager_->DumpSourceInfo("virtual1");

    HpaeDeviceInfo devicesInfo_;
    hpaeManager_->DumpAllAvailableDevice(devicesInfo_);
    WaitForMsgProcessing(hpaeManager_);
    hpaeManager_->DeInit();
    WaitForMsgProcessing(hpaeManager_);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    HpaeRenderManagerReloadTest,
    HpaeRenderManagerReloadTest2,
    HpaeManagerGetSinkAndSourceInfoTest,
    HpaeManagerGetSinkAndSourceInfoTest2,
    HpaeManagerEffectLiveTest,
    HpaeManagerEffectTest,
    HpaeManagerEffectTest2,
    GetAllSinksFuzzTest,
    GetAllSinksFuzzTest2,
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
