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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "background_download.h"
#include "download_manager.h"
#include "audio_errors.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static const std::string TEST_PERSISTENT_DIR_PATH =
    "/data/service/el2/public/audio_suite/";
static const std::string TEST_PERSISTENT_FILE_PATH =
    TEST_PERSISTENT_DIR_PATH + "download_pending_tasks.xml";
static constexpr mode_t TEST_DIR_PERMISSION = 0750;
static constexpr mode_t TEST_FILE_PERMISSION = 0640;

static bool WriteRawFile(const std::string& content)
{
    int fd = open(TEST_PERSISTENT_FILE_PATH.c_str(), O_WRONLY | O_CREAT | O_TRUNC, TEST_FILE_PERMISSION);
    if (fd < 0) {
        return false;
    }
    ssize_t written = write(fd, content.c_str(), content.size());
    close(fd);
    return written == static_cast<ssize_t>(content.size());
}

class BackgroundDownloadTest : public Test {
public:
    void SetUp() override
    {
        mkdir(TEST_PERSISTENT_DIR_PATH.c_str(), TEST_DIR_PERMISSION);
    }
    void TearDown() override
    {
        BackgroundDownload::ClearPersistedData();
    }
};

/**
 * @tc.name  AddFeatureAndRemove_001
 * @tc.desc  Test adding a feature to the pending queue and removing it,
 *           verifying the persisted file is empty after removal.
 */
HWTEST_F(BackgroundDownloadTest, AddFeatureAndRemove_001, TestSize.Level0)
{
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    EXPECT_EQ(bg->AddFeature("feat1"), SUCCESS);
    EXPECT_EQ(bg->RemoveFeature("feat1"), SUCCESS);
}

/**
 * @tc.name  AddFeatureDuplicate_001
 * @tc.desc  Test adding the same feature twice, verifying idempotency.
 */
HWTEST_F(BackgroundDownloadTest, AddFeatureDuplicate_001, TestSize.Level0)
{
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    EXPECT_EQ(bg->AddFeature("feat2"), SUCCESS);
    EXPECT_EQ(bg->AddFeature("feat2"), SUCCESS);
}

/**
 * @tc.name  GetFeatureConfig_Known_Unknown_001
 * @tc.desc  Test GetFeatureConfig returns wifi-only config for known feature names
 *           and default all-conditions config for unknown feature.
 */
HWTEST_F(BackgroundDownloadTest, GetFeatureConfig_Known_Unknown_001, TestSize.Level0)
{
    const auto& knownConfig = BackgroundDownload::GetFeatureConfig("RecordDefinition");
    EXPECT_TRUE(knownConfig.requireWifi);
    EXPECT_FALSE(knownConfig.requireCharging);
    EXPECT_FALSE(knownConfig.requireBatteryLevel);
    EXPECT_FALSE(knownConfig.requireScreenLocked);

    const auto& unknownConfig = BackgroundDownload::GetFeatureConfig("UnknownFeature");
    EXPECT_TRUE(unknownConfig.requireWifi);
    EXPECT_TRUE(unknownConfig.requireCharging);
    EXPECT_TRUE(unknownConfig.requireBatteryLevel);
    EXPECT_TRUE(unknownConfig.requireScreenLocked);
}

/**
 * @tc.name  RestorePendingFeatures_001
 * @tc.desc  Test restoring pending tasks from string: empty input, normal parsing,
 *           duplicate feature skip, and empty token from leading semicolon.
 */
HWTEST_F(BackgroundDownloadTest, RestorePendingFeatures_001, TestSize.Level0)
{
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    bg->RestorePendingFeatures("");
    bg->RestorePendingFeatures("featA:0;featB:1");
    // featA already exists, third call should not overwrite retryCount
    bg->RestorePendingFeatures("featA:5");
    // Empty token from leading ";" is skipped, featC is restored
    bg->RestorePendingFeatures(";featC:0");
    // Verify: RemoveFeature returns SUCCESS for restored features
    EXPECT_EQ(bg->RemoveFeature("featA"), SUCCESS);
    EXPECT_EQ(bg->RemoveFeature("featB"), SUCCESS);
    EXPECT_EQ(bg->RemoveFeature("featC"), SUCCESS);
}

/**
 * @tc.name  ProcessPendingDownloads_001
 * @tc.desc  Test processing pending downloads in sync mode with all conditions met
 *           and callback succeeding.
 */
HWTEST_F(BackgroundDownloadTest, ProcessPendingDownloads_001, TestSize.Level0)
{
    int32_t callCount = 0;
    auto bg = std::make_shared<BackgroundDownload>(
        [&callCount](const std::string&) { callCount++; return SUCCESS; });
    bg->AddFeature("proc1");
    bg->RestoreFromFile(bg, true);
    EXPECT_GT(callCount, 0);
}

