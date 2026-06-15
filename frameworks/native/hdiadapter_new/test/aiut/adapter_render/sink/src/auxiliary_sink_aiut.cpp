/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "auxiliary_sink_aiut.h"
#include "aiut_log.h"
#include "sink/auxiliary_sink.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SAMPLE_RATE_48000 = 48000;
constexpr uint32_t TEST_SAMPLE_RATE_44100 = 44100;
constexpr uint32_t TEST_SAMPLE_RATE_8000 = 8000;
constexpr uint32_t TEST_CHANNEL_STEREO = 2;
constexpr uint32_t TEST_CHANNEL_MONO = 1;
constexpr uint32_t TEST_CHANNEL_LAYOUT_STEREO = 3;
constexpr uint32_t TEST_CHANNEL_LAYOUT_MONO = 4;
constexpr float TEST_VOLUME_FULL = 1.0f;
constexpr float TEST_VOLUME_HALF = 0.5f;
constexpr float TEST_BALANCE_CENTER = 0.0f;
constexpr float TEST_BALANCE_LEFT = -1.0f;
constexpr float TEST_BALANCE_RIGHT = 1.0f;
constexpr uint32_t TEST_DURATION_MS = 100;
constexpr size_t TEST_BUFFER_SIZE = 4096;
constexpr int32_t TEST_APP_UID_1 = 1001;
constexpr int32_t TEST_APP_UID_2 = 1002;
constexpr int32_t MAX_AUXILIARY_BUFFERSIZE = 32768;
} // namespace

// ==================== AuxiliarySinkAiut ====================

void AuxiliarySinkAiut::SetUpTestCase(void) {}
void AuxiliarySinkAiut::TearDownTestCase(void) {}

void AuxiliarySinkAiut::SetUp(void)
{
    mockDeviceManager_ = std::make_shared<MockIDeviceManager>();
    mockMmapFd_ = -1;
}

void AuxiliarySinkAiut::TearDown(void)
{
    RestoreDeviceManager();
    if (mockMmapFd_ >= 0) {
        close(mockMmapFd_);
        mockMmapFd_ = -1;
    }
}

void AuxiliarySinkAiut::InjectMockDeviceManager()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    originalDeviceManager_ = manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL];
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager_;
}

void AuxiliarySinkAiut::RestoreDeviceManager()
{
    if (originalDeviceManager_ != nullptr) {
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = originalDeviceManager_;
        originalDeviceManager_ = nullptr;
    }
}

void AuxiliarySinkAiut::SetupMockCreateCognitionStreamSuccess()
{
    ON_CALL(*mockDeviceManager_, CreateCognitionStream(_, _, _, _))
        .WillByDefault(Invoke([](const std::string &, void *param, int32_t &sinkId, void *buffer) -> int32_t {
            sinkId = MOCK_SINK_ID;
            struct AudioMmapBufferDescriptor *desc =
                static_cast<struct AudioMmapBufferDescriptor *>(buffer);
            if (desc != nullptr) {
                // Create a temp fd for mmap
                int fd = memfd_create("test_aux_mmap", MFD_ALLOW_SEALING);
                if (fd < 0) {
                    char path[] = "/tmp/test_aux_mmap_XXXXXX";
                    fd = mkstemp(path);
                    unlink(path);
                }
                if (fd >= 0) {
                    ftruncate(fd, MAX_AUXILIARY_BUFFERSIZE);
                }
                desc->memoryAddress = nullptr;
                desc->memoryFd = fd;
                desc->totalBufferFrames = MOCK_TOTAL_BUFFER_FRAMES;
                desc->transferFrameSize = MOCK_TRANSFER_FRAME_SIZE;
                desc->isShareable = true;
                desc->offset = 0;
            }
            return SUCCESS;
        }));
}

void AuxiliarySinkAiut::SetupMockDestroyCognitionStreamSuccess()
{
    ON_CALL(*mockDeviceManager_, DestroyCognitionStream(_, _))
        .WillByDefault(Return(SUCCESS));
}

void AuxiliarySinkAiut::SetupMockNotifyCognitionDataSuccess()
{
    ON_CALL(*mockDeviceManager_, NotifyCognitionData(_, _, _, _))
        .WillByDefault(Return(SUCCESS));
}

