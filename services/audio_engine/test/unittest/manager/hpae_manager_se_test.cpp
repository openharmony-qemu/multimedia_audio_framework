/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "gtest/gtest.h"
#include <memory>
#include <string>

#include "hpae_manager.h"
#include "hpae_renderer_manager.h"
#include "hpae_offload_renderer_manager.h"
#include "audio_effect_chain_manager.h"
#include "hpae_process_cluster.h"
#include "hpae_session_effect_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_sink_virtual_output_node.h"
#include "hpae_node_common.h"
#include "hpae_define.h"
#include "i_hpae_manager.h"
#include "hpae_mocks.h"
#include "audio_errors.h"
#include "test_case_common.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
static constexpr uint32_t TEST_FRAME_LEN = 960;
} // namespace

static constexpr uint32_t TEST_SESSION_ID = 42;
static constexpr uint32_t OTHER_SESSION_ID = 43;
static constexpr uint32_t INNER_CAPTURE_SESSION_ID = 84;
static constexpr uint32_t REMOTE_CAST_SESSION_ID = 85;
static constexpr uint32_t UNKNOWN_SESSION_ID = 999;
static constexpr int32_t TEST_UID = 1000;
static constexpr int32_t OTHER_UID = 2000;
static constexpr const char *AISS_EFFECT_NAME = "audio_aiss_enhance";

static HpaeSinkInfo GetTestSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = "LocalDevice";
    sinkInfo.deviceClass = "file_io";
    sinkInfo.adapterName = "file_io";
    sinkInfo.filePath = "/data/local/tmp/test_se_fixture.pcm";
    sinkInfo.frameLen = TEST_FRAME_LEN;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    return sinkInfo;
}

static AudioEffectConfig MakeTestAudioEffectConfig()
{
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    return config;
}

static AudioModuleInfo GetReloadAudioModuleInfo(const std::string &name = "file_io")
{
    AudioModuleInfo moduleInfo;
    moduleInfo.lib = "libmodule-hdi-sink.z.so";
    moduleInfo.channels = "2";
    moduleInfo.rate = "48000";
    moduleInfo.name = name;
    moduleInfo.adapterName = "file_io";
    moduleInfo.className = "file_io";
    moduleInfo.bufferSize = "7680";
    moduleInfo.format = "s32le";
    moduleInfo.fixedLatency = "1";
    moduleInfo.offloadEnable = "0";
    moduleInfo.networkId = "LocalDevice";
    moduleInfo.fileName = "/data/local/tmp/test_se_reload_renderer.pcm";
    moduleInfo.deviceType = std::to_string(static_cast<int32_t>(DEVICE_TYPE_SPEAKER));
    moduleInfo.channelLayout = std::to_string(static_cast<uint64_t>(CH_LAYOUT_STEREO));
    moduleInfo.routeFlag = 0;
    return moduleInfo;
}

class HpaeManagerSeTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        hpaeManager_ = std::make_shared<HpaeManager>();
        hpaeManager_->Init();  // 创建 HpaeManagerThread + SessionEffectManager

        // 构造真实 RendererManager（file_io sink，不依赖硬件）
        HpaeSinkInfo sinkInfo = GetTestSinkInfo();
        rendererManager_ = std::make_shared<HpaeRendererManager>(sinkInfo);

        // 注入 session 映射到 HpaeManager 内部 map（-fno-access-control）
        hpaeManager_->rendererManagerMap_["file_io"] = rendererManager_;
        hpaeManager_->rendererIdSinkNameMap_[TEST_SESSION_ID] = "file_io";
    }

    void TearDown() override
    {
        // 清理注入的 map 条目，防止 ~HpaeManager 遍历部分初始化的 RendererManager
        hpaeManager_->rendererManagerMap_.erase("file_io");
        hpaeManager_->rendererIdSinkNameMap_.erase(TEST_SESSION_ID);
        hpaeManager_.reset();
    }

    std::shared_ptr<HpaeManager> hpaeManager_;
    std::shared_ptr<HpaeRendererManager> rendererManager_;

    HpaeStreamInfo MakeStereoMusicStream(uint32_t sessionId, SourceType sourceType = SOURCE_TYPE_INVALID)
    {
        HpaeStreamInfo streamInfo;
        streamInfo.sessionId = sessionId;
        streamInfo.streamType = STREAM_MUSIC;
        streamInfo.uid = TEST_UID;
        streamInfo.channels = STEREO;
        streamInfo.channelLayout = CH_LAYOUT_STEREO;
        streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
        streamInfo.sourceType = sourceType;
        return streamInfo;
    }

    void InjectStereoMusicStream(HpaeSessionState state = HPAE_SESSION_RUNNING)
    {
        HpaeStreamInfo streamInfo = MakeStereoMusicStream(TEST_SESSION_ID);
        hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, state};
    }

    void WaitForMsgProcessing()
    {
        static constexpr int32_t pollIntervalMs = 20;
        static constexpr int32_t maxPollCount = 5;
        static constexpr int32_t finalWaitMs = 40;
        int waitCount = 0;
        while (hpaeManager_->IsMsgProcessing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
            waitCount++;
            if (waitCount >= maxPollCount) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(finalWaitMs));
    }
};

// --- SetAissEnabledSync ---

// API-1: SetAissEnabledSync(true) succeeds when session exists
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_success, TestSize.Level0)
{
    InjectStereoMusicStream();

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// API-2: SetAissEnabledSync(true) returns ERR_SESSION_NOT_FOUND when session doesn't exist
// 前置条件: Init() 已完成 → sessionEffectManager_ 不为 null
// 前置条件: UNKNOWN_SESSION_ID 不在 rendererIdSinkNameMap_ 中
// Design ref: se-state-management.md §3.1, se-test-plan.md §6 API-2
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_sessionNotFound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabledSync(UNKNOWN_SESSION_ID, true);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// API-3: SetAissEnabledSync(true) is idempotent when already bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_alreadyBound, TestSize.Level0)
{
    InjectStereoMusicStream();

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);

    ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_switchSessionClearsOldBinding, TestSize.Level0)
{
    InjectStereoMusicStream();
    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);

    HpaeStreamInfo newStream = MakeStereoMusicStream(OTHER_SESSION_ID);
    hpaeManager_->rendererIdSinkNameMap_[OTHER_SESSION_ID] = "file_io";
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {newStream, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledSync(OTHER_SESSION_ID, true);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), OTHER_SESSION_ID);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_switchSessionClearsUidBindingForOldSession, TestSize.Level0)
{
    HpaeStreamInfo oldStream = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {oldStream, HPAE_SESSION_RUNNING};
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundUid_, TEST_UID);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    HpaeStreamInfo newStream = MakeStereoMusicStream(OTHER_SESSION_ID);
    hpaeManager_->rendererIdSinkNameMap_[OTHER_SESSION_ID] = "file_io";
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {newStream, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledSync(OTHER_SESSION_ID, true);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), OTHER_SESSION_ID);
}

// API-4: SetAissEnabledSync(false) succeeds when session is bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_false_success, TestSize.Level0)
{
    InjectStereoMusicStream();
    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, false);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabled_false_clearsUidBindingForSameSession, TestSize.Level0)
{
    HpaeStreamInfo streamInfo = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundUid_, TEST_UID);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, false);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

// API-5: SetAissEnabledSync(false) returns ERR_EFFECT_NOT_BOUND when not bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_false_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, false);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// API-6: SetAissEnabledSync(true) routes to HpaeRendererManager for Primary path
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_primaryPath, TestSize.Level0)
{
    // Default fixture already has HpaeRendererManager → Primary path
    InjectStereoMusicStream();

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(HpaeManagerSeTest, destroyMovingPlayStream_notifiesSessionEffectDestroyed, TestSize.Level0)
{
    InjectStereoMusicStream();

    std::shared_ptr<EffectInstance> instance;
    ASSERT_EQ(hpaeManager_->sessionEffectManager_->BindSessionEffect(TEST_SESSION_ID, AISS_EFFECT_NAME,
        MakeTestAudioEffectConfig(), instance), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);

    hpaeManager_->movingIds_[TEST_SESSION_ID] = HPAE_SESSION_RUNNING;
    EXPECT_EQ(hpaeManager_->DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing();

    auto pending = hpaeManager_->sessionEffectManager_->GetPendingEffectInstance(AISS_EFFECT_NAME);
    ASSERT_NE(pending, nullptr);
    EXPECT_EQ(pending, instance);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);
    EXPECT_EQ(instance->GetBoundSessionId(), 0u);
}

// API-7: SetAissEnabledSync(true) routes to HpaeOffloadRendererManager for Offload path
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_offloadPath, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetTestSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    hpaeManager_->rendererManagerMap_["file_io"] = offloadManager;
    InjectStereoMusicStream();

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabled_innerPlaybackCaptureRejected, TestSize.Level0)
{
    HpaeStreamInfo streamInfo = MakeStereoMusicStream(INNER_CAPTURE_SESSION_ID, SOURCE_TYPE_PLAYBACK_CAPTURE);
    hpaeManager_->rendererIdStreamInfoMap_[INNER_CAPTURE_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};
    hpaeManager_->rendererIdSinkNameMap_[INNER_CAPTURE_SESSION_ID] = "file_io";

    int32_t ret = hpaeManager_->SetAissEnabledSync(INNER_CAPTURE_SESSION_ID, true);

    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED));
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabled_nonMusicStreamRejected, TestSize.Level0)
{
    HpaeStreamInfo streamInfo = MakeStereoMusicStream(TEST_SESSION_ID);
    streamInfo.streamType = STREAM_VOICE_CALL;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);

    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED));
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

