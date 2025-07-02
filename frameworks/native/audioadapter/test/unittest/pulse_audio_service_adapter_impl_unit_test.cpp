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
#include "gmock/gmock.h"
#include "audio_errors.h"
#include "pulse_audio_service_adapter_impl.h"

using namespace testing::ext;
using namespace testing;
using namespace std;
namespace OHOS {
namespace AudioStandard {

class PulseAudioServiceAdapterImplUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioServiceAdapterCallbackTest : public AudioServiceAdapterCallback {
public:
    void OnAudioStreamRemoved(const uint64_t sessionID) override { return; }
    void OnSetVolumeDbCb() override { return; }
};

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_001
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_001, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    auto ret = pulseAudioServiceAdapterImpl->ConnectToPulseAudio();
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_002
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_002, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pulseAudioServiceAdapterImpl->mContext = nullptr;

    int32_t audioHandleIndex = 0;
    auto ret = pulseAudioServiceAdapterImpl->CloseAudioPort(audioHandleIndex);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_003
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_003, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    int32_t audioHandleIndex = 5;
    auto ret = pulseAudioServiceAdapterImpl->CloseAudioPort(audioHandleIndex);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_004
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_004, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    int32_t audioHandleIndex = 5;
    auto ret = pulseAudioServiceAdapterImpl->CloseAudioPort(audioHandleIndex);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_005
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_005, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pulseAudioServiceAdapterImpl->mContext = nullptr;

    string audioPortName = "abc";
    bool isSuspend = false;
    auto ret = pulseAudioServiceAdapterImpl->SuspendAudioDevice(audioPortName, isSuspend);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_006
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_006, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    string sinkName = "abc";
    bool isMute = false;
    bool isSync = true;
    auto ret = pulseAudioServiceAdapterImpl->SetSinkMute(sinkName, isMute, isSync);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_007
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_007, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    string sinkName = "abc";
    bool isMute = false;
    bool isSync = false;
    auto ret = pulseAudioServiceAdapterImpl->SetSinkMute(sinkName, isMute, isSync);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_008
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_008, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pulseAudioServiceAdapterImpl->mContext = nullptr;

    string name = "abc";
    auto ret = pulseAudioServiceAdapterImpl->SetDefaultSink(name);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_009
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_009, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    string name = "abc";
    auto ret = pulseAudioServiceAdapterImpl->SetDefaultSink(name);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_010
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_010, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pulseAudioServiceAdapterImpl->mContext = nullptr;

    string name = "abc";
    auto ret = pulseAudioServiceAdapterImpl->SetDefaultSource(name);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_011
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_011, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);

    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    string name = "abc";
    auto ret = pulseAudioServiceAdapterImpl->SetDefaultSource(name);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_012
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_012, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *c = pa_context_new(mainloop_api, "MyAudioApp");
    pa_sink_info *i = nullptr;
    int eol = -1;
    PulseAudioServiceAdapterImpl::UserData *userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    pulseAudioServiceAdapterImpl->PaGetSinksCb(c, i, eol, userdata);
    eol = 1;
    pulseAudioServiceAdapterImpl->PaGetSinksCb(c, i, eol, userdata);
    ASSERT_NE(userdata->thiz->mMainLoop, nullptr);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_013
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_013, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    auto ret = pulseAudioServiceAdapterImpl->GetAllSinks();
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_014
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_014, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    uint32_t sinkInputId = 0;
    uint32_t sinkIndex = 0;
    std::string sinkName = "";
    auto ret = pulseAudioServiceAdapterImpl->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
    EXPECT_EQ(ret, ERROR);

    sinkName = "abc";
    ret = pulseAudioServiceAdapterImpl->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_015
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_015, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pulseAudioServiceAdapterImpl->mContext = nullptr;

    uint32_t sourceOutputId = 0;
    uint32_t sourceIndex = 0;
    std::string sourceName = "";
    auto ret = pulseAudioServiceAdapterImpl->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
    EXPECT_EQ(ret, ERROR);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    sourceName = "";
    ret = pulseAudioServiceAdapterImpl->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
    EXPECT_EQ(ret, ERROR);

