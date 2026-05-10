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

// API-3: SetAissEnabled(true) returns ERR_SESSION_ALREADY_BOUND when already bound
HWTEST_F(HpaeManagerSeTest, SetAissEnabled_true_alreadyBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);

    ret = hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));
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

    int32_t ret = hpaeManager_->SetVocalRatio(TEST_SESSION_ID, 0.5f);
    EXPECT_EQ(ret, SUCCESS);
}

// API-9: SetVocalRatio returns ERR_INVALID_PARAMETER for out-of-range ratio
// Design ref: se-state-management.md §3.1, se-test-plan.md §6 API-9
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_invalidRange, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetVocalRatio(1, -0.1f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));

    ret = hpaeManager_->SetVocalRatio(1, 1.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));
}

// API-10: SetVocalRatio returns ERR_EFFECT_NOT_BOUND when not bound
HWTEST_F(HpaeManagerSeTest, SetVocalRatio_notBound, TestSize.Level0)
{
    int32_t ret = hpaeManager_->SetVocalRatio(TEST_SESSION_ID, 0.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
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

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
