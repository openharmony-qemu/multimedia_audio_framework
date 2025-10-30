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
#include "hpae_manager_stream_fuzzer.h"

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
static int32_t NUM_2 = 2;
static std::string g_rootPath = "/data/";
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t FRAME_LENGTH = 882;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_RENDER_SESSION_ID = 123457;
constexpr int32_t TEST_CAP_SESSION_ID = 123458;
bool g_isFirst = false;
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

vector<EffectChain> DEFAULT_EFFECT_CHAINS = {
    {"EFFECTCHAIN_SPK_MUSIC", {"apply1", "apply2", "apply3"}, ""},
    {"EFFECTCHAIN_BT_MUSIC", {}, ""}
};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};

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
    int waitCount = 0;
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
    streamInfo.sessionId = TEST_RENDER_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

HPAE::HpaeStreamInfo GetCapStreamInfo()
{
    HPAE::HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH;
    streamInfo.sessionId = TEST_CAP_SESSION_ID;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    streamInfo.streamClassType = HPAE::HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.deviceName = "Speaker_File";
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
void HpaeManagerFuzzTest::Fisrt()
{
    CHECK_AND_RETURN(!g_isFirst);
    g_isFirst = true;
    hpaeManager_->GetAllSinkInputs();
    hpaeManager_->GetAllSinks();
    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(streamInfo_.streamClassType, streamInfo_.sessionId, sessionInfo);
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
    hpaeManager_->SetSourceOutputMute(sourcePortId_, true);
    hpaeManager_->GetAllSourceOutputs();
    hpaeManager_->DumpSourceOutputsInfo();
    hpaeManager_->DumpSinkInputsInfo();
    AudioEffectPropertyArrayV3 propertyV3;
    hpaeManager_->GetAudioEffectProperty(propertyV3);
    hpaeManager_->SetAudioEffectProperty(propertyV3);
    AudioEffectPropertyArray property;
    hpaeManager_->GetAudioEffectProperty(property);
    hpaeManager_->SetAudioEffectProperty(property);
    AudioEnhancePropertyArray propertyEn;
    hpaeManager_->GetAudioEnhanceProperty(propertyEn, DEVICE_TYPE_SPEAKER);
    hpaeManager_->SetAudioEnhanceProperty(propertyEn, DEVICE_TYPE_SPEAKER);
    hpaeManager_->GetAudioEnhanceProperty(propertyV3, DEVICE_TYPE_SPEAKER);
    hpaeManager_->SetAudioEnhanceProperty(propertyV3, DEVICE_TYPE_SPEAKER);
    hpaeManager_->UpdateExtraSceneType("123", "456", "789");
    hpaeManager_->UpdateSpatialDeviceType(EARPHONE_TYPE_INEAR);
    hpaeManager_->SetOutputDevice(TEST_STREAM_SESSION_ID, DEVICE_TYPE_SPEAKER);
    hpaeManager_->InitHdiState();
    hpaeManager_->AddStreamVolumeToEffect("123", 1.0);
    hpaeManager_->DeleteStreamVolumeToEffect("123");
    hpaeManager_->SetStreamVolumeInfo(streamInfo_.sessionId, 1.0);
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerFuzzTest::StreamSetUp()
{
    hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    hpaeManager_->IsInit();

    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);

    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(sinkAudioModuleInfo);
    hpaeManager_->SetDefaultSink(sinkAudioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    sinkPortId_ = callback->GetPortId();

    AudioModuleInfo sinkAudioModuleInfo2 = GetSinkAudioModeInfo("Speaker_File1");
    hpaeManager_->OpenAudioPort(sinkAudioModuleInfo2);
    hpaeManager_->SetDefaultSink(sinkAudioModuleInfo2.name);
    WaitForMsgProcessing(hpaeManager_);
    sinkPortId2_ = callback->GetPortId();

    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(sourceAudioModuleInfo);
    hpaeManager_->SetDefaultSource(sourceAudioModuleInfo.name);
    WaitForMsgProcessing(hpaeManager_);
    sourcePortId_ = callback->GetPortId();

    AudioModuleInfo sourceAudioModuleInfo2 = GetSourceAudioModeInfo("mic1");
    hpaeManager_->OpenAudioPort(sourceAudioModuleInfo2);
    hpaeManager_->SetDefaultSource(sourceAudioModuleInfo2.name);
    WaitForMsgProcessing(hpaeManager_);
    sourcePortId2_ = callback->GetPortId();
    rendererStreamInfo_ = GetRenderStreamInfo();
    hpaeManager_->CreateStream(rendererStreamInfo_);
    streamInfo_ = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo_);
    capStreamInfo_ = GetCapStreamInfo();
    hpaeManager_->CreateStream(capStreamInfo_);
    WaitForMsgProcessing(hpaeManager_);
    Fisrt();
    InitFunc();
}
void HpaeManagerFuzzTest::InitStreamFunc()
{
    renderStreamFunc_.clear();
    renderStreamFunc_.push_back([=, this]() {
        hpaeManager_->Start(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId);
    });
    renderStreamFunc_.push_back([=, this]() {
        hpaeManager_->StartWithSyncId(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId, 1);
    });
    renderStreamFunc_.push_back([=, this]() {
        hpaeManager_->Pause(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId);
    });
    renderStreamFunc_.push_back([=, this]() {
        hpaeManager_->Flush(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId);
    });
    renderStreamFunc_.push_back([=, this]() {
        hpaeManager_->Drain(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId);
    });
    renderStreamFunc_.push_back([=, this]() {
        hpaeManager_->Stop(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId);
    });

    capturerStreamFunc_.clear();
    capturerStreamFunc_.push_back([=, this]() {
        hpaeManager_->Start(streamInfo_.streamClassType, streamInfo_.sessionId);
    });
    capturerStreamFunc_.push_back([=, this]() {
        hpaeManager_->StartWithSyncId(streamInfo_.streamClassType, streamInfo_.sessionId, 1);
    });
    capturerStreamFunc_.push_back([=, this]() {
        hpaeManager_->Pause(streamInfo_.streamClassType, streamInfo_.sessionId);
    });
    capturerStreamFunc_.push_back([=, this]() {
        hpaeManager_->Flush(streamInfo_.streamClassType, streamInfo_.sessionId);
    });
    capturerStreamFunc_.push_back([=, this]() {
        hpaeManager_->Drain(streamInfo_.streamClassType, streamInfo_.sessionId);
    });
    capturerStreamFunc_.push_back([=, this]() {
        hpaeManager_->Stop(streamInfo_.streamClassType, streamInfo_.sessionId);
    });
}

void HpaeManagerFuzzTest::InitFunc()
{
    sourceOutputIdList_ = {streamInfo_.sessionId, GetData<uint32_t>()};
    sinkInputIdList_ = {rendererStreamInfo_.sessionId, GetData<uint32_t>()};
    sourceNameList_ = {"mic", "mic1", "test"};
    sinkNameList_ = {"Speaker_File", "Speaker_File1", "test", ""};
    sessionIdList_ = {streamInfo_.sessionId, rendererStreamInfo_.sessionId, GetData<uint32_t>()};
    HpaeSessionInfo sessionInfo;
    hpaeManager_->GetSessionInfo(rendererStreamInfo_.streamClassType, rendererStreamInfo_.sessionId, sessionInfo);
    hpaeManager_->GetSessionInfo(streamInfo_.streamClassType, streamInfo_.sessionId, sessionInfo);

    InitStreamFunc();

    moveStreamFunc_.clear();
    uint32_t sessionId = sinkInputIdList_[GetData<uint32_t>() % sinkInputIdList_.size()];
    std::string sinkName = sinkNameList_[GetData<uint32_t>() % sinkNameList_.size()];
    moveStreamFunc_.push_back([=, this]() { hpaeManager_->MoveSinkInputByIndexOrName(sessionId, 0, sinkName); });
    sessionId = sourceOutputIdList_[GetData<uint32_t>() % sourceOutputIdList_.size()];
    std::string sourceName = sourceNameList_[GetData<uint32_t>() % sourceNameList_.size()];
    moveStreamFunc_.push_back([=, this]() { hpaeManager_->MoveSourceOutputByIndexOrName(sessionId, 0, sourceName); });

    errorStreamFunc_.clear();
    
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    sessionId = GetData<uint32_t>();
    errorStreamFunc_.push_back([=, this]() { hpaeManager_->Start(streamClassType, sessionId); });
    errorStreamFunc_.push_back([=, this]() { hpaeManager_->StartWithSyncId(streamClassType, sessionId, 1); });
    errorStreamFunc_.push_back([=, this]() { hpaeManager_->Pause(streamClassType, sessionId); });
    errorStreamFunc_.push_back([=, this]() { hpaeManager_->Flush(streamClassType, sessionId); });
    errorStreamFunc_.push_back([=, this]() { hpaeManager_->Drain(streamClassType, sessionId); });
    errorStreamFunc_.push_back([=, this]() { hpaeManager_->Stop(streamClassType, sessionId); });
}

void HpaeManagerFuzzTest::TearDown()
{
    hpaeManager_->CloseAudioPort(sinkPortId_);
    hpaeManager_->CloseAudioPort(sinkPortId2_);
    hpaeManager_->CloseAudioPort(sourcePortId_);
    hpaeManager_->CloseAudioPort(sourcePortId2_);
    hpaeManager_->DeInit();
}

void HpaeManagerFuzzTest::DumpFuzzTest()
{
    StreamSetUp();
    std::string sinkName = sinkNameList_[GetData<uint32_t>() % sinkNameList_.size()];
    hpaeManager_->DumpSinkInfo(sinkName);
    std::string sourceName = sourceNameList_[GetData<uint32_t>() % sourceNameList_.size()];
    hpaeManager_->DumpSourceInfo(sourceName);
    HpaeDeviceInfo devicesInfo_;
    hpaeManager_->DumpAllAvailableDevice(devicesInfo_);
    hpaeManager_->DumpSourceInfo(sourceName);
    bool mute = GetData<bool>();
    bool isSync = GetData<bool>();
    hpaeManager_->SetSinkMute(sinkName, mute, isSync);
    bool isSuspend = GetData<bool>();
    hpaeManager_->SuspendAudioDevice(sinkName, isSuspend);
    uint32_t index = GetData<uint32_t>() % HpaeStreamClassTypeVec.size();
    HpaeStreamClassType streamClassType = HpaeStreamClassTypeVec[index];
    uint32_t sessionId = sessionIdList_[GetData<uint32_t>() % sessionIdList_.size()];
    hpaeManager_->ShouldNotSkipProcess(streamClassType, sessionId);
    TearDown();
}

void HpaeManagerFuzzTest::StreamManagerFuzzTest()
{
    StreamSetUp();
    for (size_t i = 0; i < renderStreamFunc_.size() + 1; i++) {
        uint32_t index = GetData<uint32_t>() % renderStreamFunc_.size();
        renderStreamFunc_[index]();
        index = GetData<uint32_t>() % capturerStreamFunc_.size();
        capturerStreamFunc_[index]();
        index = GetData<uint32_t>() % errorStreamFunc_.size();
        errorStreamFunc_[index]();
    }
    TearDown();
}

void HpaeManagerFuzzTest::MoveStreamManagerFuzzTest()
{
    StreamSetUp();
    for (size_t i = 0; i < renderStreamFunc_.size() + 1; i++) {
        uint32_t index = GetData<uint32_t>() % moveStreamFunc_.size();
        moveStreamFunc_[index]();
        index = GetData<uint32_t>() % renderStreamFunc_.size();
        renderStreamFunc_[index]();
        index = GetData<uint32_t>() % capturerStreamFunc_.size();
        capturerStreamFunc_[index]();
        index = GetData<uint32_t>() % errorStreamFunc_.size();
        errorStreamFunc_[index]();
    }
    TearDown();
}

void HpaeManagerFuzzTest::HpaeManagerEffectTest()
{
    StreamSetUp();
    std::vector<EffectChain> enhanceChains = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
    EffectChainManagerParam managerParam;
    std::vector<std::shared_ptr<AudioEffectLibEntry>> enhanceLibraryList;
    hpaeManager_->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList);
    hpaeManager_->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_EFFECT_CHAIN_MANAGER_PARAM,
        DEFAULT_EFFECT_LIBRARY_LIST);

    effectFunc_.clear();
    uint32_t sessionId = sessionIdList_[GetData<uint32_t>() % sessionIdList_.size()];
    int32_t value = GetData<int32_t>();
    effectFunc_.push_back([=, this]() { hpaeManager_->SetRate(sessionId, value); });
    effectFunc_.push_back([&, this]() { hpaeManager_->GetAudioEffectMode(sessionId, value); });
    effectFunc_.push_back([=, this]() { hpaeManager_->SetPrivacyType(sessionId, value); });
    effectFunc_.push_back([=, this]() { hpaeManager_->GetWritableSize(sessionId); });

    effectFunc_.push_back([=, this]() { hpaeManager_->UpdateMaxLength(sessionId, TEST_SLEEP_TIME_20); });
    effectFunc_.push_back([=, this]() { hpaeManager_->SetOffloadRenderCallbackType(sessionId, value); });
    effectFunc_.push_back([=, this]() { hpaeManager_->SetOffloadPolicy(sessionId, value); });
    effectFunc_.push_back([=, this]() { hpaeManager_->SetSpeed(sessionId, value); });
    for (size_t i = 0; i < NUM_2; i++) {
        uint32_t index = GetData<uint32_t>() % effectFunc_.size();
        effectFunc_[index]();
    }
    TearDown();
}