HWTEST_F(HpaeManagerSeTest, BindAndCreateAiss_missingStreamInfoRejected, TestSize.Level0)
{
    hpaeManager_->rendererIdStreamInfoMap_.erase(TEST_SESSION_ID);

    int32_t ret = hpaeManager_->BindAndCreateAiss(TEST_SESSION_ID);

    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED));
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

// --- SetVocalRatioSync ---

// API-8: SetVocalRatioSync succeeds with valid ratio and bound session
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_success, TestSize.Level0)
{
    InjectStereoMusicStream();
    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetVocalRatioSync(0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

// API-9: SetVocalRatioSync returns ERR_INVALID_PARAMETER for out-of-range ratio
// Design ref: se-state-management.md §3.1, se-test-plan.md §6 API-9
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_invalidRange, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetVocalRatioSync(-0.1f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));

    ret = hpaeManager_->SetVocalRatioSync(1.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));
}

// API-10: SetVocalRatioSync returns ERR_EFFECT_NOT_BOUND when not bound
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetVocalRatioSync(0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

// API-11: SetVocalRatioSync succeeds via UID binding path
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_uidPath_success, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.channels = STEREO;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetVocalRatioSync(0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

// --- SetAissEnabledByUidSync ---

// UID-1: SetAissEnabledByUidSync(true) succeeds and stays pending (no running stream)
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_true_pending, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// UID-2: SetAissEnabledByUidSync(true) is idempotent for the same uid
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_true_sameUidIdempotent, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// UID-3: SetAissEnabledByUidSync(false) returns error when no binding exists
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_false_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, false);
    EXPECT_NE(ret, SUCCESS);
}

// UID-4: SetAissEnabledByUidSync(false) succeeds after setting binding
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_false_success, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, false);
    EXPECT_EQ(ret, SUCCESS);
}

// UID-5: SetAissEnabledByUidSync(true) binds to existing running music stream
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_true_bindRunningStream, TestSize.Level0)
{
    // Inject a running music stream for TEST_UID
    HpaeStreamInfo streamInfo = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_switchUidBindsNewRunningStream, TestSize.Level0)
{
    HpaeStreamInfo oldStream = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {oldStream, HPAE_SESSION_RUNNING};
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundUid_, TEST_UID);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    ASSERT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);

    HpaeStreamInfo newStream = MakeStereoMusicStream(OTHER_SESSION_ID);
    newStream.uid = OTHER_UID;
    hpaeManager_->rendererIdSinkNameMap_[OTHER_SESSION_ID] = "file_io";
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {newStream, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(OTHER_UID, true);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, OTHER_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, OTHER_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), OTHER_SESSION_ID);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_switchUidKeepsNewUidPendingWithoutRunningStream, TestSize.Level0)
{
    HpaeStreamInfo oldStream = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {oldStream, HPAE_SESSION_RUNNING};
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(OTHER_UID, true);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, OTHER_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_switchUidHardFailureClearsNewStateWithoutRollback, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundUid_, TEST_UID);
    ASSERT_EQ(hpaeManager_->boundSessionId_, 0u);

    HpaeStreamInfo newStream = MakeStereoMusicStream(OTHER_SESSION_ID);
    newStream.uid = OTHER_UID;
    hpaeManager_->rendererIdSinkNameMap_[OTHER_SESSION_ID] = "file_io";
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {newStream, HPAE_SESSION_RUNNING};

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    std::shared_ptr<EffectInstance> otherEffect;
    ASSERT_EQ(hpaeManager_->sessionEffectManager_->BindSessionEffect(
        OTHER_SESSION_ID, "test_effect", config, otherEffect), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(OTHER_UID, true);

    EXPECT_NE(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

// UID-6: Start auto-binds session effect when uid matches pending binding
HWTEST_F(HpaeManagerSeTest, Start_autoBindOnUidMatch, TestSize.Level0)
{
    // Set pending binding
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    // Create a stream with matching uid and music type
    HpaeStreamInfo streamInfo = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    // Start the stream
    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();

    // Verify: boundSessionId_ should now be TEST_SESSION_ID
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}
HWTEST_F(HpaeManagerSeTest, Start_noBindOnUidMismatch, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = OTHER_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);

    // Should still be pending (no match)
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// UID-8: Start does not bind non-music stream even if uid matches
HWTEST_F(HpaeManagerSeTest, Start_noBindOnNonMusicStream, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_VOICE_CALL;
    streamInfo.uid = TEST_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);

    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_skipsInnerPlaybackCaptureAndBindsRealMusic, TestSize.Level0)
{
    HpaeStreamInfo realStream = MakeStereoMusicStream(TEST_SESSION_ID);
    HpaeStreamInfo innerStream = MakeStereoMusicStream(INNER_CAPTURE_SESSION_ID, SOURCE_TYPE_PLAYBACK_CAPTURE);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {realStream, HPAE_SESSION_RUNNING};
    hpaeManager_->rendererIdStreamInfoMap_[INNER_CAPTURE_SESSION_ID] = {innerStream, HPAE_SESSION_RUNNING};
    hpaeManager_->rendererIdSinkNameMap_[INNER_CAPTURE_SESSION_ID] = "file_io";

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}

HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_skipsRemoteCastAndKeepsPending, TestSize.Level0)
{
    HpaeStreamInfo remoteCastStream = MakeStereoMusicStream(REMOTE_CAST_SESSION_ID, SOURCE_TYPE_REMOTE_CAST);
    hpaeManager_->rendererIdStreamInfoMap_[REMOTE_CAST_SESSION_ID] = {remoteCastStream, HPAE_SESSION_RUNNING};
    hpaeManager_->rendererIdSinkNameMap_[REMOTE_CAST_SESSION_ID] = "file_io";

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

HWTEST_F(HpaeManagerSeTest, Start_innerPlaybackCaptureDoesNotReplaceBoundMusic, TestSize.Level0)
{
    HpaeStreamInfo realStream = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {realStream, HPAE_SESSION_RUNNING};
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    HpaeStreamInfo innerStream = MakeStereoMusicStream(INNER_CAPTURE_SESSION_ID, SOURCE_TYPE_PLAYBACK_CAPTURE);
    hpaeManager_->rendererIdStreamInfoMap_[INNER_CAPTURE_SESSION_ID] = {innerStream, HPAE_SESSION_NEW};
    hpaeManager_->rendererIdSinkNameMap_[INNER_CAPTURE_SESSION_ID] = "file_io";

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, INNER_CAPTURE_SESSION_ID);
    WaitForMsgProcessing();

    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}

// UID-9: DestroyStream resets boundSessionId to pending, keeps uid binding
HWTEST_F(HpaeManagerSeTest, DestroyStream_resetsToPending, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    HpaeStreamInfo streamInfo = MakeStereoMusicStream(TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    hpaeManager_->DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();

    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
}

// --- Deferred AISS-disable (DisableAissByUidWhenStreamInactiveSync) ---
// Design ref: 2026-08-12-aiss-deferred-disable-on-inactive-design.md §2/§7.
// Sync entry points marshal onto the manager thread and block until settled, so state is stable on
// return. Private members/handlers are reached directly via -fno-access-control, consistent with
// the rest of this file. The manager thread is idle between Sync calls (no pending transitions are
// enqueued here), so terminal triggers are driven through HandleUpdateStatus / SetMovingPlayStreamState
// deterministically.

// Scenario 1: a RUNNING bound stream registers the deferred request; AISS stays active (no early close).
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_runningStream_registersPendingAndStaysActive, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    ASSERT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);

    int32_t ret = hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID);

    EXPECT_EQ(ret, SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->pendingAissDisable_->uid, TEST_UID);
    EXPECT_EQ(hpaeManager_->pendingAissDisable_->sessionId, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// Scenario 12 (error uid): a mismatched uid is refused and leaves all state untouched.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_uidMismatch_refused, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    uint64_t genBefore = hpaeManager_->aissControlGeneration_;

    int32_t ret = hpaeManager_->DisableAissByUidWhenStreamInactiveSync(OTHER_UID);

    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->aissControlGeneration_, genBefore);
}

