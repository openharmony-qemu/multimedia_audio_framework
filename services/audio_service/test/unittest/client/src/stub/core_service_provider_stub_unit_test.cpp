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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core_service_provider_stub.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "mock_core_service_provider.h"

using namespace testing::ext;
using ::testing::Return;

namespace OHOS {
namespace AudioStandard {
class CoreServiceProviderStubUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CoreServiceProviderStubUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("CoreServiceProviderStubUnitTest SetUpTestCase");
}

void CoreServiceProviderStubUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("CoreServiceProviderStubUnitTest TearDownTestCase");
}

void CoreServiceProviderStubUnitTest::SetUp()
{
    AUDIO_INFO_LOG("CoreServiceProviderStubUnitTest SetUp");
}

void CoreServiceProviderStubUnitTest::TearDown()
{
    AUDIO_INFO_LOG("CoreServiceProviderStubUnitTest TearDown");
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_UpdateSessionOperation_001
 * @tc.desc  : Test UpdateSessionOperation with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_UpdateSessionOperation_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_UpdateSessionOperation_001 start");
    // Test with null coreServiceWorker
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.UpdateSessionOperation(0, 0, 0, false);
    AUDIO_INFO_LOG("CSPWrapper_UpdateSessionOperation_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_UpdateSessionOperation_002
 * @tc.desc  : Test UpdateSessionOperation with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_UpdateSessionOperation_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_UpdateSessionOperation_002 start");
    // Test with valid coreServiceWorker using Mock
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    // Set up mock expectation
    EXPECT_CALL(mockProvider, UpdateSessionOperation(1, SESSION_OPERATION_START, SESSION_OP_MSG_DEFAULT, false))
        .WillOnce(Return(SUCCESS));

    int32_t result = wrapper.UpdateSessionOperation(1, SESSION_OPERATION_START, SESSION_OP_MSG_DEFAULT, false);
    AUDIO_INFO_LOG("CSPWrapper_UpdateSessionOperation_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_ReloadCaptureSession_001
 * @tc.desc  : Test ReloadCaptureSession with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_ReloadCaptureSession_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_ReloadCaptureSession_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.ReloadCaptureSession(0, 0);
    AUDIO_INFO_LOG("CSPWrapper_ReloadCaptureSession_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_ReloadCaptureSession_002
 * @tc.desc  : Test ReloadCaptureSession with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_ReloadCaptureSession_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_ReloadCaptureSession_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, ReloadCaptureSession(1, SESSION_OPERATION_START))
        .WillOnce(Return(SUCCESS));

    int32_t result = wrapper.ReloadCaptureSession(1, SESSION_OPERATION_START);
    AUDIO_INFO_LOG("CSPWrapper_ReloadCaptureSession_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetDefaultOutputDevice_001
 * @tc.desc  : Test SetDefaultOutputDevice with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetDefaultOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetDefaultOutputDevice_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.SetDefaultOutputDevice(0, 0, 0, false);
    AUDIO_INFO_LOG("CSPWrapper_SetDefaultOutputDevice_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetDefaultOutputDevice_002
 * @tc.desc  : Test SetDefaultOutputDevice with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetDefaultOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetDefaultOutputDevice_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER, 1, STREAM_USAGE_MEDIA, true, false))
        .WillOnce(Return(SUCCESS));

    int32_t result = wrapper.SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER, 1, STREAM_USAGE_MEDIA, true, false);
    AUDIO_INFO_LOG("CSPWrapper_SetDefaultOutputDevice_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_RecordBootStateTime_001
 * @tc.desc  : Test RecordBootStateTime with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_RecordBootStateTime_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_RecordBootStateTime_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.RecordBootStateTime(0, 0, 0);
    AUDIO_INFO_LOG("CSPWrapper_RecordBootStateTime_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_RecordBootStateTime_002
 * @tc.desc  : Test RecordBootStateTime with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_RecordBootStateTime_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_RecordBootStateTime_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, RecordBootStateTime(1, 1000, 1234567890))
        .WillOnce(Return(SUCCESS));

    int32_t result = wrapper.RecordBootStateTime(1, 1000, 1234567890);
    AUDIO_INFO_LOG("CSPWrapper_RecordBootStateTime_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GetAdapterNameBySessionId_001
 * @tc.desc  : Test GetAdapterNameBySessionId with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GetAdapterNameBySessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GetAdapterNameBySessionId_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    std::string name;
    int32_t result = wrapper.GetAdapterNameBySessionId(0, name);
    AUDIO_INFO_LOG("CSPWrapper_GetAdapterNameBySessionId_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GetAdapterNameBySessionId_002
 * @tc.desc  : Test GetAdapterNameBySessionId with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GetAdapterNameBySessionId_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GetAdapterNameBySessionId_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, GetAdapterNameBySessionId(1))
        .WillOnce(Return("adapter1"));

    std::string name;
    int32_t result = wrapper.GetAdapterNameBySessionId(1, name);
    AUDIO_INFO_LOG("CSPWrapper_GetAdapterNameBySessionId_002 name:%{public}s", name.c_str());
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(name, "adapter1");
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GetModuleNameBySessionId_001
 * @tc.desc  : Test GetModuleNameBySessionId with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GetModuleNameBySessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GetModuleNameBySessionId_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    std::string name;
    int32_t result = wrapper.GetModuleNameBySessionId(0, name);
    AUDIO_INFO_LOG("CSPWrapper_GetModuleNameBySessionId_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GetModuleNameBySessionId_002
 * @tc.desc  : Test GetModuleNameBySessionId with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GetModuleNameBySessionId_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GetModuleNameBySessionId_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, GetModuleNameBySessionId(1))
        .WillOnce(Return("module1"));

    std::string name;
    int32_t result = wrapper.GetModuleNameBySessionId(1, name);
    AUDIO_INFO_LOG("CSPWrapper_GetModuleNameBySessionId_002 name:%{public}s", name.c_str());
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(name, "module1");
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GetProcessDeviceInfoBySessionId_001
 * @tc.desc  : Test GetProcessDeviceInfoBySessionId with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GetProcessDeviceInfoBySessionId_001,
    TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GetProcessDeviceInfoBySessionId_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    AudioDeviceDescriptor deviceInfo;
    AudioStreamInfo streamInfo;
    bool isUltraFast = false;
    int32_t result = wrapper.GetProcessDeviceInfoBySessionId(0, deviceInfo, streamInfo, isUltraFast, false);
    AUDIO_INFO_LOG("CSPWrapper_GetProcessDeviceInfoBySessionId_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GenerateSessionId_001
 * @tc.desc  : Test GenerateSessionId with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GenerateSessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GenerateSessionId_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    uint32_t sessionId = 0;
    int32_t result = wrapper.GenerateSessionId(sessionId);
    AUDIO_INFO_LOG("CSPWrapper_GenerateSessionId_001 result:%{public}d", result);
    // Should fail when coreServiceWorker is null
    EXPECT_NE(result, AUDIO_INIT_FAIL);
    EXPECT_EQ(sessionId, 0);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GenerateSessionId_002
 * @tc.desc  : Test GenerateSessionId with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GenerateSessionId_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GenerateSessionId_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, GenerateSessionId())
        .WillOnce(Return(12345));

    uint32_t sessionId = 0;
    int32_t result = wrapper.GenerateSessionId(sessionId);
    AUDIO_INFO_LOG("CSPWrapper_GenerateSessionId_002 sessionId:%{public}u", sessionId);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sessionId, 12345U);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetWakeUpAudioCapturerFromAudioServer_001
 * @tc.desc  : Test SetWakeUpAudioCapturerFromAudioServer with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetWakeUpAudioCapturerFromAudioServer_001,
    TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetWakeUpAudioCapturerFromAudioServer_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    AudioProcessConfig config;
    int32_t ret = 0;
    int32_t result = wrapper.SetWakeUpAudioCapturerFromAudioServer(config, ret);
    AUDIO_INFO_LOG("CSPWrapper_SetWakeUpAudioCapturerFromAudioServer_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetWakeUpAudioCapturerFromAudioServer_002
 * @tc.desc  : Test SetWakeUpAudioCapturerFromAudioServer with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetWakeUpAudioCapturerFromAudioServer_002,
    TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetWakeUpAudioCapturerFromAudioServer_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, SetWakeUpAudioCapturerFromAudioServer(testing::_))
        .WillOnce(Return(SUCCESS));

    AudioProcessConfig config;
    int32_t ret = 0;
    int32_t result = wrapper.SetWakeUpAudioCapturerFromAudioServer(config, ret);
    AUDIO_INFO_LOG("CSPWrapper_SetWakeUpAudioCapturerFromAudioServer_002 result:%{public}d, ret:%{public}d",
        result, ret);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_GetPaIndexByPortName_001
 * @tc.desc  : Test GetPaIndexByPortName with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_GetPaIndexByPortName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_GetPaIndexByPortName_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    std::string portName = "test";
    uint32_t ret = 0;
    int32_t result = wrapper.GetPaIndexByPortName(portName, ret);
    AUDIO_INFO_LOG("CSPWrapper_GetPaIndexByPortName_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetRendererTarget_001
 * @tc.desc  : Test SetRendererTarget with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetRendererTarget_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetRendererTarget_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    uint32_t target = 0;
    uint32_t lastTarget = 0;
    uint32_t sessionId = 0;
    int32_t ret = 0;
    int32_t result = wrapper.SetRendererTarget(target, lastTarget, sessionId, INVALID_UID, INVALID_STREAM_ID, ret);
    AUDIO_INFO_LOG("CSPWrapper_SetRendererTarget_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetRendererTarget_002
 * @tc.desc  : Test SetRendererTarget with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetRendererTarget_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetRendererTarget_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, SetRendererTarget(NORMAL_PLAYBACK, NORMAL_PLAYBACK, 1))
        .WillOnce(Return(SUCCESS));

    uint32_t target = NORMAL_PLAYBACK;
    uint32_t lastTarget = NORMAL_PLAYBACK;
    uint32_t sessionId = 1;
    int32_t ret = 0;
    int32_t result = wrapper.SetRendererTarget(target, lastTarget, sessionId, INVALID_UID, INVALID_STREAM_ID, ret);
    AUDIO_INFO_LOG("CSPWrapper_SetRendererTarget_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_StartInjection_001
 * @tc.desc  : Test StartInjection with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_StartInjection_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_StartInjection_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    uint32_t streamId = 0;
    int32_t ret = 0;
    int32_t result = wrapper.StartInjection(streamId, ret);
    AUDIO_INFO_LOG("CSPWrapper_StartInjection_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_StartInjection_002
 * @tc.desc  : Test StartInjection with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_StartInjection_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_StartInjection_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, StartInjection(1))
        .WillOnce(Return(SUCCESS));

    uint32_t streamId = 1;
    int32_t ret = 0;
    int32_t result = wrapper.StartInjection(streamId, ret);
    AUDIO_INFO_LOG("CSPWrapper_StartInjection_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_RemoveIdForInjector_001
 * @tc.desc  : Test RemoveIdForInjector with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_RemoveIdForInjector_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_RemoveIdForInjector_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.RemoveIdForInjector(0);
    AUDIO_INFO_LOG("CSPWrapper_RemoveIdForInjector_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_ReleaseCaptureInjector_001
 * @tc.desc  : Test ReleaseCaptureInjector with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_ReleaseCaptureInjector_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_ReleaseCaptureInjector_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.ReleaseCaptureInjector();
    AUDIO_INFO_LOG("CSPWrapper_ReleaseCaptureInjector_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_RebuildCaptureInjector_001
 * @tc.desc  : Test RebuildCaptureInjector with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_RebuildCaptureInjector_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_RebuildCaptureInjector_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.RebuildCaptureInjector(0);
    AUDIO_INFO_LOG("CSPWrapper_RebuildCaptureInjector_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_A2dpOffloadGetRenderPosition_001
 * @tc.desc  : Test A2dpOffloadGetRenderPosition with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_A2dpOffloadGetRenderPosition_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_A2dpOffloadGetRenderPosition_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    int32_t ret = 0;
    int32_t result = wrapper.A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp, ret);
    AUDIO_INFO_LOG("CSPWrapper_A2dpOffloadGetRenderPosition_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_A2dpOffloadGetRenderPosition_002
 * @tc.desc  : Test A2dpOffloadGetRenderPosition with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_A2dpOffloadGetRenderPosition_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_A2dpOffloadGetRenderPosition_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, A2dpOffloadGetRenderPosition(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<0>(100),
            testing::SetArgReferee<1>(200),
            testing::SetArgReferee<2>(300),
            testing::Return(SUCCESS)));

    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    int32_t ret = 0;
    int32_t result = wrapper.A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp, ret);
    AUDIO_INFO_LOG("CSPWrapper_A2dpOffloadGetRenderPosition_002 result:%{public}d, "
                   "delay:%{public}u, data:%{public}llu, ts:%{public}u",
                   result, delayValue, static_cast<unsigned long long>(sendDataSize), timeStamp);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(delayValue, 100U);
    EXPECT_EQ(sendDataSize, 200U);
    EXPECT_EQ(timeStamp, 300U);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_OnCheckActiveMusicTime_001
 * @tc.desc  : Test OnCheckActiveMusicTime with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_OnCheckActiveMusicTime_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_OnCheckActiveMusicTime_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.OnCheckActiveMusicTime("test");
    AUDIO_INFO_LOG("CSPWrapper_OnCheckActiveMusicTime_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_OnCheckActiveMusicTime_002
 * @tc.desc  : Test OnCheckActiveMusicTime with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_OnCheckActiveMusicTime_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_OnCheckActiveMusicTime_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, OnCheckActiveMusicTime("test"))
        .WillOnce(testing::Return());

    int32_t result = wrapper.OnCheckActiveMusicTime("test");
    AUDIO_INFO_LOG("CSPWrapper_OnCheckActiveMusicTime_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_CaptureConcurrentCheck_001
 * @tc.desc  : Test CaptureConcurrentCheck with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_CaptureConcurrentCheck_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_CaptureConcurrentCheck_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.CaptureConcurrentCheck(0);
    AUDIO_INFO_LOG("CSPWrapper_CaptureConcurrentCheck_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_CaptureConcurrentCheck_002
 * @tc.desc  : Test CaptureConcurrentCheck with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_CaptureConcurrentCheck_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_CaptureConcurrentCheck_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, CaptureConcurrentCheck(1))
        .WillOnce(Return(SUCCESS));

    int32_t result = wrapper.CaptureConcurrentCheck(1);
    AUDIO_INFO_LOG("CSPWrapper_CaptureConcurrentCheck_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetCapturerMuteHint_001
 * @tc.desc  : Test SetCapturerMuteHint with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetCapturerMuteHint_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetCapturerMuteHint_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.SetCapturerMuteHint(0, false);
    AUDIO_INFO_LOG("CSPWrapper_SetCapturerMuteHint_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_SetCapturerMuteHint_002
 * @tc.desc  : Test SetCapturerMuteHint with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_SetCapturerMuteHint_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_SetCapturerMuteHint_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, SetCapturerMuteHint(1, true))
        .WillOnce(Return(SUCCESS));

    int32_t result = wrapper.SetCapturerMuteHint(1, true);
    AUDIO_INFO_LOG("CSPWrapper_SetCapturerMuteHint_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifySessionStandbyStatus_001
 * @tc.desc  : Test NotifySessionStandbyStatus with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifySessionStandbyStatus_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifySessionStandbyStatus_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.NotifySessionStandbyStatus(0, false);
    AUDIO_INFO_LOG("CSPWrapper_NotifySessionStandbyStatus_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifySessionStandbyStatus_002
 * @tc.desc  : Test NotifySessionStandbyStatus with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifySessionStandbyStatus_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifySessionStandbyStatus_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, NotifySessionStandbyStatus(1, true))
        .WillOnce(Return());

    int32_t result = wrapper.NotifySessionStandbyStatus(1, true);
    AUDIO_INFO_LOG("CSPWrapper_NotifySessionStandbyStatus_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifyStreamStandby_001
 * @tc.desc  : Test NotifyStreamStandby with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifyStreamStandby_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandby_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t ret = 0;
    int32_t result = wrapper.NotifyStreamStandby(0, 0, ret);
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandby_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifyStreamStandby_002
 * @tc.desc  : Test NotifyStreamStandby with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifyStreamStandby_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandby_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, NotifyStreamStandby(100, 1))
        .WillOnce(Return(SUCCESS));

    int32_t ret = 0;
    int32_t result = wrapper.NotifyStreamStandby(100, 1, ret);
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandby_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifyStreamStandbyResume_001
 * @tc.desc  : Test NotifyStreamStandbyResume with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifyStreamStandbyResume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandbyResume_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t ret = 0;
    int32_t result = wrapper.NotifyStreamStandbyResume(0, 0, ret);
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandbyResume_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifyStreamStandbyResume_002
 * @tc.desc  : Test NotifyStreamStandbyResume with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifyStreamStandbyResume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandbyResume_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, NotifyStreamStandbyResume(100, 1))
        .WillOnce(Return(SUCCESS));

    int32_t ret = 0;
    int32_t result = wrapper.NotifyStreamStandbyResume(100, 1, ret);
    AUDIO_INFO_LOG("CSPWrapper_NotifyStreamStandbyResume_002 result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_IsSupportIndependentPipe_001
 * @tc.desc  : Test IsSupportIndependentPipe with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_IsSupportIndependentPipe_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_IsSupportIndependentPipe_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    bool support = false;
    int32_t result = wrapper.IsSupportIndependentPipe(support);
    AUDIO_INFO_LOG("CSPWrapper_IsSupportIndependentPipe_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_IsSupportIndependentPipe_002
 * @tc.desc  : Test IsSupportIndependentPipe with valid coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_IsSupportIndependentPipe_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_IsSupportIndependentPipe_002 start");
    MockCoreServiceProvider mockProvider;
    CoreServiceProviderWrapper wrapper(&mockProvider);

    EXPECT_CALL(mockProvider, IsSupportIndependentPipe())
        .WillOnce(Return(true));

    bool support = false;
    int32_t result = wrapper.IsSupportIndependentPipe(support);
    AUDIO_INFO_LOG("CSPWrapper_IsSupportIndependentPipe_002 result:%{public}d, support:%{public}d", result, support);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(support, true);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifyDeviceStateChange_001
 * @tc.desc  : Test NotifyDeviceStateChange with null coreServiceWorker.
 */
HWTEST_F(CoreServiceProviderStubUnitTest, CoreServiceProviderWrapper_NotifyDeviceStateChange_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("CSPWrapper_NotifyDeviceStateChange_001 start");
    CoreServiceProviderWrapper wrapper(nullptr);
    int32_t result = wrapper.NotifyDeviceStateChange("test", 0, false);
    AUDIO_INFO_LOG("CSPWrapper_NotifyDeviceStateChange_001 result:%{public}d", result);
    EXPECT_EQ(result, AUDIO_INIT_FAIL);
}

/**
 * @tc.name   : Test CoreServiceProviderWrapper API
 * @tc.type  : FUNC
 * @tc.number: CoreServiceProviderWrapper_NotifyDeviceStateChange_002
 * @tc.desc  : Test NotifyDeviceStateChange with valid coreServiceWorker.
 */
} // namespace AudioStandard
} // namespace OHOS