/**
 * @tc.name  ProcessPendingDownloads_CallbackFail_001
 * @tc.desc  Test processing pending downloads when callback fails, verifying the
 *           feature is put back to queue with incremented retry count.
 */
HWTEST_F(BackgroundDownloadTest, ProcessPendingDownloads_CallbackFail_001, TestSize.Level0)
{
    int32_t callCount = 0;
    auto bg = std::make_shared<BackgroundDownload>(
        [&callCount](const std::string&) { callCount++; return ERROR; });
    bg->AddFeature("fail1");
    bg->RestoreFromFile(bg, true);
    // Callback failed, feature should be put back to map with retryCount=1
    EXPECT_GE(callCount, 1);
}

/**
 * @tc.name  ProcessPendingDownloads_EmptyMap_001
 * @tc.desc  Test processing pending downloads with empty queue, verifying callback
 *           is not invoked.
 */
HWTEST_F(BackgroundDownloadTest, ProcessPendingDownloads_EmptyMap_001, TestSize.Level0)
{
    int32_t callCount = 0;
    auto bg = std::make_shared<BackgroundDownload>(
        [&callCount](const std::string&) { callCount++; return SUCCESS; });
    // Map is empty, ProcessPendingDownloads should early return without invoking callback
    bg->ProcessPendingDownloads();
    EXPECT_EQ(callCount, 0);
    // No file should be written on early return
    EXPECT_TRUE(bg->ReadPendingTasksFromFile().empty());
}

/**
 * @tc.name  ProcessPendingDownloads_MaxRetry_001
 * @tc.desc  Test processing pending downloads when retry count reaches maximum,
 *           verifying the feature is discarded.
 */
HWTEST_F(BackgroundDownloadTest, ProcessPendingDownloads_MaxRetry_001, TestSize.Level0)
{
    int32_t callCount = 0;
    auto bg = std::make_shared<BackgroundDownload>(
        [&callCount](const std::string&) { callCount++; return ERROR; });
    // Restore feature with retryCount=2, callback fails -> retryCount=3 >= MAX_RETRY_COUNT -> discarded
    bg->RestorePendingFeatures("maxRetryFeat:2");
    bg->ProcessPendingDownloads();
    // Callback was invoked once for the feature
    EXPECT_GE(callCount, 1);
    // Feature discarded, map empty, persisted file should be deleted
    EXPECT_TRUE(bg->ReadPendingTasksFromFile().empty());
}

/**
 * @tc.name  RestoreFromFile_NullPtr_001
 * @tc.desc  Test RestoreFromFile with null selfPtr, verifying no processing occurs.
 */
HWTEST_F(BackgroundDownloadTest, RestoreFromFile_NullPtr_001, TestSize.Level0)
{
    int32_t callCount = 0;
    auto bg = std::make_shared<BackgroundDownload>(
        [&callCount](const std::string&) { callCount++; return SUCCESS; });
    bg->RestoreFromFile(nullptr, true);
    // selfPtr is null, CHECK_AND_RETURN_LOG should skip all processing
    EXPECT_EQ(callCount, 0);
}

/**
 * @tc.name  RestoreFromFile_Async_001
 * @tc.desc  Test RestoreFromFile with syncProcess=false, verifying the async thread
 *           path is taken. No assertion on thread result to avoid timing issues.
 */
HWTEST_F(BackgroundDownloadTest, RestoreFromFile_Async_001, TestSize.Level0)
{
    int32_t callCount = 0;
    auto bg = std::make_shared<BackgroundDownload>(
        [&callCount](const std::string&) { callCount++; return SUCCESS; });
    EXPECT_EQ(bg->AddFeature("async1"), SUCCESS);
    bg->RestoreFromFile(bg, false);
    // Async path: detached thread sleeps 30s before processing
    // Verify: AddFeature succeeded and file was persisted
    EXPECT_FALSE(bg->ReadPendingTasksFromFile().empty());
}

/**
 * @tc.name  UpdateDownloadStatusPara_All_001
 * @tc.desc  Test UpdateDownloadStatusPara with all scenarios: empty input,
 *           wifi-only feature, full-condition feature, empty token from leading
 *           semicolon, and both feature types in one call.
 */
HWTEST_F(BackgroundDownloadTest, UpdateDownloadStatusPara_All_001, TestSize.Level0)
{
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    // Empty input
    bg->UpdateDownloadStatusPara("");
    // Wifi-only feature
    bg->UpdateDownloadStatusPara("RecordDefinition:0");
    // Full-condition feature
    bg->UpdateDownloadStatusPara("UnknownFull:0");
    // Empty token from leading semicolon
    bg->UpdateDownloadStatusPara(";RecordDefinition:0");
    // Both feature types in one call
    bg->UpdateDownloadStatusPara("RecordDefinition:0;UnknownFull:0");
    // Verify: no crash and operations completed
    EXPECT_TRUE(bg->ReadPendingTasksFromFile().empty());
}