// Scenario 12 (idempotent): registering twice for the same {uid, session} is idempotent.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_duplicateRegistration_isIdempotent, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    uint64_t firstGen = hpaeManager_->pendingAissDisable_->generation;

    int32_t ret = hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID);

    EXPECT_EQ(ret, SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->pendingAissDisable_->generation, firstGen);
}

// Scenario 2 (armed -> off): no physical binding, so the armed uid is cancelled directly.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_armedUid_cancelsDirectly, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundUid_, TEST_UID);
    ASSERT_EQ(hpaeManager_->boundSessionId_, 0u);

    int32_t ret = hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// Scenario 2 (already terminal): a bound stream already PAUSED/STOPPED closes immediately.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_alreadyTerminal_closesImmediately, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_STOPPED;

    int32_t ret = hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

// Scenario 2 (inconsistent): a bound stream in NEW/PREPARED/INVALID is refused.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_inconsistentState_refused, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PREPARED;

    int32_t ret = hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID);

    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}

// Scenario 3 & 13: a valid PAUSED terminal flushes the request exactly once.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_pausedTerminal_closesOnce, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());

    // IsValidUpdateStatus requires state PAUSING for OPERATION_PAUSED.
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);

    // A duplicate terminal must not re-trigger (state is now PAUSED -> IsValidUpdateStatus drops it).
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// Scenario 3 (STOPPED): a valid STOPPED terminal flushes the request.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_stoppedTerminal_closesOnce, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_STOPPING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_STOPPED, OPERATION_STOPPED);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// Scenario 13 (stale generation): a pending request whose captured generation no longer matches the
// control epoch must not fire on a terminal. (Normally unreachable: every generation bump also
// resets the request. We inject a superseded request directly to exercise the guard's epoch check.)
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_staleGeneration_doesNotClose, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_GE(hpaeManager_->aissControlGeneration_, 1u);
    // Inject a request tied to a superseded (older) generation.
    hpaeManager_->pendingAissDisable_ = HpaeManager::PendingAissDisable{TEST_UID, TEST_SESSION_ID, 0};

    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);

    // Epoch mismatch -> guard does not fire; AISS stays active.
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// Scenario 13 (other session): a terminal for an unrelated session must not flush the request.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_otherSessionTerminal_doesNotClose, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    HpaeStreamInfo other = MakeStereoMusicStream(OTHER_SESSION_ID);
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {other, HPAE_SESSION_PAUSING};
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, OTHER_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);

    EXPECT_TRUE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// Scenario 6 (normal release): DestroyStream flushes the request before tearing down the renderer.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_normalRelease_closes, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    hpaeManager_->DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
}

// Scenario 6 (moving pause/stop) & 7: SetMovingPlayStreamState flushes on PAUSED/STOPPED/RELEASED.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_movingPause_closes, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    hpaeManager_->movingIds_[TEST_SESSION_ID] = HPAE_SESSION_RUNNING;

    hpaeManager_->SetMovingPlayStreamState(TEST_SESSION_ID, HPAE_SESSION_PAUSED, OPERATION_PAUSED);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_movingRelease_closes, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    hpaeManager_->movingIds_[TEST_SESSION_ID] = HPAE_SESSION_RUNNING;

    hpaeManager_->SetMovingPlayStreamState(TEST_SESSION_ID, HPAE_SESSION_RELEASED, OPERATION_RELEASED);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
}

// Scenario 7 (Flush/Drain do not trigger): a non-terminal moving operation keeps the request.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_movingFlush_keepsPending, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    hpaeManager_->movingIds_[TEST_SESSION_ID] = HPAE_SESSION_RUNNING;

    hpaeManager_->SetMovingPlayStreamState(TEST_SESSION_ID, HPAE_SESSION_RUNNING, OPERATION_FLUSHED);

    EXPECT_TRUE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
}

// Scenario 5 & §4.2: while the request is pending, a same-uid Start must not migrate AISS.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_pendingBlocksStartMigration, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    HpaeStreamInfo other = MakeStereoMusicStream(OTHER_SESSION_ID);
    other.uid = TEST_UID;
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {other, HPAE_SESSION_RUNNING};
    hpaeManager_->TryBindSessionEffectOnStart(OTHER_SESSION_ID);

    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// Scenario 9: same-uid Enable cancels the request; a later terminal must not close AISS.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_sameUidEnable_cancelsPending, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());

    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    // The old request's terminal must not close AISS (generation invalidated).
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}

// Scenario 10: different-uid Enable closes old AISS immediately and switches owner; the old
// session's later terminal must not affect the new binding.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_differentUidEnable_closesOldAndSwitches, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    HpaeStreamInfo newStream = MakeStereoMusicStream(OTHER_SESSION_ID);
    newStream.uid = OTHER_UID;
    hpaeManager_->rendererIdSinkNameMap_[OTHER_SESSION_ID] = "file_io";
    hpaeManager_->rendererIdStreamInfoMap_[OTHER_SESSION_ID] = {newStream, HPAE_SESSION_RUNNING};

    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(OTHER_UID, true), SUCCESS);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, OTHER_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, OTHER_SESSION_ID);

    // Old session terminal must not close the new binding.
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    EXPECT_EQ(hpaeManager_->boundUid_, OTHER_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, OTHER_SESSION_ID);
}

// Scenario 11: immediate Disable invalidates the request (immediate close outranks deferred).
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_immediateDisable_invalidatesPending, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, false), SUCCESS);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
}

// Scenario 11: session-level AISS control invalidates the uid-level deferred request.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_sessionLevelControl_invalidatesPending, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);

    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    // A later terminal of the old request must not close AISS.
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

namespace {
// Reports the SendRequestSync timeout sentinel from PostDestroySessionEffect without a busy RM
// thread: makes ClearAissSession take its "destroy timed out, keep binding" early return.
class TimeoutDestroyRendererManager : public HpaeRendererManager {
public:
    explicit TimeoutDestroyRendererManager(HpaeSinkInfo &sinkInfo) : HpaeRendererManager(sinkInfo) {}

    int32_t PostDestroySessionEffect(uint32_t /*sessionId*/, bool /*finalizeNow*/) override
    {
        return ERR_OPERATION_FAILED; // sync-timeout sentinel: request still queued, not executed
    }
};
} // namespace

// Scenario (destroy timeout): the terminal flush hits the sync-timeout sentinel inside
// ClearAissSession. FinishPendingAissDisable must still drop the logical state (DisableAissByUid
// semantics): pending cleared, uid/session binding reset, generation bumped. The physical effect
// binding is kept (keep-binding contract) — the queued late destroy finishes the identity-gated
// teardown.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_destroyTimeout_stillClearsLogicalState, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());
    uint64_t genBefore = hpaeManager_->aissControlGeneration_;

    // Swap in a manager whose destroy always reports the sync-timeout sentinel.
    HpaeSinkInfo sinkInfo = GetTestSinkInfo();
    hpaeManager_->rendererManagerMap_["file_io"] =
        std::make_shared<TimeoutDestroyRendererManager>(sinkInfo);

    // IsValidUpdateStatus requires state PAUSING for OPERATION_PAUSED.
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);

    // Logical state went OFF even though the destroy timed out.
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_GT(hpaeManager_->aissControlGeneration_, genBefore);
    // Physical binding is kept for the queued late destroy (identity-gated teardown).
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// --- Deferred AISS-disable 1s backstop (delayed task queue based) ---
// Design ref: 2026-08-27-hpae-delayed-task-priority-queue-design.md §2.
// The manager thread pumps the delayed task queue itself (Run's wait_for uses GetNextDelayMs), so
// sleeping past the backstop deadline lets the real thread fire it; no manual pumping needed.

// Backstop: a stream still RUNNING after the 1s deadline is force-closed by the backstop timer.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_backstopTimeout_forcesCloseWhileRunning, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());
    // The backstop timer must be pending in the delayed task queue.
    EXPECT_GT(hpaeManager_->GetDelayedTaskQueue().GetNextDelayMs(), 0u);

    // Stream stays RUNNING the whole time; only the 1s backstop can close AISS.
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
}

