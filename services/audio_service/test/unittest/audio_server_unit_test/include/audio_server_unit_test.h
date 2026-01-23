/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SERVER_UNIT_TEST_H
#define AUDIO_SERVER_UNIT_TEST_H

#include "gtest/gtest.h"
#include "source/i_audio_capture_source.h"

namespace OHOS {
namespace AudioStandard {

class TestAudioCaptureSource : public IAudioCaptureSource {
public:
    int32_t Init(const IAudioSourceAttr& attr) override {return 0;};

    void DeInit(void) override {};
    bool IsInited(void) override {return true;};

    int32_t Start(void) override {return 0;};
    int32_t Stop(void) override {return 0;};
    int32_t Resume(void) override {return 0;};
    int32_t Pause(void) override {return 0;};
    int32_t Flush(void) override {return 0;};
    int32_t Reset(void) override {return 0;};
    int32_t CaptureFrame(char* frame, uint64_t requestBytes, uint64_t& replyBytes) override {return 0;};

    std::string GetAudioParameter(const AudioParamKey key, const std::string& condition) override {return "";};
    void SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value) override {};

    int32_t SetVolume(float left, float right)override {return 0;};
    int32_t GetVolume(float &left, float &right)override {return 0;};
    int32_t SetMute(bool isMute)override {return 0;};
    int32_t GetMute(bool &isMute)override {return 0;};

    uint64_t GetTransactionId(void) override {return 0;};
    int32_t GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec) override {return 0;};
    float GetMaxAmplitude(void) override {return 0;};

    int32_t UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size) override {return 0;};
    int32_t UpdateAppsUid(const std::vector<int32_t>& appsUid) override {return 0;};

    void DumpInfo(std::string& dumpString) override {};
};

class AudioServerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVER_UNIT_TEST_H