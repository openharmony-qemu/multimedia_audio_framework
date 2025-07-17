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

#ifndef AUDIO_CAPTURER_ADAPTER_UNIT_TEST_H
#define AUDIO_CAPTURER_ADAPTER_UNIT_TEST_H

#include "gtest/gtest.h"

namespace OHOS {
namespace AudioStandard {

static int g_writeOverflowNum = 1000;
class TestAudioStreamStub : public FastAudioStream {
public:
    TestAudioStreamStub() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return state_; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    bool ReleaseAudioStream(bool releaseRunner, bool isSwitchStream) override { return true; }

    State state_ = State::RUNNING;
};

class TestAudioStreamStub1 : public FastAudioStream {
public:
    TestAudioStreamStub1() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return state_; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    bool ReleaseAudioStream(bool releaseRunner, bool isSwitchStream) override { return true; }

    State state_ = State::PAUSED;
};

class TestAudioStreamStub2 : public FastAudioStream {
public:
    TestAudioStreamStub2() : FastAudioStream(AudioStreamType::STREAM_MUSIC,
        AudioMode::AUDIO_MODE_RECORD, 0) {}
    uint32_t GetOverflowCount() override { return g_writeOverflowNum; }
    State GetState() override { return state_; }
    bool StopAudioStream() override { return true; }
    bool StartAudioStream(StateChangeCmdType cmdType,
        AudioStreamDeviceChangeReasonExt reason) override { return true; }
    bool ReleaseAudioStream(bool releaseRunner, bool isSwitchStream) override { return true; }

    State state_ = State::STOPPED;
};


class AudioCapturerAdapterUnitTest : public testing::Test {
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

#endif // AUDIO_CAPTURER_ADAPTER_UNIT_TEST_H