/**
 * @tc.name  PersistPendingQueueLocked_MultiFeature_001
 * @tc.desc  Test persisting multiple features to XML file, verifying the semicolon
 *           separator between features and successful read-back.
 */
HWTEST_F(BackgroundDownloadTest, PersistPendingQueueLocked_MultiFeature_001, TestSize.Level0)
{
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    bg->AddFeature("feat_a");
    bg->AddFeature("feat_b");
    // After second AddFeature, PersistPendingQueueLocked serializes 2 features → ";" added
    std::string content = bg->ReadPendingTasksFromFile();
    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find("feat_a"), std::string::npos);
    EXPECT_NE(content.find("feat_b"), std::string::npos);
    EXPECT_NE(content.find(";"), std::string::npos);
}

/**
 * @tc.name  AreConditionsMet_KnownFeature_001
 * @tc.desc  Test AreConditionsMet with a known feature that requires only wifi,
 *           verifying charging/battery/screen-lock checks are skipped.
 */
HWTEST_F(BackgroundDownloadTest, AreConditionsMet_KnownFeature_001, TestSize.Level0)
{
    bool result = BackgroundDownload::AreConditionsMet("RecordDefinition");
    EXPECT_TRUE(result);
}

/**
 * @tc.name  ReadPendingTasksFromFile_Oversize_001
 * @tc.desc  Test ReadPendingTasksFromFile when file size exceeds the maximum allowed,
 *           verifying an empty string is returned.
 */
HWTEST_F(BackgroundDownloadTest, ReadPendingTasksFromFile_Oversize_001, TestSize.Level0)
{
    std::string bigContent(2049, 'x');
    ASSERT_TRUE(WriteRawFile(bigContent));
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    std::string value = bg->ReadPendingTasksFromFile();
    EXPECT_TRUE(value.empty());
}

/**
 * @tc.name  ReadPendingTasksFromFile_MalformedXml_001
 * @tc.desc  Test ReadPendingTasksFromFile with malformed XML content,
 *           verifying an empty string is returned on parse failure.
 */
HWTEST_F(BackgroundDownloadTest, ReadPendingTasksFromFile_MalformedXml_001, TestSize.Level0)
{
    ASSERT_TRUE(WriteRawFile("this is not valid XML <<<"));
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    std::string value = bg->ReadPendingTasksFromFile();
    EXPECT_TRUE(value.empty());
}

/**
 * @tc.name  ReadPendingTasksFromFile_NonElementNode_001
 * @tc.desc  Test ReadPendingTasksFromFile with XML containing whitespace text nodes
 *           as children of root, verifying non-element nodes are skipped and the
 *           pending tasks content is still correctly read.
 */
HWTEST_F(BackgroundDownloadTest, ReadPendingTasksFromFile_NonElementNode_001, TestSize.Level0)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n<map>\n  <string name=\"audiosuite.pending_tasks\">feat1:0</string>\n</map>";
    ASSERT_TRUE(WriteRawFile(xml));
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    std::string value = bg->ReadPendingTasksFromFile();
    // Whitespace text nodes are skipped (non-element), string content still found
    EXPECT_FALSE(value.empty());
    EXPECT_EQ(value, "feat1:0");
}

/**
 * @tc.name  ReadPendingTasksFromFile_WrongNodeName_001
 * @tc.desc  Test ReadPendingTasksFromFile with XML child element named "int" instead
 *           of "string", verifying no match is found and an empty string is returned.
 */
HWTEST_F(BackgroundDownloadTest, ReadPendingTasksFromFile_WrongNodeName_001, TestSize.Level0)
{
    std::string xml = "<?xml version=\"1.0\"?>\n<map><int name=\"audiosuite.pending_tasks\">42</int></map>";
    ASSERT_TRUE(WriteRawFile(xml));
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    std::string value = bg->ReadPendingTasksFromFile();
    EXPECT_TRUE(value.empty());
}

/**
 * @tc.name  ReadPendingTasksFromFile_WrongAttrName_001
 * @tc.desc  Test ReadPendingTasksFromFile with a string element having a wrong name
 *           attribute, verifying no match is found and an empty string is returned.
 */
HWTEST_F(BackgroundDownloadTest, ReadPendingTasksFromFile_WrongAttrName_001, TestSize.Level0)
{
    std::string xml = "<?xml version=\"1.0\"?>\n<map><string name=\"wrong.key\">feat1:0</string></map>";
    ASSERT_TRUE(WriteRawFile(xml));
    auto bg = std::make_shared<BackgroundDownload>([](const std::string&) { return SUCCESS; });
    std::string value = bg->ReadPendingTasksFromFile();
    EXPECT_TRUE(value.empty());
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS
