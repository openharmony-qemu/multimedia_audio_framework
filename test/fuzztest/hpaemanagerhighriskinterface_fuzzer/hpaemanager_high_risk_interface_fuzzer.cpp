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
#include "hpaemanager_high_risk_interface_fuzzer.h"

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

vector<std::string> DeviceClassList = {
    "primary",
    "usb",
    "a2dp",
    "file_io",
    "remote",
    "accessory",
    "dp",
    "hearing_aid",
    "a2dp_fast",
    "remote_offload",
    "multichannel"
    "dp_multichannel",
    "primary_direct_voip",
    "primary_mmap_voip",
    "offload",
    "test",
    "",
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
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
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

void HpaeManagerFuzzTest::StreamSetUp()
{
    hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(sinkAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    sinkPortId_ = callback->GetPortId();
    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(sourceAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    sourcePortId_ = callback->GetPortId();
    std::shared_ptr<HpaeAudioServiceDumpCallbackUnitTest> dumpCallback =
        std::make_shared<HpaeAudioServiceDumpCallbackUnitTest>();
    hpaeManager_->RegisterHpaeDumpCallback(dumpCallback);
    HpaeStreamInfo rendererStreamInfo = GetRenderStreamInfo();
    hpaeManager_->CreateStream(rendererStreamInfo);
    HpaeStreamInfo streamInfo = GetCaptureStreamInfo();
    hpaeManager_->CreateStream(streamInfo);
    sourceNameList_ = {"mic", "test"};
    sinkNameList_ = {"Speaker_File", "test"};
    sourceOutputIdList_ = {streamInfo.sessionId, GetData<uint32_t>()};
    sinkInputIdList_ = {rendererStreamInfo.sessionId, GetData<uint32_t>()};
    WaitForMsgProcessing(hpaeManager_);
}

void HpaeManagerFuzzTest::AudioPortSetUp()
{
    hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
    hpaeManager_->Init();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo sinkAudioModuleInfo = GetSinkAudioModeInfo();
    hpaeManager_->OpenAudioPort(sinkAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    sinkPortId_ = callback->GetPortId();
    AudioModuleInfo sourceAudioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(sourceAudioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    sourcePortId_ = callback->GetPortId();
    audioPortNameList_ = {"Speaker_File", "mic", "test"};
    libList_ = {"libmodule-hdi-source.z.so", "libmodule-inner-capturer-sink.z.so", "libmodule-hdi-sink.z.so", "test"};
}

void HpaeManagerFuzzTest::TearDown()
{
    hpaeManager_->CloseAudioPort(sinkPortId_);
    hpaeManager_->CloseAudioPort(sourcePortId_);
    hpaeManager_->DeInit();
}

void HpaeManagerFuzzTest::HpaeCaptureStreamManagerMoveFuzzTest()
{
    StreamSetUp();
    uint32_t sessionId = sourceOutputIdList_[GetData<uint32_t>() % sourceOutputIdList_.size()];
    uint32_t sourceIndex = GetData<uint32_t>();
    std::string sourceName = sourceNameList_[GetData<uint32_t>() % sourceNameList_.size()];
    hpaeManager_->MoveSourceOutputByIndexOrName(sessionId, sourceIndex, sourceName);
    TearDown();
}

void HpaeManagerFuzzTest::HpaeRenderStreamManagerMoveFuzzTest()
{
    StreamSetUp();
    uint32_t sessionId = sinkInputIdList_[GetData<uint32_t>() % sinkInputIdList_.size()];
    uint32_t sinkIndex = GetData<uint32_t>();
    std::string sinkName = sinkNameList_[GetData<uint32_t>() % sinkNameList_.size()];
    hpaeManager_->MoveSinkInputByIndexOrName(sessionId, sinkIndex, sinkName);
    TearDown();
}

void HpaeManagerFuzzTest::OpenAudioPortFuzzTest()
{
    AudioPortSetUp();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    hpaeManager_->RegisterSerivceCallback(callback);
    AudioModuleInfo audioModuleInfo = GetSourceAudioModeInfo();
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();
    audioModuleInfo.lib = libList_[GetData<uint32_t>() % libList_.size()];
    audioModuleInfo.name = audioPortNameList_[GetData<uint32_t>() % audioPortNameList_.size()];
    audioModuleInfo.className = DeviceClassList[GetData<uint32_t>() % DeviceClassList.size()];
    hpaeManager_->OpenAudioPort(audioModuleInfo);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId2 = callback->GetPortId();
    hpaeManager_->CloseAudioPort(GetData<int32_t>());
    hpaeManager_->CloseAudioPort(portId);
    hpaeManager_->CloseAudioPort(portId2);
    TearDown();
}

void HpaeManagerFuzzTest::ReloadAudioPortFuzzTest()
{
    AudioPortSetUp();
    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    audioModuleInfo.lib = libList_[GetData<uint32_t>() % libList_.size()];
    audioModuleInfo.name = audioPortNameList_[GetData<uint32_t>() % audioPortNameList_.size()];
    audioModuleInfo.className = DeviceClassList[GetData<uint32_t>() % DeviceClassList.size()];
    hpaeManager_->ReloadAudioPort(audioModuleInfo);
    TearDown();
}

void HpaeRenderStreamManagerMoveFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.HpaeRenderStreamManagerMoveFuzzTest();
}

void HpaeCaptureStreamManagerMoveFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.HpaeCaptureStreamManagerMoveFuzzTest();
}

void OpenAudioPortFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.OpenAudioPortFuzzTest();
}

void ReloadAudioPortFuzzTest()
{
    HpaeManagerFuzzTest t;
    t.ReloadAudioPortFuzzTest();
}

TestFuncs g_testFuncs[] = {
    HpaeRenderStreamManagerMoveFuzzTest,
    HpaeCaptureStreamManagerMoveFuzzTest,
    OpenAudioPortFuzzTest,
    ReloadAudioPortFuzzTest,
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
