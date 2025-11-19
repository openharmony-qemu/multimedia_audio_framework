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
#include <gtest/gtest.h>
#include "dfx/audio_stream_concurrency_detector.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioStreamConcurrencyDetectorTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name  : Test FlushReportFlag API
 * @tc.type  : FUNC
 * @tc.number: FlushReportFlag_001
 */
HWTEST(AudioStreamConcurrencyDetectorTest, FlushReportFlag_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    cfg.appInfo.appUid = 1;
    cfg.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    uint32_t fakeStreamId = 888;

    AudioStreamConcurrencyDetector::GetInstance().UpdateWriteTime(cfg, fakeStreamId);
    EXPECT_GT(AudioStreamConcurrencyDetector::GetInstance().streamConcurrInfoMap_.size(), 0);

    AudioStreamConcurrencyDetector::GetInstance().RemoveStream(cfg, fakeStreamId);
}

/**
 * @tc.name  : Test ReportHisysEvent API
 * @tc.type  : FUNC
 * @tc.number: ReportHisysEvent_001
 */
HWTEST(AudioStreamConcurrencyDetectorTest, ReportHisysEvent_001, TestSize.Level1)
{
    AudioProcessConfig cfg;
    uint32_t fakeUid = 1;
    StreamUsage fakeUsage = STREAM_USAGE_MEDIA;
    uint32_t fakeIdStart = 888;
    AudioStreamConcurrDetectorRecordInfo fakeInfo;
    fakeInfo.startTime = 1;
    fakeInfo.updateTime = threshold + 1;

    AudioStreamConcurrDetectorReportInfo info;
    info.uid = fakeUid;
    info.usage = fakeUsage;

    for (unsigned int i = 0; i <= maxStreamNums; i++) {
        AudioStreamConcurrencyDetector::GetInstance().streamConcurrInfoMap_[fakeUid][fakeUsage][fakeIdStart + i] =
            fakeInfo;
        info.streamIds.push_back(fakeIdStart + i);
    }

    AudioProcessConfig config;
    config.appInfo.appUid = fakeUid;
    config.rendererInfo.streamUsage = fakeUsage;

    for (unsigned int i = 0; i <= maxStreamNums; i++) {
        AudioStreamConcurrencyDetector::GetInstance().UpdateWriteTime(config, fakeIdStart + i);
    }

    sleep(3);

    for (unsigned int i = 0; i <= maxStreamNums; i++) {
        AudioStreamConcurrencyDetector::GetInstance().UpdateWriteTime(config, fakeIdStart + i);
    }
    EXPECT_GT(AudioStreamConcurrencyDetector::GetInstance().streamConcurrInfoMap_.size(), 0);


    for (unsigned int i = 0; i <= maxStreamNums; i++) {
        AudioStreamConcurrencyDetector::GetInstance().RemoveStream(config, fakeIdStart + i);
    }
}

}
}