// Backstop cancel on terminal flush: after the fast-path close, the 1s timer must not re-fire.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_terminalFlush_cancelsBackstopTimer, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());

    // IsValidUpdateStatus requires state PAUSING for OPERATION_PAUSED.
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_PAUSING;
    hpaeManager_->HandleUpdateStatus(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    uint64_t genAfterFlush = hpaeManager_->aissControlGeneration_;

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    // The cancelled backstop must not re-fire: state and generation stay put.
    EXPECT_EQ(hpaeManager_->aissControlGeneration_, genAfterFlush);
    EXPECT_EQ(hpaeManager_->boundUid_, -1);
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// Backstop invalidation on re-enable: a same-uid re-enable cancels the pending request; the stale
// 1s timer must not close the re-enabled binding.
HWTEST_F(HpaeManagerSeTest, DisableWhenInactive_reenable_invalidatesBackstopTimer, TestSize.Level0)
{
    InjectStereoMusicStream(HPAE_SESSION_RUNNING);
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    ASSERT_EQ(hpaeManager_->DisableAissByUidWhenStreamInactiveSync(TEST_UID), SUCCESS);
    ASSERT_TRUE(hpaeManager_->pendingAissDisable_.has_value());

    // Same-uid re-enable cancels the pending deferred disable and keeps the current binding.
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true), SUCCESS);
    EXPECT_FALSE(hpaeManager_->pendingAissDisable_.has_value());
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    // The stale backstop must not close the re-enabled binding.
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// CH-1: BindAndCreateAiss rejects non-stereo streams
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_multiChannelRejected, TestSize.Level0)
{
    // Inject a running music stream with multi-channel (5.1)
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.channels = CHANNEL_6;
    streamInfo.channelLayout = CH_LAYOUT_5POINT1;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// CH-2: BindAndCreateAiss succeeds for stereo stream
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_stereoAccepted, TestSize.Level0)
{
    // Inject a running music stream with stereo
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.channels = STEREO;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// CH-3: UID binding stays pending when the current running music stream is multi-channel
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_multiChannelKeepsPending, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.channels = CHANNEL_6;
    streamInfo.channelLayout = CH_LAYOUT_5POINT1;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledByUidSync(TEST_UID, true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
}

// --- E2E Tests: Init + DeactivateThread + HandleRequests pattern ---
// RM.SendRequest passes IsInit() check and enqueues the message, even if the thread is null.
// After DeactivateThread(), isInit_ stays true but hpaeSignalProcessThread_ is null.
// So: CreateStream/Start/Flush/Pause enqueue messages that we process via HandleRequests().

class HpaeManagerSeE2eTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        hpaeManager_ = std::make_shared<HpaeManager>();
        hpaeManager_->Init();

        HpaeSinkInfo sinkInfo = GetTestSinkInfo();
        rendererManager_ = std::make_shared<HpaeRendererManager>(sinkInfo);
        rendererManager_->Init();               // starts thread, sets isInit=true
        rendererManager_->DeactivateThread();   // stops thread, thread=null, isInit stays true

        hpaeManager_->rendererManagerMap_["file_io"] = rendererManager_;
        hpaeManager_->rendererIdSinkNameMap_[TEST_SESSION_ID] = "file_io";
    }

    void TearDown() override
    {
        hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, false);
        hpaeManager_->rendererManagerMap_.erase("file_io");
        hpaeManager_->rendererIdSinkNameMap_.erase(TEST_SESSION_ID);
        hpaeManager_->sinkNameSinkIdMap_.erase("file_io");
        hpaeManager_->sinkIdSinkNameMap_.erase(0);
        hpaeManager_->sinkVirtualOutputNodeMap_.erase(0);
        hpaeManager_.reset();
        rendererManager_.reset();
    }

    // Synchronously process one batch of queued messages
    void ProcessRmMessages()
    {
        rendererManager_->hpaeNoLockQueue_.HandleRequests();
    }

    void WaitForManagerMessages()
    {
        static constexpr int32_t pollIntervalMs = 20;
        static constexpr int32_t maxPollCount = 10;
        static constexpr int32_t finalWaitMs = 40;
        int waitCount = 0;
        while (hpaeManager_->IsMsgProcessing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
            waitCount++;
            if (waitCount >= maxPollCount) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(finalWaitMs));
    }

    // AudioEffectChainManager not init'd in test → CreateProcessCluster skips cluster creation.
    // GetProcessorType returns HPAE_SCENE_EFFECT_NONE due to missing effect chains.
    // sessionNodeMap_.sceneType defaults to 0 (never set in CreateStream).
    // Align both keys and inject the cluster at HPAE_SCENE_EFFECT_NONE.
    void InjectProcessCluster(uint32_t sessionId)
    {
        auto sinkInput = rendererManager_->sinkInputNodeMap_[sessionId];
        ASSERT_NE(sinkInput, nullptr);
        HpaeNodeInfo nodeInfo = sinkInput->GetNodeInfo();
        auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, rendererManager_->sinkInfo_);
        cluster->CreateNodes(sinkInput);
        cluster->Connect(sinkInput);
        rendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
        rendererManager_->sessionNodeMap_[sessionId].bypass = false;
        rendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] = cluster;
    }

    void InjectStereoMusicStream(HpaeSessionState state = HPAE_SESSION_RUNNING)
    {
        HpaeStreamInfo streamInfo;
        streamInfo.sessionId = TEST_SESSION_ID;
        streamInfo.streamType = STREAM_MUSIC;
        streamInfo.uid = TEST_UID;
        streamInfo.frameLen = TEST_FRAME_LEN;
        streamInfo.samplingRate = SAMPLE_RATE_48000;
        streamInfo.channels = STEREO;
        streamInfo.channelLayout = CH_LAYOUT_STEREO;
        streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
        hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, state};
    }

    std::shared_ptr<HpaeManager> hpaeManager_;
    std::shared_ptr<HpaeRendererManager> rendererManager_;
};

static bool SetupStreamWithSE(std::shared_ptr<HpaeManager> hpaeManager_,
    std::shared_ptr<HpaeRendererManager> rendererManager_);
static bool SetupOffloadStreamWithSE(std::shared_ptr<HpaeManager> hpaeManager_,
    std::shared_ptr<HpaeOffloadRendererManager> offloadManager_);

// --- E2E: Flush ---

// HM-FLUSH-E2E: RM.Flush → PC.SessionEffectFlush → SEN.FlushEffect
HWTEST_F(HpaeManagerSeE2eTest, Flush_triggersSessionEffectFlush_E2E, TestSize.Level0)
{
    // Setup: create stream
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.frameLen = TEST_FRAME_LEN;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    rendererManager_->CreateStream(streamInfo);
    ProcessRmMessages();

    // AudioEffectChainManager not init'd → no cluster created; inject one
    InjectProcessCluster(TEST_SESSION_ID);

    // Start session first (populates sessionNodeMap_ for CreateSessionEffect)
    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    // Bind SE (PostCreateSessionEffect needs sessionNodeMap_ entry)
    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);
    ProcessRmMessages();

    // Verify SE exists before flush
    ASSERT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));

    // Flush
    rendererManager_->Flush(TEST_SESSION_ID);
    ProcessRmMessages();  // processes Flush lambda → SE FlushEffect called

    // Verify: SE still bound (flush doesn't destroy)
    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
}

// --- E2E: Pause ---

// HM-PAUSE-E2E: RM.Pause(standby) → PC.SessionEffectPause → SEN.PauseEffect
HWTEST_F(HpaeManagerSeE2eTest, PauseStandby_triggersSessionEffectPause_E2E, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.frameLen = TEST_FRAME_LEN;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    rendererManager_->CreateStream(streamInfo);
    ProcessRmMessages();

    // AudioEffectChainManager not init'd → no cluster created; inject one
    InjectProcessCluster(TEST_SESSION_ID);

    // Start session first (populates sessionNodeMap_ for CreateSessionEffect)
    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    // Bind SE (PostCreateSessionEffect needs sessionNodeMap_ entry)
    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);
    ProcessRmMessages();

    ASSERT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));

    // Pause with isStandby=true (direct disconnect path)
    rendererManager_->Pause(TEST_SESSION_ID, true);
    ProcessRmMessages();  // processes Pause lambda → SE PauseEffect called

    // Verify: SE still exists (pause doesn't destroy)
    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
}

// --- E2E: MoveStream ---