void AuxiliarySinkAiut::SetupMockForFullInit()
{
    InjectMockDeviceManager();
    SetupMockCreateCognitionStreamSuccess();
    SetupMockDestroyCognitionStreamSuccess();
    SetupMockNotifyCognitionDataSuccess();
}

IAudioSinkAttr AuxiliarySinkAiut::CreateDefaultSinkAttr()
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sinkName = "usb";
    attr.sampleRate = TEST_SAMPLE_RATE_48000;
    attr.channel = TEST_CHANNEL_STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_STEREO;
    attr.deviceType = DEVICE_TYPE_SPEAKER;
    attr.volume = TEST_VOLUME_FULL;
    attr.openMicSpeaker = 1;
    return attr;
}

IAudioSinkAttr AuxiliarySinkAiut::CreateBtA2dpSinkAttr()
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "bt_a2dp";
    attr.sinkName = "a2dp";
    attr.sampleRate = TEST_SAMPLE_RATE_48000;
    attr.channel = TEST_CHANNEL_STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_STEREO;
    attr.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    attr.volume = TEST_VOLUME_FULL;
    attr.openMicSpeaker = 1;
    return attr;
}

std::shared_ptr<AuxiliarySink> AuxiliarySinkAiut::CreateAndInitSink()
{
    auto sink = std::make_shared<AuxiliarySink>();
    if (sink == nullptr) {
        return nullptr;
    }
    IAudioSinkAttr attr = CreateDefaultSinkAttr();
    int32_t ret = sink->Init(attr);
    if (ret != SUCCESS) {
        return nullptr;
    }
    return sink;
}

void AuxiliarySinkAiut::DeInitAndResetSink(std::shared_ptr<AuxiliarySink> &sink)
{
    if (sink != nullptr && sink->IsInited()) {
        sink->DeInit();
    }
    sink.reset();
}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test AuxiliarySink default construction
 * @tc.number: AuxiliarySink_Constructor_001
 * @tc.desc  : Test AuxiliarySink default constructor creates object that is not initialized.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

