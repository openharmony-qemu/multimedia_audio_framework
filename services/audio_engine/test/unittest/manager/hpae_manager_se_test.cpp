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
static constexpr uint32_t UNKNOWN_SESSION_ID = 999;
static constexpr int32_t TEST_UID = 1000;
static constexpr int32_t OTHER_UID = 2000;

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

    void WaitForMsgProcessing()
    {
        int waitCount = 0;
        while (hpaeManager_->IsMsgProcessing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            waitCount++;
            if (waitCount >= 5) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
};

// --- SetAissEnabled ---

// API-1: SetAissEnabled(true) succeeds when session exists
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_success, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// API-2: SetAissEnabled(true) returns ERR_SESSION_NOT_FOUND when session doesn't exist
// 前置条件: Init() 已完成 → sessionEffectManager_ 不为 null
// 前置条件: UNKNOWN_SESSION_ID 不在 rendererIdSinkNameMap_ 中
// Design ref: se-state-management.md §3.1, se-test-plan.md §6 API-2
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_sessionNotFound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabled(UNKNOWN_SESSION_ID, true);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// API-3: SetAissEnabled(true) is idempotent when already bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_alreadyBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);

    ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// API-4: SetAissEnabled(false) succeeds when session is bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_false_success, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, false);
    EXPECT_EQ(ret, SUCCESS);
}

// API-5: SetAissEnabled(false) returns ERR_EFFECT_NOT_BOUND when not bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_false_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, false);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// API-6: SetAissEnabled(true) routes to HpaeRendererManager for Primary path
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_primaryPath, TestSize.Level0)
{
    // Default fixture already has HpaeRendererManager → Primary path
    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// API-7: SetAissEnabled(true) routes to HpaeOffloadRendererManager for Offload path
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_offloadPath, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetTestSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    hpaeManager_->rendererManagerMap_["file_io"] = offloadManager;

    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// --- SetVocalRatio ---

// API-8: SetVocalRatio succeeds with valid ratio and bound session
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_success, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetVocalRatio(0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

// API-9: SetVocalRatio returns ERR_INVALID_PARAMETER for out-of-range ratio
// Design ref: se-state-management.md §3.1, se-test-plan.md §6 API-9
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_invalidRange, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetVocalRatio(-0.1f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));

    ret = hpaeManager_->SetVocalRatio(1.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));
}

// API-10: SetVocalRatio returns ERR_EFFECT_NOT_BOUND when not bound
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetVocalRatio(0.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// API-11: SetVocalRatio succeeds via UID binding path
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_uidPath_success, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetVocalRatio(0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

// --- SetAissEnabledByUid ---

// UID-1: SetAissEnabledByUid(true) succeeds and stays pending (no running stream)
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_true_pending, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabledByUid(TEST_UID, true);
    EXPECT_EQ(ret, SUCCESS);
}

// UID-2: SetAissEnabledByUid(true) returns error on duplicate uid
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_true_duplicateUid, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabledByUid(TEST_UID, true);
    EXPECT_NE(ret, SUCCESS);
}

// UID-3: SetAissEnabledByUid(false) returns error when no binding exists
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_false_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabledByUid(TEST_UID, false);
    EXPECT_NE(ret, SUCCESS);
}

// UID-4: SetAissEnabledByUid(false) succeeds after setting binding
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_false_success, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

    int32_t ret = hpaeManager_->SetAissEnabledByUid(TEST_UID, false);
    EXPECT_EQ(ret, SUCCESS);
}

// UID-5: SetAissEnabledByUid(true) binds to existing running music stream
HWTEST_F(HpaeManagerSeTest, SetAissEnabledByUid_true_bindRunningStream, TestSize.Level0)
{
    // Inject a running music stream for TEST_UID
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    int32_t ret = hpaeManager_->SetAissEnabledByUid(TEST_UID, true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}

// UID-6: Start auto-binds session effect when uid matches pending binding
HWTEST_F(HpaeManagerSeTest, Start_autoBindOnUidMatch, TestSize.Level0)
{
    // Set pending binding
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

    // Create a stream with matching uid and music type
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    // Start the stream
    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();

    // Verify: boundSessionId_ should now be TEST_SESSION_ID
    EXPECT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);
}
HWTEST_F(HpaeManagerSeTest, Start_noBindOnUidMismatch, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

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
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_VOICE_CALL;
    streamInfo.uid = TEST_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_NEW};

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);

    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
}

// UID-9: DestroyStream resets boundSessionId to pending, keeps uid binding
HWTEST_F(HpaeManagerSeTest, DestroyStream_resetsToPending, TestSize.Level0)
{
    ASSERT_EQ(hpaeManager_->SetAissEnabledByUid(TEST_UID, true), SUCCESS);

    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = TEST_UID;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeManager_->rendererIdStreamInfoMap_[TEST_SESSION_ID] = {streamInfo, HPAE_SESSION_RUNNING};

    hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();
    ASSERT_EQ(hpaeManager_->boundSessionId_, TEST_SESSION_ID);

    hpaeManager_->DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
    WaitForMsgProcessing();

    EXPECT_EQ(hpaeManager_->boundSessionId_, 0u);
    EXPECT_EQ(hpaeManager_->boundUid_, TEST_UID);
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
        hpaeManager_->SetAissEnabled(TEST_SESSION_ID, false);
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
    ASSERT_EQ(hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true), SUCCESS);
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
    ASSERT_EQ(hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true), SUCCESS);
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
    ASSERT_EQ(hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true), SUCCESS);

    auto boundId = hpaeManager_->sessionEffectManager_->GetBoundSessionId("ai_audio_enhance");
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
        TEST_SESSION_ID, "ai_audio_enhance", config, instance);
    ASSERT_NE(instance, nullptr);
    // Call CreateSessionEffect directly (bypass queue — queue path tested by Flush/Pause)
    int32_t ret = rm2->CreateSessionEffect(TEST_SESSION_ID, "ai_audio_enhance", instance, config);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Verify: binding tracked, SE exists on target RM
    boundId = hpaeManager_->sessionEffectManager_->GetBoundSessionId("ai_audio_enhance");
    EXPECT_EQ(boundId, TEST_SESSION_ID);
    EXPECT_TRUE(rm2->HasSessionEffect(TEST_SESSION_ID));

    // Cleanup
    rm2->PostDestroySessionEffect(TEST_SESSION_ID);
    rm2->hpaeNoLockQueue_.HandleRequests();
    hpaeManager_->rendererManagerMap_.erase("file_io_2");
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