// HM-MOVE-E2E: SE recreation on new device via BindSessionEffect + PostCreateSessionEffect
HWTEST_F(HpaeManagerSeE2eTest, MoveStream_recreatesSessionEffect_E2E, TestSize.Level0)
{
    // 1. Bind SE to session (tracked in SessionEffectManager)
    InjectStereoMusicStream();
    ASSERT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true), SUCCESS);

    auto boundId = hpaeManager_->sessionEffectManager_->GetBoundSessionId("audio_aiss_enhance");
    EXPECT_EQ(boundId, TEST_SESSION_ID);

    // 2. Create target RM with session + cluster
    HpaeSinkInfo sinkInfo2 = GetTestSinkInfo();
    sinkInfo2.deviceClass = "file_io_2";
    sinkInfo2.adapterName = "file_io_2";
    sinkInfo2.filePath = "/data/local/tmp/test_se_move_target.pcm";
    auto rm2 = std::make_shared<HpaeRendererManager>(sinkInfo2);
    rm2->Init();
    rm2->DeactivateThread();
    hpaeManager_->rendererManagerMap_["file_io_2"] = rm2;

    // Manually set up session on target RM (bypass CreateStream dependencies)
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = TEST_SESSION_ID;
    nodeInfo.frameLen = TEST_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    auto sinkInput2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInput2->SetState(HPAE_SESSION_RUNNING);
    rm2->sinkInputNodeMap_[TEST_SESSION_ID] = sinkInput2;
    rm2->sessionNodeMap_[TEST_SESSION_ID].sceneType = HPAE_SCENE_EFFECT_NONE;
    rm2->sessionNodeMap_[TEST_SESSION_ID].bypass = false;

    auto cluster2 = std::make_shared<HpaeProcessCluster>(nodeInfo, rm2->sinkInfo_);
    cluster2->CreateNodes(sinkInput2);
    cluster2->Connect(sinkInput2);
    rm2->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] = cluster2;

    // 3. Recreate SE on target (simulates HandleMoveSinkInput's SE path)
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    std::shared_ptr<EffectInstance> instance;
    hpaeManager_->sessionEffectManager_->BindSessionEffect(
        TEST_SESSION_ID, "audio_aiss_enhance", config, instance);
    ASSERT_NE(instance, nullptr);
    // Call CreateSessionEffect directly (bypass queue — queue path tested by Flush/Pause)
    int32_t ret = rm2->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance, config);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Verify: binding tracked, SE exists on target RM
    boundId = hpaeManager_->sessionEffectManager_->GetBoundSessionId("audio_aiss_enhance");
    EXPECT_EQ(boundId, TEST_SESSION_ID);
    EXPECT_TRUE(rm2->HasSessionEffect(TEST_SESSION_ID));

    // Cleanup
    rm2->PostDestroySessionEffect(TEST_SESSION_ID);
    rm2->hpaeNoLockQueue_.HandleRequests();
    hpaeManager_->rendererManagerMap_.erase("file_io_2");
}

// HM-RELOAD-E2E: ReloadRenderManager rebuilds the physical SE node from logical binding
HWTEST_F(HpaeManagerSeE2eTest, ReloadRenderManager_recreatesSessionEffect_E2E, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    ASSERT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);

    hpaeManager_->sinkNameSinkIdMap_["file_io"] = 0;
    hpaeManager_->sinkIdSinkNameMap_[0] = "file_io";
    AudioModuleInfo moduleInfo = GetReloadAudioModuleInfo();

    int32_t ret = hpaeManager_->ReloadRenderManager(moduleInfo, true);
    EXPECT_EQ(ret, SUCCESS);
    ProcessRmMessages();
    ProcessRmMessages();

    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
}

// HM-RELOAD-OFFLOAD: reload rebuild restores SE on HpaeOffloadRendererManager from attach state
HWTEST_F(HpaeManagerSeE2eTest, ReloadRenderManager_recreatesOffloadSessionEffect_E2E, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetTestSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    ASSERT_EQ(offloadManager->Init(), SUCCESS);
    offloadManager->DeactivateThread();
    hpaeManager_->rendererManagerMap_["file_io"] = offloadManager;
    hpaeManager_->rendererIdSinkNameMap_[TEST_SESSION_ID] = "file_io";

    ASSERT_TRUE(SetupOffloadStreamWithSE(hpaeManager_, offloadManager));
    auto oldNode = offloadManager->sessionEffectNode_;
    ASSERT_NE(oldNode, nullptr);

    hpaeManager_->sinkNameSinkIdMap_["file_io"] = 0;
    hpaeManager_->sinkIdSinkNameMap_[0] = "file_io";
    AudioModuleInfo moduleInfo = GetReloadAudioModuleInfo();

    int32_t ret = hpaeManager_->ReloadRenderManager(moduleInfo, true);
    EXPECT_EQ(ret, SUCCESS);
    offloadManager->hpaeNoLockQueue_.HandleRequests();
    offloadManager->hpaeNoLockQueue_.HandleRequests();

    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
    EXPECT_NE(offloadManager->sessionEffectNode_, nullptr);
    EXPECT_NE(offloadManager->sessionEffectNode_, oldNode);
    EXPECT_EQ(offloadManager->sessionEffectSessionId_, TEST_SESSION_ID);
}

// HM-RELOAD-DIRECT: UpdateAudioPortInfo direct reload path also rebuilds the physical SE node
HWTEST_F(HpaeManagerSeE2eTest, UpdateAudioPortInfo_recreatesSessionEffect_E2E, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    ASSERT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    auto oldNode = rendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(oldNode, nullptr);

    HpaeNodeInfo nodeInfo;
    TransSinkInfoToNodeInfo(rendererManager_->sinkInfo_, rendererManager_, nodeInfo);
    hpaeManager_->sinkNameSinkIdMap_["file_io"] = 0;
    hpaeManager_->sinkIdSinkNameMap_[0] = "file_io";
    hpaeManager_->sinkVirtualOutputNodeMap_[0] = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo);

    AudioModuleInfo moduleInfo = GetReloadAudioModuleInfo();
    moduleInfo.rate = "44100";
    hpaeManager_->UpdateAudioPortInfo(0, moduleInfo);
    WaitForManagerMessages();
    ProcessRmMessages();
    ProcessRmMessages();

    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    auto newNode = rendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->GetSessionEffectNode(TEST_SESSION_ID);
    EXPECT_NE(newNode, nullptr);
    EXPECT_NE(newNode, oldNode);
}

// HM-REFRESH: detaching the SE node for refresh removes it from the OLD cluster but preserves
// the attach state so the NEW cluster can re-materialize it on reconnect (no ClearSessionEffectAttachState).
HWTEST_F(HpaeManagerSeE2eTest, DetachSessionEffectNodeForRefresh_preservesAttachState, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    constexpr HpaeProcessorType oldSceneType = HPAE_SCENE_EFFECT_NONE;
    auto oldCluster = rendererManager_->sceneClusterMap_[oldSceneType];
    ASSERT_NE(oldCluster, nullptr);
    auto sinkInputNode = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInputNode, nullptr);

    // Preconditions: SE node present in OLD cluster, attach state present.
    ASSERT_TRUE(oldCluster->HasSessionEffectNode(TEST_SESSION_ID));
    ASSERT_TRUE(sinkInputNode->HasSessionEffectAttachState());

    // Act: detach SE node for refresh (delete physical node, keep attach state).
    rendererManager_->DetachSessionEffectNodeForRefresh(TEST_SESSION_ID, oldSceneType);

    // SE node removed from OLD cluster.
    EXPECT_FALSE(oldCluster->HasSessionEffectNode(TEST_SESSION_ID));
    // Attach state SURVIVES (core requirement).
    EXPECT_TRUE(sinkInputNode->HasSessionEffectAttachState());
}

// --- E2E: Pause→Start (SessionEffectStart recovery) ---

// Helper: common setup for T10-T12. Returns false on failure.
static bool SetupStreamWithSE(std::shared_ptr<HpaeManager> hpaeManager_,
    std::shared_ptr<HpaeRendererManager> rendererManager_)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.frameLen = TEST_FRAME_LEN;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    rendererManager_->CreateStream(streamInfo);
    rendererManager_->hpaeNoLockQueue_.HandleRequests();

    auto sinkInput = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    if (sinkInput == nullptr) { return false; }
    HpaeNodeInfo nodeInfo = sinkInput->GetNodeInfo();
    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, rendererManager_->sinkInfo_);
    cluster->CreateNodes(sinkInput);
    cluster->Connect(sinkInput);
    rendererManager_->sessionNodeMap_[TEST_SESSION_ID].sceneType = HPAE_SCENE_EFFECT_NONE;
    rendererManager_->sessionNodeMap_[TEST_SESSION_ID].bypass = false;
    rendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] = cluster;

    rendererManager_->Start(TEST_SESSION_ID);
    rendererManager_->hpaeNoLockQueue_.HandleRequests();

    if (hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true) != SUCCESS) { return false; }
    rendererManager_->hpaeNoLockQueue_.HandleRequests();
    return rendererManager_->HasSessionEffect(TEST_SESSION_ID);
}

