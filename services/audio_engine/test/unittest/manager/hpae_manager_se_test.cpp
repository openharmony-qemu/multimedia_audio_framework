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
#include "hpae_process_cluster.h"
#include "hpae_session_effect_node.h"
#include "hpae_sink_input_node.h"
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
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
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
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED));
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
    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->sessionEffectManager_->GetBoundSessionId(AISS_EFFECT_NAME), 0u);
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
        hpaeManager_.reset();
        rendererManager_.reset();
    }

    // Synchronously process one batch of queued messages
    void ProcessRmMessages()
    {
        rendererManager_->hpaeNoLockQueue_.HandleRequests();
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

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