/**
 * @tc.name  : Test AuxiliarySink destruction with uninitialized sink
 * @tc.number: AuxiliarySink_Destructor_001
 * @tc.desc  : Test AuxiliarySink destructor on uninitialized sink does not crash.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink.reset(); // should not crash
}

// ==================== IsInited Tests ====================

/**
 * @tc.name  : Test IsInited returns false before Init
 * @tc.number: AuxiliarySink_IsInited_001
 * @tc.desc  : Test IsInited returns false for newly constructed AuxiliarySink.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_IsInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited before Init");
}

// ==================== Start / Stop / Pause / Resume / Flush / Reset Tests ====================

/**
 * @tc.name  : Test Start returns SUCCESS (not supported but returns SUCCESS)
 * @tc.number: AuxiliarySink_Start_001
 * @tc.desc  : Test Start returns SUCCESS since AuxiliarySink does not support Start.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Start_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Start should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test Stop returns SUCCESS (not supported but returns SUCCESS)
 * @tc.number: AuxiliarySink_Stop_001
 * @tc.desc  : Test Stop returns SUCCESS since AuxiliarySink does not support Stop.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Stop_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Stop should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test Pause returns SUCCESS (not supported but returns SUCCESS)
 * @tc.number: AuxiliarySink_Pause_001
 * @tc.desc  : Test Pause returns SUCCESS since AuxiliarySink does not support Pause.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Pause_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Pause();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Pause should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test Resume returns SUCCESS (not supported but returns SUCCESS)
 * @tc.number: AuxiliarySink_Resume_001
 * @tc.desc  : Test Resume returns SUCCESS since AuxiliarySink does not support Resume.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Resume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Resume should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test Flush returns SUCCESS (not supported but returns SUCCESS)
 * @tc.number: AuxiliarySink_Flush_001
 * @tc.desc  : Test Flush returns SUCCESS since AuxiliarySink does not support Flush.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Flush_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Flush();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Flush should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test Reset returns SUCCESS (not supported but returns SUCCESS)
 * @tc.number: AuxiliarySink_Reset_001
 * @tc.desc  : Test Reset returns SUCCESS since AuxiliarySink does not support Reset.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Reset_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Reset();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Reset should return SUCCESS, ret=%d", ret);
}

// ==================== Volume Tests ====================

/**
 * @tc.name  : Test SetVolume returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_SetVolume_001
 * @tc.desc  : Test SetVolume returns ERR_NOT_SUPPORTED since AuxiliarySink does not support volume control.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "SetVolume should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test SetVolumeWithRamp returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_SetVolumeWithRamp_001
 * @tc.desc  : Test SetVolumeWithRamp returns ERR_NOT_SUPPORTED since AuxiliarySink does not support it.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetVolumeWithRamp_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolumeWithRamp(TEST_VOLUME_HALF, TEST_VOLUME_HALF, TEST_DURATION_MS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "SetVolumeWithRamp should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetVolume returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_GetVolume_001
 * @tc.desc  : Test GetVolume returns ERR_NOT_SUPPORTED since AuxiliarySink does not support it.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink->GetVolume(left, right);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetVolume should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

// ==================== Latency / Position / Amplitude Tests ====================

/**
 * @tc.name  : Test GetLatency returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_GetLatency_001
 * @tc.desc  : Test GetLatency returns ERR_NOT_SUPPORTED since AuxiliarySink does not support it.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetLatency_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint32_t latency = 0;
    int32_t ret = sink->GetLatency(latency);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetLatency should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetPresentationPosition returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_GetPresentationPosition_001
 * @tc.desc  : Test GetPresentationPosition returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetPresentationPosition_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = sink->GetPresentationPosition(frames, timeSec, timeNanoSec);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetPresentationPosition should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude returns 0.0
 * @tc.number: AuxiliarySink_GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude returns 0.0 since AuxiliarySink does not support it.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetMaxAmplitude_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float amplitude = sink->GetMaxAmplitude();
    AIUT_EXPECT_EQ_INFO_LOG(amplitude, 0.0f,
        "GetMaxAmplitude should return 0.0, amplitude=%f", amplitude);
}

// ==================== AudioMono / AudioBalance Tests ====================

/**
 * @tc.name  : Test SetAudioMonoState does not crash
 * @tc.number: AuxiliarySink_SetAudioMonoState_001
 * @tc.desc  : Test SetAudioMonoState with true and false does not crash.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetAudioMonoState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
}

/**
 * @tc.name  : Test SetAudioBalanceValue does not crash
 * @tc.number: AuxiliarySink_SetAudioBalanceValue_001
 * @tc.desc  : Test SetAudioBalanceValue with various values does not crash.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetAudioBalanceValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioBalanceValue(TEST_BALANCE_CENTER);
    sink->SetAudioBalanceValue(TEST_BALANCE_LEFT);
    sink->SetAudioBalanceValue(TEST_BALANCE_RIGHT);
}

// ==================== UpdateAppsUid Tests ====================

/**
 * @tc.name  : Test UpdateAppsUid with array input returns SUCCESS
 * @tc.number: AuxiliarySink_UpdateAppsUid_Array_001
 * @tc.desc  : Test UpdateAppsUid with array input returns SUCCESS.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_UpdateAppsUid_Array_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid array should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test UpdateAppsUid with vector input returns SUCCESS
 * @tc.number: AuxiliarySink_UpdateAppsUid_Vector_001
 * @tc.desc  : Test UpdateAppsUid with vector input returns SUCCESS.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_UpdateAppsUid_Vector_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<int32_t> appsUid = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UpdateAppsUid vector should return SUCCESS, ret=%d", ret);
}

// ==================== GetVolumeDataCount Tests ====================

/**
 * @tc.name  : Test GetVolumeDataCount returns 0
 * @tc.number: AuxiliarySink_GetVolumeDataCount_001
 * @tc.desc  : Test GetVolumeDataCount returns 0 since AuxiliarySink does not support it.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetVolumeDataCount_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int64_t volumeData = -1;
    int32_t ret = sink->GetVolumeDataCount(volumeData);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetVolumeDataCount should return 0, ret=%d", ret);
}

// ==================== RenderFrame Tests (Not Inited) ====================

/**
 * @tc.name  : Test RenderFrame without Init returns ERROR_ILLEGAL_STATE
 * @tc.number: AuxiliarySink_RenderFrame_NotInited_001
 * @tc.desc  : Test RenderFrame on uninitialized sink returns ERROR_ILLEGAL_STATE.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RenderFrame_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_ILLEGAL_STATE,
        "RenderFrame on uninitied sink should return ERROR_ILLEGAL_STATE, ret=%d", ret);
}

// ==================== DeInit Tests (Not Inited) ====================

/**
 * @tc.name  : Test DeInit without Init does not crash
 * @tc.number: AuxiliarySink_DeInit_NotInited_001
 * @tc.desc  : Test DeInit on uninitialized sink does not crash.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_DeInit_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->DeInit(); // should not crash
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo on uninitialized sink
 * @tc.number: AuxiliarySink_DumpInfo_001
 * @tc.desc  : Test DumpInfo produces output containing "AuxSink" and init state for uninitiated sink.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::string dumpString;
    sink->DumpInfo(dumpString);
    AIUT_EXPECT_NE_INFO_LOG(dumpString.find("AuxSink"), std::string::npos,
        "dump string should contain 'AuxSink'");
    AIUT_EXPECT_NE_INFO_LOG(dumpString.find("false"), std::string::npos,
        "dump string should contain 'false' for init state");
}

// ==================== HdiAdapterFactory CreateAuxiliarySink Tests ====================

/**
 * @tc.name  : Test CreateAuxiliarySink via HdiAdapterFactory
 * @tc.number: HdiAdapterFactory_CreateAuxiliarySink_001
 * @tc.desc  : Test HdiAdapterFactory::CreateAuxiliarySink creates a valid AuxiliarySink instance.
 */