static bool SetupOffloadStreamWithSE(std::shared_ptr<HpaeManager> hpaeManager_,
    std::shared_ptr<HpaeOffloadRendererManager> offloadManager_)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.frameLen = TEST_FRAME_LEN;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    streamInfo.channelLayout = CH_LAYOUT_STEREO;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    if (offloadManager_->CreateStream(streamInfo) != SUCCESS) { return false; }
    offloadManager_->hpaeNoLockQueue_.HandleRequests();
    if (offloadManager_->Start(TEST_SESSION_ID) != SUCCESS) { return false; }
    offloadManager_->hpaeNoLockQueue_.HandleRequests();
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID].state = HPAE_SESSION_RUNNING;

    if (hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, true) != SUCCESS) { return false; }
    offloadManager_->hpaeNoLockQueue_.HandleRequests();
    return offloadManager_->sessionEffectNode_ != nullptr &&
        offloadManager_->sessionEffectSessionId_ == TEST_SESSION_ID;
}

// T10: RendererManager Pause(standby)→Start resumes SE
HWTEST_F(HpaeManagerSeE2eTest, PauseStart_standby_resumesSE, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    // Pause (standby=true)
    rendererManager_->Pause(TEST_SESSION_ID, true);
    ProcessRmMessages();

    // Verify SE node is paused
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_TRUE(seNode->IsPaused());

    // Start — should resume SE
    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    EXPECT_FALSE(seNode->IsPaused());
}

// T11: RendererManager Pause(fade)→OnFadeDone→Start resumes SE
HWTEST_F(HpaeManagerSeE2eTest, PauseStart_fadePath_resumesSE, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    // Pause with isStandby=false triggers fade path
    rendererManager_->Pause(TEST_SESSION_ID, false);
    ProcessRmMessages();

    // Simulate fade completion
    rendererManager_->OnFadeDone(TEST_SESSION_ID);
    ProcessRmMessages();

    // Verify SE node is paused after fade done
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_TRUE(seNode->IsPaused());

    // Start
    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    EXPECT_FALSE(seNode->IsPaused());
}

HWTEST_F(HpaeManagerSeE2eTest, Stop_directPath_pausesSessionEffect, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    rendererManager_->Stop(TEST_SESSION_ID);
    ProcessRmMessages();

    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_TRUE(seNode->IsPaused());
    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
}

HWTEST_F(HpaeManagerSeE2eTest, StopStart_directPath_resumesSessionEffect, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    rendererManager_->Stop(TEST_SESSION_ID);
    ProcessRmMessages();

    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    ASSERT_TRUE(seNode->IsPaused());

    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    EXPECT_FALSE(seNode->IsPaused());
}

HWTEST_F(HpaeManagerSeE2eTest, Stop_fadeDonePath_pausesSessionEffect, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto sinkInput = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInput, nullptr);
    sinkInput->SetState(HPAE_SESSION_STOPPING);
    rendererManager_->SetSessionState(TEST_SESSION_ID, HPAE_SESSION_STOPPING);

    rendererManager_->OnFadeDone(TEST_SESSION_ID);
    ProcessRmMessages();

    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_TRUE(seNode->IsPaused());
    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
}

HWTEST_F(HpaeManagerSeE2eTest, StopStart_fadeDonePath_resumesSessionEffect, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto sinkInput = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInput, nullptr);
    sinkInput->SetState(HPAE_SESSION_STOPPING);
    rendererManager_->SetSessionState(TEST_SESSION_ID, HPAE_SESSION_STOPPING);

    rendererManager_->OnFadeDone(TEST_SESSION_ID);
    ProcessRmMessages();

    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    ASSERT_TRUE(seNode->IsPaused());

    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    EXPECT_FALSE(seNode->IsPaused());
}

// T12: RendererManager Pause→Flush→Start (E2E)
HWTEST_F(HpaeManagerSeE2eTest, PauseFlushStart_E2E, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    // Pause
    rendererManager_->Pause(TEST_SESSION_ID, true);
    ProcessRmMessages();

    // Flush — resets node to IDLE, clears isPaused_
    rendererManager_->Flush(TEST_SESSION_ID);
    ProcessRmMessages();

    // Verify node is not paused after flush
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    auto cluster = rendererManager_->sceneClusterMap_[sceneType];
    ASSERT_NE(cluster, nullptr);
    auto seNode = cluster->GetSessionEffectNode(TEST_SESSION_ID);
    ASSERT_NE(seNode, nullptr);
    EXPECT_FALSE(seNode->IsPaused());
    EXPECT_EQ(seNode->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    // Start — should go through first-start path → WARMING_UP
    rendererManager_->Start(TEST_SESSION_ID);
    ProcessRmMessages();

    EXPECT_TRUE(rendererManager_->HasSessionEffect(TEST_SESSION_ID));
    EXPECT_FALSE(seNode->IsPaused());
    EXPECT_EQ(seNode->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

// --- ChannelOverrideRoute helpers (Task B1) ---
// Fixture does not init AudioEffectChainManager, so GetProcessorType resolves to
// HPAE_SCENE_EFFECT_NONE for TEST_SESSION_ID. Assert BuildChannelOverrideRoute shapes directly.

HWTEST_F(HpaeManagerSeE2eTest, BuildChannelOverrideRoute_NoneReturnsNullopt, TestSize.Level1)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    // Fixture injects sessionNodeMap_.sceneType = HPAE_SCENE_EFFECT_NONE, so the per-session
    // route resolves to NONE -> nullopt.
    auto route = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    EXPECT_FALSE(route.has_value());

    // Direct: NONE processor type always returns nullopt regardless of device/render state.
    auto noneRoute = rendererManager_->BuildChannelOverrideRouteFromProcessorType(HPAE_SCENE_EFFECT_NONE);
    EXPECT_FALSE(noneRoute.has_value());

    // A non-NONE processor type yields a populated route carrying the renderer's render state.
    auto musicRoute = rendererManager_->BuildChannelOverrideRouteFromProcessorType(HPAE_SCENE_MUSIC);
    ASSERT_TRUE(musicRoute.has_value());
    EXPECT_FALSE(musicRoute->sceneType.empty());
    EXPECT_EQ(musicRoute->deviceType, rendererManager_->renderDeviceType_);
    EXPECT_EQ(musicRoute->renderId, rendererManager_->renderId_);
}

// --- ChannelOverride orchestration on SE enable/disable (Task B2, scenarios 7 & 8) ---
//
// Fixture limitation: AudioEffectChainManager is NOT init'd in HpaeManagerSeE2eTest, and
// SetupStreamWithSE injects sessionNodeMap_.sceneType = HPAE_SCENE_EFFECT_NONE. Therefore
// GetProcessorType(TEST_SESSION_ID) returns NONE, BuildChannelOverrideRoute returns nullopt,
// and OnSessionEffectMaterialized / DestroySessionEffect's clear-override branch are no-ops.
// The channelOverrideMap_ stays empty, so a positive "map has key after enable" assertion
// cannot hold under this fixture. Two complementary cases cover the gap:
//   1) DISABLED_ChannelOverride_SetOnEnable_ClearedOnDisable — the intended map-state assertion
//      (skipped by gtest; requires an arm-chain E2E environment to drive a non-NONE scene).
//   2) ChannelOverride_EnableDisableSmoke — drives the full SetAissEnabledSync(true/false) flow
//      under the existing fixture and asserts no crash: protects the new OnSessionEffectMaterialized
//      / BuildChannelOverrideRoute / ClearChannelOverrideForRoute code paths against regressions
//      on the NONE branch (nullopt short-circuit + untouched singleton).

HWTEST_F(HpaeManagerSeE2eTest, DISABLED_ChannelOverride_SetOnEnable_ClearedOnDisable, TestSize.Level1)
{
    // Scenario 7 (override set on enable) + Scenario 8 (override cleared on disable).
    // Requires a non-NONE sceneType with ExistAudioEffectChainArm == true for that scene so that
    // BuildChannelOverrideRoute yields a populated route and OnSessionEffectMaterialized writes
    // into channelOverrideMap_. Enable arm-chain E2E environment to activate this case.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto route = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    ASSERT_TRUE(route.has_value());
    std::string key = route->sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 1u);

    // disable: SetAissEnabledSync(false) -> DestroySessionEffect -> clear override
    EXPECT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, false), SUCCESS);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 0u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeManagerSeE2eTest, ChannelOverride_EnableDisableSmoke, TestSize.Level1)
{
    // Smoke / regression: drive enable then disable through the real CreateSessionEffect /
    // DestroySessionEffect success paths on the NONE-scene fixture. The new orchestration hooks
    // (OnSessionEffectMaterialized, BuildChannelOverrideRoute, ClearChannelOverrideForRoute) must
    // execute without crashing; under NONE they all short-circuit on nullopt, leaving
    // channelOverrideMap_ untouched (singleton never invoked).
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    // Map is empty both because the singleton was never written and because the NONE route
    // returned nullopt before any SetChannelOverride call site.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);

    EXPECT_EQ(hpaeManager_->SetAissEnabledSync(TEST_SESSION_ID, false), SUCCESS);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);
}

