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

#ifndef AUDIO_LOOPBACK_WRAPPER_UNIT_TEST_H
#define AUDIO_LOOPBACK_WRAPPER_UNIT_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_loopback_wrapper.h"
#include "audio_loopback_private.h"
#include "iloopback.h"
#include "loopback_callback_stub.h"

namespace OHOS {
namespace AudioStandard {

class MockILoopback : public ILoopback {
public:
    MOCK_METHOD(ErrCode, Enable, (bool enable, int32_t &ret), (override));
    MOCK_METHOD(ErrCode, GetStatus, (int32_t &status, int32_t &ret), (override));
    MOCK_METHOD(ErrCode, GetVolume, (float &volume, int32_t &ret), (override));
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
};

class MockAudioLoopbackPrivate : public AudioLoopbackPrivate {
public:
    MockAudioLoopbackPrivate(AudioLoopbackMode mode, const AppInfo &appInfo) : AudioLoopbackPrivate(mode, appInfo) {}
    MOCK_METHOD(bool, Enable, (bool enable), (override));
    MOCK_METHOD(AudioLoopbackStatus, GetStatus, (), (override));
    MOCK_METHOD(int32_t, SetVolume, (float volume), (override));
    MOCK_METHOD(float, GetVolume, (), (override));
    MOCK_METHOD(int32_t, SetAudioLoopbackCallback, (const std::shared_ptr<AudioLoopbackCallback> &callback),
        (override));
    MOCK_METHOD(int32_t, RemoveAudioLoopbackCallback, (), (override));
    MOCK_METHOD(bool, SetReverbPreset, (AudioLoopbackReverbPreset preset), (override));
    MOCK_METHOD(AudioLoopbackReverbPreset, GetReverbPreset, (), (override));
    MOCK_METHOD(bool, SetEqualizerPreset, (AudioLoopbackEqualizerPreset preset), (override));
    MOCK_METHOD(AudioLoopbackEqualizerPreset, GetEqualizerPreset, (), (override));
    MOCK_METHOD(std::vector<AudioDevicePair>, GetSupportedDevicePairs, (), (override));
    MOCK_METHOD(AudioDevicePair, GetPreferredDevicePair, (), (override));
};

class MockAudioLoopbackCallback : public AudioLoopbackCallback {
public:
    MOCK_METHOD(void, OnStatusChange, (const AudioLoopbackStatus status, const StateChangeCmdType cmdType), (override));
};

class TestAudioLoopbackWrapper : public AudioLoopbackWrapper {
public:
    TestAudioLoopbackWrapper(AudioLoopbackMode mode, LoopbackType type, const AppInfo &appInfo)
        : AudioLoopbackWrapper(mode, type, appInfo) {}

    void SetLoopbackProxy(sptr<ILoopback> proxy) { loopbackProxy_ = proxy; }
    void SetPrivate(std::shared_ptr<AudioLoopbackPrivate> priv) { private_ = priv; }
    sptr<ILoopback> GetLoopbackProxy() { return loopbackProxy_; }
    std::shared_ptr<AudioLoopbackPrivate> GetPrivate() { return private_; }
    LoopbackType GetType() { return type_; }
    AudioLoopbackMode GetMode() { return mode_; }
};

class LoopbackCallbackUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioLoopbackWrapperUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AppInfo appInfo_;
    sptr<MockILoopback> mockLoopback_;
    std::shared_ptr<MockAudioLoopbackPrivate> mockPrivate_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_LOOPBACK_WRAPPER_UNIT_TEST_H