HWTEST_F(AuxiliarySinkAiut, HdiAdapterFactory_CreateAuxiliarySink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = HdiAdapterFactory::GetInstance().CreateAuxiliarySink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "CreateAuxiliarySink returned null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(),
        "factory-created AuxiliarySink should not be inited");
}

// ==================== HdiAdapterManager GetAuxiliarySink Tests ====================

/**
 * @tc.name  : Test GetAuxiliarySink via HdiAdapterManager
 * @tc.number: HdiAdapterManager_GetAuxiliarySink_001
 * @tc.desc  : Test HdiAdapterManager::GetAuxiliarySink returns a valid AuxiliarySink instance.
 */
HWTEST_F(AuxiliarySinkAiut, HdiAdapterManager_GetAuxiliarySink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = HdiAdapterManager::GetInstance().GetAuxiliarySink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "GetAuxiliarySink returned null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(),
        "manager-created AuxiliarySink should not be inited");
}

// ==================== IAudioRenderSink Default Interface Tests ====================

/**
 * @tc.name  : Test SuspendRenderSink default returns SUCCESS
 * @tc.number: AuxiliarySink_SuspendRenderSink_001
 * @tc.desc  : Test SuspendRenderSink returns SUCCESS by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SuspendRenderSink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SuspendRenderSink();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SuspendRenderSink should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test RestoreRenderSink default returns SUCCESS
 * @tc.number: AuxiliarySink_RestoreRenderSink_001
 * @tc.desc  : Test RestoreRenderSink returns SUCCESS by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RestoreRenderSink_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->RestoreRenderSink();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "RestoreRenderSink should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice default returns SUCCESS
 * @tc.number: AuxiliarySink_SetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice returns SUCCESS by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetSinkMuteForSwitchDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetSinkMuteForSwitchDevice(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice should return SUCCESS, ret=%d", ret);
    ret = sink->SetSinkMuteForSwitchDevice(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetSinkMuteForSwitchDevice(false) should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test SetRenderEmpty default returns SUCCESS
 * @tc.number: AuxiliarySink_SetRenderEmpty_001
 * @tc.desc  : Test SetRenderEmpty returns SUCCESS by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetRenderEmpty_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetRenderEmpty(0);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "SetRenderEmpty should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test RegistDirectHdiCallback default returns SUCCESS
 * @tc.number: AuxiliarySink_RegistDirectHdiCallback_001
 * @tc.desc  : Test RegistDirectHdiCallback returns SUCCESS by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RegistDirectHdiCallback_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::function<void(const RenderCallbackType type)> callback =
        [](const RenderCallbackType type) {};
    int32_t ret = sink->RegistDirectHdiCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "RegistDirectHdiCallback should return SUCCESS, ret=%d", ret);
}

// ==================== Not-Supported Interface Tests ====================

/**
 * @tc.name  : Test SetAudioScene returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene returns ERR_NOT_SUPPORTED by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetAudioScene(AUDIO_SCENE_DEFAULT);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "SetAudioScene should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetAudioScene returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_GetAudioScene_001
 * @tc.desc  : Test GetAudioScene returns ERR_NOT_SUPPORTED by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->GetAudioScene();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetAudioScene should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test Drain returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_Drain_001
 * @tc.desc  : Test Drain returns ERR_NOT_SUPPORTED by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Drain_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->Drain(AUDIO_DRAIN_NORMAL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "Drain should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test SetBufferSize returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_SetBufferSize_001
 * @tc.desc  : Test SetBufferSize returns ERR_NOT_SUPPORTED by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_SetBufferSize_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetBufferSize(100);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "SetBufferSize should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test IsInA2dpOffload returns false
 * @tc.number: AuxiliarySink_IsInA2dpOffload_001
 * @tc.desc  : Test IsInA2dpOffload returns false by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_IsInA2dpOffload_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInA2dpOffload(), "IsInA2dpOffload should return false");
}

/**
 * @tc.name  : Test IsSinkInited returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_IsSinkInited_001
 * @tc.desc  : Test IsSinkInited returns ERR_NOT_SUPPORTED by default (base class).
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_IsSinkInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->IsSinkInited();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "IsSinkInited should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetMmapBufferInfo returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_GetMmapBufferInfo_001
 * @tc.desc  : Test GetMmapBufferInfo returns ERR_NOT_SUPPORTED by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetMmapBufferInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t fd = -1;
    uint32_t totalSizeInframe = 0;
    uint32_t spanSizeInframe = 0;
    uint32_t byteSizePerFrame = 0;
    uint32_t syncInfoSize = 0;
    int32_t ret = sink->GetMmapBufferInfo(fd, totalSizeInframe, spanSizeInframe,
        byteSizePerFrame, syncInfoSize);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetMmapBufferInfo should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetMmapHandlePosition returns ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_GetMmapHandlePosition_001
 * @tc.desc  : Test GetMmapHandlePosition returns ERR_NOT_SUPPORTED by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetMmapHandlePosition_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = sink->GetMmapHandlePosition(frames, timeSec, timeNanoSec);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetMmapHandlePosition should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetCurrentOutputDevice returns DEVICE_TYPE_INVALID
 * @tc.number: AuxiliarySink_GetCurrentOutputDevice_001
 * @tc.desc  : Test GetCurrentOutputDevice returns DEVICE_TYPE_INVALID by default.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetCurrentOutputDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    DeviceType deviceType = sink->GetCurrentOutputDevice();
    AIUT_EXPECT_EQ_INFO_LOG(deviceType, DEVICE_TYPE_INVALID,
        "GetCurrentOutputDevice should return DEVICE_TYPE_INVALID, deviceType=%d", deviceType);
}

// ==================== Init With Mock Tests ====================

/**
 * @tc.name  : Test Init with null device manager returns ERR_INVALID_HANDLE
 * @tc.number: AuxiliarySink_Init_NullDeviceManager_001
 * @tc.desc  : Test Init fails when GetDeviceManager returns nullptr.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Init_NullDeviceManager_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // Inject mock but set GetDeviceManager to return null by not setting it
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    originalDeviceManager_ = manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL];
    manager.deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = nullptr;

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultSinkAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED,
        "Init should fail when deviceManager is null, ret=%d", ret);
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after failed Init");
}

/**
 * @tc.name  : Test Init when CreateCognitionStream fails
 * @tc.number: AuxiliarySink_Init_CreateCognitionStreamFail_001
 * @tc.desc  : Test Init fails when CreateCognitionStream returns error.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Init_CreateCognitionStreamFail_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    InjectMockDeviceManager();
    EXPECT_CALL(*mockDeviceManager_, CreateCognitionStream(_, _, _, _))
        .WillOnce(Return(ERR_OPERATION_FAILED));

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultSinkAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED,
        "Init should fail when CreateCognitionStream fails, ret=%d", ret);
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after failed Init");
}

/**
 * @tc.name  : Test Init succeeds with mock device manager
 * @tc.number: AuxiliarySink_Init_Success_001
 * @tc.desc  : Test Init returns SUCCESS and IsInited becomes true with valid mock.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Init_Success_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultSinkAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited after successful Init");

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test Init with bt_a2dp adapterName
 * @tc.number: AuxiliarySink_Init_BtA2dp_001
 * @tc.desc  : Test Init with bt_a2dp adapterName sets halName and logTag correctly.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Init_BtA2dp_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateBtA2dpSinkAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited after successful Init");

    // Verify DumpInfo contains bt-related info
    std::string dumpString;
    sink->DumpInfo(dumpString);
    AIUT_EXPECT_NE_INFO_LOG(dumpString.find("true"), std::string::npos,
        "dump string should contain 'true' for inited state");

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test Init with different sample rates
 * @tc.number: AuxiliarySink_Init_DifferentSampleRates_001
 * @tc.desc  : Test Init with various sample rate configurations.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Init_DifferentSampleRates_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");

    // Test 48kHz stereo
    IAudioSinkAttr attr = CreateDefaultSinkAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init 48kHz should return SUCCESS, ret=%d", ret);
    sink->DeInit();

    // Test 44.1kHz stereo
    attr = CreateDefaultSinkAttr();
    attr.sampleRate = TEST_SAMPLE_RATE_44100;
    ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init 44.1kHz should return SUCCESS, ret=%d", ret);
    sink->DeInit();

    // Test 8kHz mono
    attr = CreateDefaultSinkAttr();
    attr.sampleRate = TEST_SAMPLE_RATE_8000;
    attr.channel = TEST_CHANNEL_MONO;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_MONO;
    ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init 8kHz mono should return SUCCESS, ret=%d", ret);
    sink->DeInit();
}

/**
 * @tc.name  : Test Init with different PCM formats
 * @tc.number: AuxiliarySink_Init_DifferentFormats_001
 * @tc.desc  : Test Init with various audio sample formats (S16LE, S24LE, S32LE, U8).
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Init_DifferentFormats_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");

    AudioSampleFormat formats[] = { SAMPLE_S16LE, SAMPLE_S24LE, SAMPLE_S32LE, SAMPLE_U8 };
    for (auto format : formats) {
        IAudioSinkAttr attr = CreateDefaultSinkAttr();
        attr.format = format;
        int32_t ret = sink->Init(attr);
        AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init with format should return SUCCESS, ret=%d", ret);
        sink->DeInit();
    }
}

// ==================== RenderFrame With Mock Tests ====================

/**
 * @tc.name  : Test RenderFrame after Init returns SUCCESS
 * @tc.number: AuxiliarySink_RenderFrame_AfterInit_001
 * @tc.desc  : Test RenderFrame on initialized sink returns SUCCESS.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RenderFrame_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "RenderFrame after Init should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_GT_INFO_LOG(writeLen, (uint64_t)0,
        "writeLen should be greater than 0, writeLen=%lu", writeLen);

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test RenderFrame with oversized buffer returns ERR_WRITE_FAILED
 * @tc.number: AuxiliarySink_RenderFrame_OversizedBuffer_001
 * @tc.desc  : Test RenderFrame with len > bufferSize returns ERR_WRITE_FAILED.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RenderFrame_OversizedBuffer_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    char data[MAX_AUXILIARY_BUFFERSIZE + 1] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, MAX_AUXILIARY_BUFFERSIZE + 1, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_WRITE_FAILED,
        "RenderFrame with oversized buffer should return ERR_WRITE_FAILED, ret=%d", ret);

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test RenderFrame when NotifyCognitionData fails
 * @tc.number: AuxiliarySink_RenderFrame_NotifyFail_001
 * @tc.desc  : Test RenderFrame returns ERR_OPERATION_FAILED when NotifyCognitionData fails.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RenderFrame_NotifyFail_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    InjectMockDeviceManager();
    SetupMockCreateCognitionStreamSuccess();
    SetupMockDestroyCognitionStreamSuccess();
    EXPECT_CALL(*mockDeviceManager_, NotifyCognitionData(_, _, _, _))
        .WillOnce(Return(ERR_OPERATION_FAILED));

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED,
        "RenderFrame should fail when NotifyCognitionData fails, ret=%d", ret);

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test RenderFrame with null device manager after Init
 * @tc.number: AuxiliarySink_RenderFrame_NullDeviceManager_001
 * @tc.desc  : Test RenderFrame returns ERR_INVALID_HANDLE when deviceManager becomes null.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RenderFrame_NullDeviceManager_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    // Remove device manager after Init
    HdiAdapterManager::GetInstance().deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = nullptr;

    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "RenderFrame should return ERR_INVALID_HANDLE when deviceManager is null, ret=%d", ret);

    // Restore for DeInit
    HdiAdapterManager::GetInstance().deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = mockDeviceManager_;
    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test RenderFrame multiple frames in sequence
 * @tc.number: AuxiliarySink_RenderFrame_MultipleFrames_001
 * @tc.desc  : Test RenderFrame can be called multiple times in sequence after Init.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_RenderFrame_MultipleFrames_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    char data[TEST_BUFFER_SIZE] = {0};
    for (int i = 0; i < 4; i++) {
        uint64_t writeLen = 0;
        int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
        AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
            "RenderFrame #%d should return SUCCESS, ret=%d", i, ret);
    }

    DeInitAndResetSink(sink);
}

// ==================== DeInit With Mock Tests ====================

/**
 * @tc.name  : Test DeInit after Init sets IsInited to false
 * @tc.number: AuxiliarySink_DeInit_AfterInit_001
 * @tc.desc  : Test DeInit after successful Init sets IsInited to false.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_DeInit_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");
    AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited before DeInit");

    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

/**
 * @tc.name  : Test DeInit when DestroyCognitionStream fails does not crash
 * @tc.number: AuxiliarySink_DeInit_DestroyCognitionStreamFail_001
 * @tc.desc  : Test DeInit does not crash even when DestroyCognitionStream returns error.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_DeInit_DestroyCognitionStreamFail_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    InjectMockDeviceManager();
    SetupMockCreateCognitionStreamSuccess();
    SetupMockNotifyCognitionDataSuccess();
    EXPECT_CALL(*mockDeviceManager_, DestroyCognitionStream(_, _))
        .WillRepeatedly(Return(ERR_OPERATION_FAILED));

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    sink->DeInit(); // should not crash even if DestroyCognitionStream fails
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

/**
 * @tc.name  : Test DeInit with null device manager after Init
 * @tc.number: AuxiliarySink_DeInit_NullDeviceManager_001
 * @tc.desc  : Test DeInit does not crash when deviceManager becomes null after Init.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_DeInit_NullDeviceManager_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    // Remove device manager before DeInit
    HdiAdapterManager::GetInstance().deviceManagers_[HDI_DEVICE_MANAGER_TYPE_LOCAL] = nullptr;

    sink->DeInit(); // should not crash
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

// ==================== DumpInfo After Init Tests ====================

/**
 * @tc.name  : Test DumpInfo after Init shows inited state
 * @tc.number: AuxiliarySink_DumpInfo_AfterInit_001
 * @tc.desc  : Test DumpInfo after Init contains "true" for init state and sample rate info.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_DumpInfo_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    std::string dumpString;
    sink->DumpInfo(dumpString);
    AIUT_EXPECT_NE_INFO_LOG(dumpString.find("AuxSink"), std::string::npos,
        "dump string should contain 'AuxSink'");
    AIUT_EXPECT_NE_INFO_LOG(dumpString.find("true"), std::string::npos,
        "dump string should contain 'true' for inited state");
    AIUT_EXPECT_NE_INFO_LOG(dumpString.find(std::to_string(TEST_SAMPLE_RATE_48000)),
        std::string::npos, "dump string should contain sample rate");

    DeInitAndResetSink(sink);
}

// ==================== Init + Lifecycle After Init Tests ====================

/**
 * @tc.name  : Test Start/Stop/Pause/Resume/Flush/Reset after Init
 * @tc.number: AuxiliarySink_Lifecycle_AfterInit_001
 * @tc.desc  : Test lifecycle methods still return SUCCESS after Init (AuxiliarySink does not use them).
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Lifecycle_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    AIUT_EXPECT_EQ_INFO_LOG(sink->Start(), SUCCESS, "Start should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Pause(), SUCCESS, "Pause should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Resume(), SUCCESS, "Resume should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Flush(), SUCCESS, "Flush should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Reset(), SUCCESS, "Reset should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Stop(), SUCCESS, "Stop should return SUCCESS");

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test volume/latency methods after Init still return ERR_NOT_SUPPORTED
 * @tc.number: AuxiliarySink_UnsupportedAfterInit_001
 * @tc.desc  : Test that volume and latency methods still return ERR_NOT_SUPPORTED after Init.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_UnsupportedAfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    float left = 0.0f;
    float right = 0.0f;
    AIUT_EXPECT_EQ_INFO_LOG(sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL),
        ERR_NOT_SUPPORTED, "SetVolume should return ERR_NOT_SUPPORTED after Init");
    AIUT_EXPECT_EQ_INFO_LOG(sink->GetVolume(left, right),
        ERR_NOT_SUPPORTED, "GetVolume should return ERR_NOT_SUPPORTED after Init");

    uint32_t latency = 0;
    AIUT_EXPECT_EQ_INFO_LOG(sink->GetLatency(latency),
        ERR_NOT_SUPPORTED, "GetLatency should return ERR_NOT_SUPPORTED after Init");

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test SetAudioMonoState and SetAudioBalanceValue after Init
 * @tc.number: AuxiliarySink_MonoBalance_AfterInit_001
 * @tc.desc  : Test SetAudioMonoState and SetAudioBalanceValue do not crash after Init.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_MonoBalance_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
    sink->SetAudioBalanceValue(TEST_BALANCE_CENTER);
    sink->SetAudioBalanceValue(TEST_BALANCE_LEFT);
    sink->SetAudioBalanceValue(TEST_BALANCE_RIGHT);

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test GetVolumeDataCount after Init
 * @tc.number: AuxiliarySink_GetVolumeDataCount_AfterInit_001
 * @tc.desc  : Test GetVolumeDataCount after Init still returns 0.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_GetVolumeDataCount_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    int64_t volumeData = -1;
    int32_t ret = sink->GetVolumeDataCount(volumeData);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetVolumeDataCount should return 0, ret=%d", ret);

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test UpdateAppsUid after Init
 * @tc.number: AuxiliarySink_UpdateAppsUid_AfterInit_001
 * @tc.desc  : Test UpdateAppsUid still returns SUCCESS after Init.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_UpdateAppsUid_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = CreateAndInitSink();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null or init failed");

    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UpdateAppsUid array should return SUCCESS, ret=%d", ret);

    std::vector<int32_t> appsUidVec = {TEST_APP_UID_1, TEST_APP_UID_2};
    ret = sink->UpdateAppsUid(appsUidVec);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UpdateAppsUid vector should return SUCCESS, ret=%d", ret);

    DeInitAndResetSink(sink);
}

/**
 * @tc.name  : Test Init and DeInit multiple times
 * @tc.number: AuxiliarySink_InitDeInit_Multiple_001
 * @tc.desc  : Test Init and DeInit can be called multiple times in sequence.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_InitDeInit_Multiple_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    auto sink = std::make_shared<AuxiliarySink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");

    for (int i = 0; i < 3; i++) {
        IAudioSinkAttr attr = CreateDefaultSinkAttr();
        int32_t ret = sink->Init(attr);
        AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init #%d should return SUCCESS, ret=%d", i, ret);
        AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited after Init #%d", i);

        sink->DeInit();
        AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit #%d", i);
    }
}

/**
 * @tc.name  : Test destructor calls DeInit on inited sink
 * @tc.number: AuxiliarySink_Destructor_Inited_001
 * @tc.desc  : Test AuxiliarySink destructor properly cleans up an inited sink without crash.
 */
HWTEST_F(AuxiliarySinkAiut, AuxiliarySink_Destructor_Inited_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    SetupMockForFullInit();

    {
        auto sink = std::make_shared<AuxiliarySink>();
        AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
        IAudioSinkAttr attr = CreateDefaultSinkAttr();
        int32_t ret = sink->Init(attr);
        AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init should return SUCCESS, ret=%d", ret);
        AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited");
        // sink goes out of scope and destructor should call DeInit
    }
    // If we reach here without crash, the test passes
}

} // namespace AudioStandard
} // namespace OHOS