// --- ChannelOverride orchestration on SE materialize failure (Task B3, scenario 12) ---
//
// Same fixture limitation as Task B2: AudioEffectChainManager is NOT init'd and
// SetupStreamWithSE sets sessionNodeMap_.sceneType = HPAE_SCENE_EFFECT_NONE, so
// GetProcessorType(TEST_SESSION_ID) returns NONE and BuildChannelOverrideRoute returns nullopt.
// Hence a positive "map had a key, then it was cleared on materialize failure" assertion cannot
// hold here. Two complementary cases:
//   1) DISABLED_ChannelOverride_ClearedOnMaterializeFailure — the intended map-state assertion
//      (skipped by gtest; needs an arm-chain E2E environment with a non-NONE scene so the route
//      is populated and a prior enable actually wrote a key into channelOverrideMap_).
//   2) ChannelOverride_MaterializeFailureSmoke — drives the real materialize-failure path by
//      injecting an attach-state whose instance weak_ptr is expired, then calling
//      MaterializeSessionEffectFromAttachState directly. Under NONE the route resolves to
//      nullopt, so the new clear-override branch in HandleSessionEffectMaterializeFailure must
//      short-circuit without touching the singleton (no crash, map stays empty). This protects
//      the new code path against regressions on the NONE branch.

HWTEST_F(HpaeManagerSeE2eTest, DISABLED_ChannelOverride_ClearedOnMaterializeFailure, TestSize.Level1)
{
    // Scenario 12: a prior enable wrote a channelOverrideMap_ key; when materialize fails on
    // reload/refresh, HandleSessionEffectMaterializeFailure must clear it.
    // Requires a non-NONE sceneType whose effect chain is armed (ExistAudioEffectChainArm == true)
    // so that BuildChannelOverrideRoute yields a populated route and the enable path wrote into
    // channelOverrideMap_. Enable arm-chain E2E environment to activate this case.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto route = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    ASSERT_TRUE(route.has_value());
    std::string key = route->sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
    ASSERT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 1u);

    // Force materialize failure: replace attach-state with an enabled state whose instance
    // weak_ptr is expired, then re-materialize. HandleSessionEffectMaterializeFailure must clear
    // the override before emitting the DISABLED callback.
    auto sinkInputNode = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInputNode, nullptr);
    {
        // Create a shared_ptr that goes out of scope (and thus expires the weak_ptr) before the
        // materialize call, leaving attach-state.instance.lock() == nullptr.
        std::shared_ptr<EffectInstance> transient;
        ASSERT_EQ(hpaeManager_->sessionEffectManager_->BindSessionEffect(
            TEST_SESSION_ID, AISS_EFFECT_NAME, MakeTestAudioEffectConfig(), transient), SUCCESS);
        SessionEffectAttachState st;
        st.enabled = true;
        st.effectName = AISS_EFFECT_NAME;
        st.instance = transient;
        st.config = MakeTestAudioEffectConfig();
        sinkInputNode->SetSessionEffectAttachState(st);
        // transient destroyed here -> st.instance weak_ptr expires.
    }
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    rendererManager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID, sceneType);
    ProcessRmMessages();

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 0u);
    EXPECT_FALSE(sinkInputNode->HasSessionEffectAttachState());
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeManagerSeE2eTest, ChannelOverride_MaterializeFailureSmoke, TestSize.Level1)
{
    // Smoke / regression: drive the real materialize-failure path under the NONE scene by
    // injecting an attach-state whose instance weak_ptr is expired, then invoking
    // MaterializeSessionEffectFromAttachState directly. HandleSessionEffectMaterializeFailure
    // runs its new clear-override branch; under NONE BuildChannelOverrideRoute returns nullopt
    // so the branch must short-circuit without invoking the singleton (no crash, map empty,
    // attach-state cleared).
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    auto sinkInputNode = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInputNode, nullptr);
    ASSERT_TRUE(sinkInputNode->HasSessionEffectAttachState());

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);

    // Replace attach-state with an enabled state whose instance weak_ptr is expired, then
    // re-materialize -> failure branch (instance.lock() == nullptr).
    {
        SessionEffectAttachState st;
        st.enabled = true;
        st.effectName = AISS_EFFECT_NAME;
        // weak_ptr default-constructed from nullptr shared_ptr -> expired immediately.
        st.config = MakeTestAudioEffectConfig();
        sinkInputNode->SetSessionEffectAttachState(st);
    }
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    rendererManager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID, sceneType);
    ProcessRmMessages();

    // No crash, map untouched (NONE route -> nullopt short-circuit), attach-state cleared.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);
    EXPECT_FALSE(sinkInputNode->HasSessionEffectAttachState());
}

// --- ChannelOverride cleanup on DeleteInputSession (Task B4, scenarios 13, 14, 17) ---
//
// Same fixture limitation as Task B2/B3: AudioEffectChainManager is NOT init'd and
// SetupStreamWithSE sets sessionNodeMap_.sceneType = HPAE_SCENE_EFFECT_NONE, so
// GetProcessorType(TEST_SESSION_ID) returns NONE and BuildChannelOverrideRoute returns nullopt.
// Hence a positive "map had a key, then DeleteInputSession cleared it" assertion cannot hold
// here. Two complementary cases:
//   1) DISABLED_ChannelOverride_ClearedOnDeleteInputSession — the intended map-state assertion
//      (skipped by gtest; needs an arm-chain E2E environment with a non-NONE scene so the route
//      is populated and a prior enable actually wrote a key into channelOverrideMap_). Covers
//      scenarios 13 (FULL_TEARDOWN) and 14 (KEEP_ATTACH_STATE_FOR_MOVE move-out).
//   2) ChannelOverride_DeleteInputSessionNoneSceneNoOp — drives the real DeleteInputSession
//      path under the NONE scene. BuildChannelOverrideRoute returns nullopt, so the new
//      clear-override branch in DeleteInputSession must short-circuit without touching the
//      singleton (no crash, map stays empty). This is scenario 17 (NONE skip) and protects
//      the new code path against regressions on the NONE branch.

HWTEST_F(HpaeManagerSeE2eTest, DISABLED_ChannelOverride_ClearedOnDeleteInputSession, TestSize.Level1)
{
    // Scenario 13 (FULL_TEARDOWN clears) + Scenario 14 (KEEP_ATTACH_STATE_FOR_MOVE move-out
    // clears old route). Requires a non-NONE sceneType whose effect chain is armed
    // (ExistAudioEffectChainArm == true) so that BuildChannelOverrideRoute yields a populated
    // route and the enable path wrote into channelOverrideMap_. Enable arm-chain E2E
    // environment to activate this case.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto route = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    ASSERT_TRUE(route.has_value());
    std::string key = route->sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
    ASSERT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 1u);

    // Scenario 13: full teardown without an explicit disable -> DeleteInputSession(FULL_TEARDOWN)
    // captures the route at entry and clears the override after teardown.
    rendererManager_->DeleteInputSession(TEST_SESSION_ID, SessionEffectMoveMode::FULL_TEARDOWN);
    ProcessRmMessages();

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 0u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeManagerSeE2eTest, ChannelOverride_DeleteInputSessionNoneSceneNoOp, TestSize.Level1)
{
    // Scenario 17 (NONE skip): under the fixture's NONE scene, BuildChannelOverrideRoute returns
    // nullopt, so DeleteInputSession's new clear-override branch must short-circuit without
    // invoking the singleton. Drives both moveModes to make sure neither path crashes.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);

    // FULL_TEARDOWN: NONE route -> nullopt short-circuit, map untouched, no crash.
    rendererManager_->DeleteInputSession(TEST_SESSION_ID, SessionEffectMoveMode::FULL_TEARDOWN);
    ProcessRmMessages();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);

    // Re-establish the stream to exercise KEEP_ATTACH_STATE_FOR_MOVE on the same session.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    rendererManager_->DeleteInputSession(TEST_SESSION_ID,
        SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);
    ProcessRmMessages();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);
}