void HpaeManagerFuzzTest::HpaeManagerEffectTest2()
{
    StreamSetUp();
    effectFunc_.clear();
    uint32_t sessionId = sessionIdList_[GetData<uint32_t>() % sessionIdList_.size()];
    bool value1 = GetData<bool>();
    bool value2 = GetData<bool>();
    effectFunc_.push_back([=, this]() { hpaeManager_->UpdateSpatializationState(sessionId, value1, value2); });
    effectFunc_.push_back([=, this]() { hpaeManager_->UpdateEffectBtOffloadSupported(value1); });
    effectFunc_.push_back([=, this]() { hpaeManager_->SetMicrophoneMuteInfo(value1); });
    effectFunc_.push_back([=, this]() { hpaeManager_->SetMicrophoneMuteInfo(value1); });
    for (size_t i = 0; i < NUM_2; i++) {
        uint32_t index = GetData<uint32_t>() % effectFunc_.size();
        effectFunc_[index]();
    }
    TearDown();
}

void DumpFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.DumpFuzzTest();
}

void StreamManagerFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.StreamManagerFuzzTest();
}

void MoveStreamManagerFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.MoveStreamManagerFuzzTest();
}

void HpaeManagerEffectTest()
{
    HpaeManagerFuzzTest t;
    t.HpaeManagerEffectTest();
}

void HpaeManagerEffectTest2()
{
    HpaeManagerFuzzTest t;
    t.HpaeManagerEffectTest2();
}

TestFuncs g_testFuncs[] = {
    DumpFuzzTest,
    StreamManagerFuzzTest,
    MoveStreamManagerFuzzTest,
    HpaeManagerEffectTest,
    HpaeManagerEffectTest2,
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