    sourceName = "abc";
    ret = pulseAudioServiceAdapterImpl->MoveSourceOutputByIndexOrName(sourceOutputId, sourceIndex, sourceName);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_016
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_016, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    auto ret = pulseAudioServiceAdapterImpl->GetAllSinkInputs();
    EXPECT_EQ(ret.size(), 0);
    auto ret2 = pulseAudioServiceAdapterImpl->GetAllSourceOutputs();
    EXPECT_EQ(ret2.size(), 0);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_017
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_017, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pulseAudioServiceAdapterImpl->mContext = nullptr;
    pulseAudioServiceAdapterImpl->mMainLoop = nullptr;
    pulseAudioServiceAdapterImpl->Disconnect();

    pulseAudioServiceAdapterImpl->mMainLoop = pa_threaded_mainloop_new();
    ASSERT_NE(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pulseAudioServiceAdapterImpl->mContext = pa_context_new(mainloop_api, "MyAudioApp");
    ASSERT_NE(pulseAudioServiceAdapterImpl->mContext, nullptr);

    pulseAudioServiceAdapterImpl->Disconnect();
    ASSERT_EQ(pulseAudioServiceAdapterImpl->mMainLoop, nullptr);
    ASSERT_EQ(pulseAudioServiceAdapterImpl->mContext, nullptr);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_018
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_018, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    string streamType = "voice_call";
    auto ret = pulseAudioServiceAdapterImpl->GetIdByStreamType(streamType);
    EXPECT_EQ(ret, STREAM_VOICE_CALL);

    streamType = "abc";
    ret = pulseAudioServiceAdapterImpl->GetIdByStreamType(streamType);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_019
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_019, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *c = pa_context_new(mainloop_api, "MyAudioApp");

    PulseAudioServiceAdapterImpl::UserData *userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    uint32_t idx = PA_INVALID_INDEX;
    pulseAudioServiceAdapterImpl->PaModuleLoadCb(c, idx, userdata);
    EXPECT_EQ(userdata->idx, PA_INVALID_INDEX);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_020
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_020, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *c = pa_context_new(mainloop_api, "MyAudioApp");
    pa_source_output_info *i = nullptr;
    int eol = -1;
    PulseAudioServiceAdapterImpl::UserData *userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    pulseAudioServiceAdapterImpl->PaGetSourceOutputNoSignalCb(c, i, eol, userdata);
    ASSERT_NE(userdata, nullptr);

    eol = 1;
    userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();
    pulseAudioServiceAdapterImpl->PaGetSourceOutputNoSignalCb(c, i, eol, userdata);
    ASSERT_NE(userdata, nullptr);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_021
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_021, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *c = pa_context_new(mainloop_api, "MyAudioApp");
    pa_sink_input_info *i = nullptr;
    int eol = -1;
    PulseAudioServiceAdapterImpl::UserData *userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    pulseAudioServiceAdapterImpl->PaGetAllSinkInputsCb(c, i, eol, userdata);
    ASSERT_NE(userdata, nullptr);

    eol = 1;
    pulseAudioServiceAdapterImpl->PaGetAllSinkInputsCb(c, i, eol, userdata);
    ASSERT_NE(userdata, nullptr);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_022
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_022, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *c = pa_context_new(mainloop_api, "MyAudioApp");
    pa_subscription_event_type_t t =
        static_cast<pa_subscription_event_type_t>(PA_SUBSCRIPTION_EVENT_CHANGE | PA_SUBSCRIPTION_EVENT_SINK);
    uint32_t idx = 0;
    PulseAudioServiceAdapterImpl::UserData *userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    EXPECT_NE(t & PA_SUBSCRIPTION_EVENT_TYPE_MASK, PA_SUBSCRIPTION_EVENT_NEW);
    EXPECT_NE(t & PA_SUBSCRIPTION_EVENT_TYPE_MASK, PA_SUBSCRIPTION_EVENT_REMOVE);
    pulseAudioServiceAdapterImpl->ProcessSourceOutputEvent(c, t, idx, userdata);
    ASSERT_NE(userdata, nullptr);

    t = static_cast<pa_subscription_event_type_t>(PA_SUBSCRIPTION_EVENT_NEW | PA_SUBSCRIPTION_EVENT_SINK);
    EXPECT_EQ(t & PA_SUBSCRIPTION_EVENT_TYPE_MASK, PA_SUBSCRIPTION_EVENT_NEW);
    pulseAudioServiceAdapterImpl->ProcessSourceOutputEvent(c, t, idx, userdata);
    ASSERT_NE(userdata, nullptr);

    userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    t = static_cast<pa_subscription_event_type_t>(PA_SUBSCRIPTION_EVENT_REMOVE | PA_SUBSCRIPTION_EVENT_SINK);
    EXPECT_EQ(t & PA_SUBSCRIPTION_EVENT_TYPE_MASK, PA_SUBSCRIPTION_EVENT_REMOVE);
    pulseAudioServiceAdapterImpl->ProcessSourceOutputEvent(c, t, idx, userdata);
    ASSERT_NE(userdata, nullptr);
}

/**
* @tc.name  : Test PulseAudioServiceAdapterImplUnitTest API
* @tc.number: PulseAudioServiceAdapterImplUnitTest_023
* @tc.desc  : Test PulseAudioServiceAdapterImplUnitTest interface.
*/
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_023, TestSize.Level1)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    pa_mainloop* mainloop = pa_mainloop_new();
    pa_mainloop_api* mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *c = pa_context_new(mainloop_api, "MyAudioApp");
    pa_subscription_event_type_t t =
        static_cast<pa_subscription_event_type_t>(PA_SUBSCRIPTION_EVENT_NEW | PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT);
    uint32_t idx = 0;
    PulseAudioServiceAdapterImpl::UserData *userdata = new PulseAudioServiceAdapterImpl::UserData();
    userdata->thiz = new PulseAudioServiceAdapterImpl(audioServiceAdapterCallback);
    userdata->thiz->mMainLoop = pa_threaded_mainloop_new();

    EXPECT_EQ(t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK, PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT);
    pulseAudioServiceAdapterImpl->PaSubscribeCb(c, t, idx, userdata);

    t = static_cast<pa_subscription_event_type_t>(PA_SUBSCRIPTION_EVENT_NEW | PA_SUBSCRIPTION_EVENT_MODULE);
    pulseAudioServiceAdapterImpl->PaSubscribeCb(c, t, idx, userdata);

    t = static_cast<pa_subscription_event_type_t>(PA_SUBSCRIPTION_EVENT_REMOVE | PA_SUBSCRIPTION_EVENT_SINK_INPUT);
    pulseAudioServiceAdapterImpl->PaSubscribeCb(c, t, idx, userdata);
    ASSERT_NE(userdata, nullptr);
}

/**
 * @tc.name  : Test PulseAudioServiceAdapterImpl API
 * @tc.number: PulseAudioServiceAdapterImplUnitTest_024
 * @tc.desc  : Test SetThreadPriority interface.
 */
HWTEST(PulseAudioServiceAdapterImplUnitTest, PulseAudioServiceAdapterImplUnitTest_024, TestSize.Level3)
{
    std::unique_ptr<AudioServiceAdapterCallback> audioServiceAdapterCallback =
        std::make_unique<AudioServiceAdapterCallbackTest>();
    auto pulseAudioServiceAdapterImpl = std::make_shared<PulseAudioServiceAdapterImpl>(audioServiceAdapterCallback);
    ASSERT_NE(pulseAudioServiceAdapterImpl, nullptr);

    // start test
    bool res = pulseAudioServiceAdapterImpl->SetThreadPriority();
    EXPECT_EQ(res, true);
    res = pulseAudioServiceAdapterImpl->SetThreadPriority();
    EXPECT_EQ(res, true);
}

} // namespace AudioStandard
} // namespace OHOS