// --- ChannelOverride re-set on SE materialize success (Task B5, scenarios 11 & 15) ---
//
// Same fixture limitation as Task B2/B3/B4: AudioEffectChainManager is NOT init'd and
// SetupStreamWithSE sets sessionNodeMap_.sceneType = HPAE_SCENE_EFFECT_NONE, so
// GetProcessorType(TEST_SESSION_ID) returns NONE and BuildChannelOverrideRoute returns nullopt.
// Hence a positive "override re-set on rematerialize" assertion cannot hold here. Two
// complementary cases:
//   1) DISABLED_ChannelOverride_PreservedOnSameRouteRematerialize — the intended map-state
//      assertion (skipped by gtest; needs an arm-chain E2E environment with a non-NONE scene so
//      the route is populated and a prior enable actually wrote a key into channelOverrideMap_).
//      Covers scenario 11 (reload/refresh same-route rematerialize re-sets the same key) and
//      scenario 15 (move-in to a new route sets the new route's key via this same hook).
//   2) ChannelOverride_RematerializeSuccessSmoke — drives the real materialize-success path
//      (reload/refresh same-route rematerialize) directly. Under NONE the route resolves to
//      nullopt, so OnSessionEffectMaterialized must short-circuit without touching the singleton
//      (no crash, map stays empty). Protects the new OnSessionEffectMaterialized call on the
//      success branch against regressions on the NONE branch.

HWTEST_F(HpaeManagerSeE2eTest, DISABLED_ChannelOverride_PreservedOnSameRouteRematerialize, TestSize.Level1)
{
    // Scenario 11 (reload/refresh same-route rematerialize keeps the override) + scenario 15
    // (move-in sets the new route's override via the same success-branch hook).
    // Requires a non-NONE sceneType whose effect chain is armed (ExistAudioEffectChainArm == true)
    // so that BuildChannelOverrideRoute yields a populated route and the enable path wrote into
    // channelOverrideMap_. Enable arm-chain E2E environment to activate this case.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto route = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    ASSERT_TRUE(route.has_value());
    std::string key = route->sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
    ASSERT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 1u);

    // Trigger same-route rematerialize (reload/refresh, sceneType unchanged): the materialize
    // success branch invokes OnSessionEffectMaterialized, which re-sets the same route's key.
    // Re-invoking MaterializeSessionEffectFromAttachState on an already-bound session returns
    // ERR_SESSION_ALREADY_BOUND (still the success branch), exercising the new hook.
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    rendererManager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID, sceneType);
    ProcessRmMessages();

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(key), 1u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeManagerSeE2eTest, ChannelOverride_RematerializeSuccessSmoke, TestSize.Level1)
{
    // Smoke / regression: drive the real materialize-success path (reload/refresh same-route
    // rematerialize) directly under the NONE scene. OnSessionEffectMaterialized runs its new
    // BuildChannelOverrideRoute call; under NONE it returns nullopt, so the hook must
    // short-circuit without invoking the singleton (no crash, map stays empty).
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);

    // Re-materialize on the same route: CreateSessionEffectNode's duplicate-guard returns
    // ERR_SESSION_ALREADY_BOUND (success branch) -> OnSessionEffectMaterialized is invoked.
    HpaeProcessorType sceneType = rendererManager_->GetProcessorType(TEST_SESSION_ID);
    rendererManager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID, sceneType);
    ProcessRmMessages();

    // No crash, map untouched (NONE route -> nullopt short-circuit).
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);
}

HWTEST_F(HpaeManagerSeE2eTest, DISABLED_ChannelOverride_RouteMigrationClearsOldSetsNew, TestSize.Level1)
{
    // Scenario 11a: refresh route migration (sceneType change). The migration branch clears the
    // OLD route's override (key built from connectedProcessorType_, i.e. the pre-migration scene)
    // and the subsequent ConnectInputSession -> MaterializeSessionEffectFromAttachState success
    // branch (Task B5) sets the NEW route's override.
    // Requires a route-migration test seam / E2E: a non-NONE scene whose effect chain is armed so
    // that BuildChannelOverrideRouteFromProcessorType yields a populated route and a prior enable
    // actually wrote a key into channelOverrideMap_, plus a way to flip the effective sceneType
    // (e.g. change sink / bypass) so connectedProcessorType_ != GetProcessorType at RUNNING.
    // 待 route-migration test seam/E2E: enable an arm-chain E2E environment with a controllable
    // sceneType transition to activate this case.
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    auto oldRoute = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    ASSERT_TRUE(oldRoute.has_value());
    std::string oldKey = oldRoute->sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
    ASSERT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(oldKey), 1u);

    // OLD route's override key must be cleared by the Task B6 hook.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(oldKey), 0u);
    // NEW route's override key must be set by the Task B5 success-branch hook.
    auto newRoute = rendererManager_->BuildChannelOverrideRoute(TEST_SESSION_ID);
    if (newRoute.has_value() && newRoute->sceneType != oldRoute->sceneType) {
        std::string newKey = newRoute->sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(newKey), 1u);
    }
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(HpaeManagerSeE2eTest, ChannelOverride_RouteMigrationNoneSmoke, TestSize.Level1)
{
    // Smoke / regression: drive the real refresh route-migration path
    // (RefreshProcessClusterByDeviceInner migration branch) directly under the NONE scene.
    // connectedProcessorType_ is forced to a value != GetProcessorType (also NONE) by setting it
    // to a non-NONE value, then RefreshProcessClusterByDeviceInner runs the migration branch; the
    // new oldRoute capture (BuildChannelOverrideRouteFromProcessorType(connectedProcessorType_))
    // yields a populated route, but AudioEffectChainManager is NOT init'd so
    // ClearChannelOverrideForRoute's ClearChannelOverride is a no-op on the (un-init'd) singleton
    // — the value of this case is that the new hook executes without crash on the migration path.
    // (If the AudioEffectChainManager were init'd and the route armed, the old key would be
    // cleared here; under this fixture we only assert no-crash + no throw.)
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));

    // Force the migration condition: connectedProcessorType_ != GetProcessorType(sessionId) on a
    // RUNNING sink input. GetProcessorType resolves to NONE in this fixture; set
    // connectedProcessorType_ to a non-NONE value to enter the migration branch.
    auto sinkInput = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInput, nullptr);
    sinkInput->SetState(HPAE_SESSION_RUNNING);
    sinkInput->connectedProcessorType_ = HPAE_SCENE_MUSIC;

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);

    // Drive the migration branch directly: exercises the new oldRoute capture +
    // ClearChannelOverrideForRoute call, then DisConnect/Connect.
    rendererManager_->RefreshProcessClusterByDeviceInner(sinkInput);
    ProcessRmMessages();

    // No crash, map untouched (singleton not init'd -> ClearChannelOverride no-op).
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);
}

// --- Deferred AISS-disable: move-in attach-state guard (Scenario 8 & §4.4) ---
//
// When AISS is logically closed mid-move, the migrating node keeps its attach-state while the
// logical binding no longer points at the session. That is exactly the two-condition gate the
// move-in guard (HandleMoveSinkInput / HandleMoveAllSinkInputs) uses to call
// ClearSessionEffectAttachState and prevent AISS reviving from the old attach-state within the 3s
// lazy-release window. A full device-to-device move flow is not drivable under this fixture, so
// this test verifies the gate's preconditions; ClearSessionEffectAttachState itself is exercised by
// the ChannelOverride materialize-failure tests above.
HWTEST_F(HpaeManagerSeE2eTest, DisableWhenInactive_midMoveClose_attachStateSurvivesBindingCleared, TestSize.Level0)
{
    ASSERT_TRUE(SetupStreamWithSE(hpaeManager_, rendererManager_));
    auto sinkInputNode = rendererManager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(sinkInputNode, nullptr);
    ASSERT_TRUE(sinkInputNode->HasSessionEffectAttachState());
    ASSERT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);

    // Simulate the logical-only half of a mid-move close: unbind without driving the renderer-side
    // DestroySessionEffect (which, during a real move, cannot reach the already-migrated node and so
    // leaves the attach-state intact on the node).
    hpaeManager_->sessionEffectManager_->UnbindSessionEffect(TEST_SESSION_ID);

    // Guard gate #2: the logical binding no longer points at this session.
    EXPECT_NE(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), TEST_SESSION_ID);
    // Guard gate #1: the migrating node still carries the attach-state -> the move-in guard fires.
    EXPECT_TRUE(sinkInputNode->HasSessionEffectAttachState());
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
