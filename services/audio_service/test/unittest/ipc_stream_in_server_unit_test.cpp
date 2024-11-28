/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "ipc_stream_in_server.h"
#include <memory>
#include "ipc_stream_stub.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "renderer_in_server.h"
#include "capturer_in_server.h"
#include <cinttypes>
#include "audio_service_log.h"
#include "audio_errors.h"
#include "ipc_stream.h"
#include "message_parcel.h"
#include "parcel.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class IpcStreamInServerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void IpcStreamInServerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void IpcStreamInServerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void IpcStreamInServerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void IpcStreamInServerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_001
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_001, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    auto ret = ipcStreamInServerRet.GetCapturer();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_002
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_002, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    std::shared_ptr<OHAudioBuffer> buffer;
    auto ret = ipcStreamInServerRet.ResolveBuffer(buffer);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_003
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_003, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret1 = ipcStreamInServerRet.UpdatePosition();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret1 = ipcStreamInServerRet.UpdatePosition();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret1 = ipcStreamInServerRet.UpdatePosition();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_004
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_004, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret1 = ipcStreamInServerRet.Config();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);
    EXPECT_EQ(ipcStreamInServerRet.mode_, AUDIO_MODE_RECORD);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_005
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_005, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    uint32_t sessionIdRet;
    auto ret = ipcStreamInServerRet.GetAudioSessionID(sessionIdRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.GetAudioSessionID(sessionIdRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.GetAudioSessionID(sessionIdRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_006
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_006, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    auto ret = ipcStreamInServerRet.Start();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.Start();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.Start();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_007
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_007, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ret = ipcStreamInServerRet.Stop();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_008
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_008, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet1(configRet, modeRet);
    uint64_t framePosRet = 0;
    uint64_t timestampRet = 0;
    uint64_t latency = 0;
    auto ret1 = ipcStreamInServerRet1.GetAudioPosition(framePosRet, timestampRet, latency);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet2(configRet, modeRet);
    ipcStreamInServerRet2.rendererInServer_ = std::make_shared<RendererInServer>(
        ipcStreamInServerRet2.config_,
        ipcStreamInServerRet2.streamListenerHolder_);
    auto ret2 = ipcStreamInServerRet1.GetAudioPosition(framePosRet, timestampRet, latency);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_009
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_009, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    uint64_t latency;

    auto ret = ipcStreamInServerRet.GetLatency(latency);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.GetLatency(latency);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.GetLatency(latency);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_010
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_010, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    float volumeRet = 0.5;

    auto ret1 = ipcStreamInServerRet.SetLowPowerVolume(volumeRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    auto ret2 = ipcStreamInServerRet.SetLowPowerVolume(volumeRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_011
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_011, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    float volumeRet = 0.5;

    auto ret = ipcStreamInServerRet.GetLowPowerVolume(volumeRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_  = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.GetLowPowerVolume(volumeRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    ret = ipcStreamInServerRet.GetLowPowerVolume(volumeRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_012
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_012, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret1 = ipcStreamInServerRet.SetAudioEffectMode(EFFECT_NONE);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    auto ret2 = ipcStreamInServerRet.SetAudioEffectMode(EFFECT_NONE);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_013
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_013, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    int32_t effectModeRet = EFFECT_NONE;

    auto ret1 = ipcStreamInServerRet.GetAudioEffectMode(effectModeRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.GetAudioEffectMode(effectModeRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    auto ret3 = ipcStreamInServerRet.GetAudioEffectMode(effectModeRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_014
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_014, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    int32_t privacyTypeRet = EFFECT_NONE;

    auto ret1 = ipcStreamInServerRet.SetPrivacyType(privacyTypeRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.SetPrivacyType(privacyTypeRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    auto ret3 = ipcStreamInServerRet.SetPrivacyType(privacyTypeRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_015
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_015, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    int32_t privacyTypeRet = EFFECT_NONE;

    auto ret1 = ipcStreamInServerRet.GetPrivacyType(privacyTypeRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.GetPrivacyType(privacyTypeRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.GetPrivacyType(privacyTypeRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_016
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_016, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    int32_t stateRet = EFFECT_NONE;
    bool isAppBackRet = false;

    auto ret1 = ipcStreamInServerRet.SetOffloadMode(stateRet, isAppBackRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.SetOffloadMode(stateRet, isAppBackRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.SetOffloadMode(stateRet, isAppBackRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_017
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_017, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret1 = ipcStreamInServerRet.UnsetOffloadMode();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.UnsetOffloadMode();
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.UnsetOffloadMode();
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_018
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_018, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    uint64_t timestampRet = EFFECT_NONE;
    uint64_t paWriteIndexRet = EFFECT_NONE;
    uint64_t cacheTimeDspRet = EFFECT_NONE;
    uint64_t cacheTimePaRet = EFFECT_NONE;

    auto ret1 = ipcStreamInServerRet.GetOffloadApproximatelyCacheTime(timestampRet, paWriteIndexRet,
        cacheTimeDspRet, cacheTimePaRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.GetOffloadApproximatelyCacheTime(timestampRet, paWriteIndexRet,
        cacheTimeDspRet, cacheTimePaRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.GetOffloadApproximatelyCacheTime(timestampRet, paWriteIndexRet,
        cacheTimeDspRet, cacheTimePaRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_020
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_020, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    bool spatializationEnabledRet = false;
    bool headTrackingEnabledRet = false;

    auto ret1 = ipcStreamInServerRet.UpdateSpatializationState(spatializationEnabledRet, headTrackingEnabledRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.UpdateSpatializationState(spatializationEnabledRet, headTrackingEnabledRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.UpdateSpatializationState(spatializationEnabledRet, headTrackingEnabledRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_021
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_021, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret1 = ipcStreamInServerRet.GetStreamManagerType();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.GetStreamManagerType();
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.GetStreamManagerType();
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_022
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_022, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    bool onRet = false;

    auto ret1 = ipcStreamInServerRet.SetSilentModeAndMixWithOthers(onRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.SetSilentModeAndMixWithOthers(onRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.SetSilentModeAndMixWithOthers(onRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_023
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_023, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret1 = ipcStreamInServerRet.SetClientVolume();
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.SetClientVolume();
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.SetClientVolume();
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_024
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_024, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    uint32_t tidRet = 0;
    std::string clientBundleNameRet;

    auto ret = ipcStreamInServerRet.RegisterThreadPriority(tidRet, clientBundleNameRet);
    EXPECT_EQ(ret, SUCCESS);

    ipcStreamInServerRet.clientThreadPriorityRequested_ = true;
    ret = ipcStreamInServerRet.RegisterThreadPriority(tidRet, clientBundleNameRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_025
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_025, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    int32_t rateRet = 0;
    uint64_t famePosRet = 0;
    uint64_t timestampRet = 0;
    uint64_t latency = 0;

    auto ret = ipcStreamInServerRet.SetRate(rateRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    auto ret1 = ipcStreamInServerRet.GetAudioPosition(famePosRet, timestampRet, latency);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.SetRate(rateRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    ret = ipcStreamInServerRet.SetRate(rateRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_026
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_026, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    uint64_t famePosRet = 0;
    uint64_t timestampRet = 0;

    auto ret = ipcStreamInServerRet.GetAudioTime(famePosRet, timestampRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.GetAudioTime(famePosRet, timestampRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.GetAudioTime(famePosRet, timestampRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_027
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_027, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    AudioPlaybackCaptureConfig config;

    auto ret = ipcStreamInServerRet.UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.UpdatePlaybackCaptureConfig(config);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_028
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_028, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    bool stopFlagRet = false;

    auto ret = ipcStreamInServerRet.Drain(stopFlagRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.Drain(stopFlagRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    ret = ipcStreamInServerRet.Drain(stopFlagRet);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_029
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_029, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_030
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_030, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.Release();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ret = ipcStreamInServerRet.Release();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    ret = ipcStreamInServerRet.Release();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_031
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_031, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.Stop();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.Stop();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.Stop();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_032
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_032, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_033
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_033, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    std::shared_ptr<OHAudioBuffer> buffer;

    auto ret = ipcStreamInServerRet.ResolveBuffer(buffer);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.ResolveBuffer(buffer);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.ResolveBuffer(buffer);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_034
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_034, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.GetCapturer();
    EXPECT_EQ(ret, nullptr);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.GetCapturer();
    EXPECT_EQ(ret, nullptr);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    ipcStreamInServerRet.ConfigCapturer();
    EXPECT_NE(ipcStreamInServerRet.capturerInServer_, nullptr);
    ret = ipcStreamInServerRet.GetCapturer();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_035
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_035, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_PLAYBACK;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);

    auto ret = ipcStreamInServerRet.GetRenderer();
    EXPECT_EQ(ret, nullptr);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ret = ipcStreamInServerRet.GetRenderer();
    EXPECT_EQ(ret, nullptr);

    ipcStreamInServerRet.ConfigRenderer();
    EXPECT_NE(ipcStreamInServerRet.rendererInServer_, nullptr);
    ret = ipcStreamInServerRet.GetRenderer();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_036
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_036, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    bool onRet = false;

    auto ret1 = ipcStreamInServerRet.SetMute(onRet);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.SetMute(onRet);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.SetMute(onRet);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test IpcStreamInServer API
 * @tc.type  : FUNC
 * @tc.number: IpcStreamInServer_037
 * @tc.desc  : Test IpcStreamInServer interface.
 */
HWTEST(IpcStreamInServerUnitTest, IpcStreamInServer_037, TestSize.Level1)
{
    AudioProcessConfig configRet;
    AudioMode modeRet = AUDIO_MODE_RECORD;
    IpcStreamInServer ipcStreamInServerRet(configRet, modeRet);
    float duckFactor = 0.2f;

    auto ret1 = ipcStreamInServerRet.SetDuckFactor(duckFactor);
    EXPECT_EQ(ret1, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_PLAYBACK;
    auto ret2 = ipcStreamInServerRet.SetDuckFactor(duckFactor);
    EXPECT_EQ(ret2, ERR_OPERATION_FAILED);

    ipcStreamInServerRet.mode_ = AUDIO_MODE_RECORD;
    ipcStreamInServerRet.ConfigRenderer();
    auto ret3 = ipcStreamInServerRet.SetDuckFactor(duckFactor);
    EXPECT_EQ(ret3, ERR_OPERATION_FAILED);
}
}
}
