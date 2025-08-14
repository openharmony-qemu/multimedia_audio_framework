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

#include "audio_stream_collector_unit_test.h"
#include "mock_audio_client_tracker.h"
#include "audio_system_manager.h"
#include "standard_client_tracker_proxy.h"
#include "audio_spatialization_service.h"
#include "audio_policy_log.h"
#include "audio_errors.h"
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
using namespace std;
using namespace std::chrono;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioStreamCollectorUnitTest::SetUpTestCase(void) {}
void AudioStreamCollectorUnitTest::TearDownTestCase(void) {}
void AudioStreamCollectorUnitTest::SetUp(void) {}
void AudioStreamCollectorUnitTest::TearDown(void) {}

constexpr uint32_t THP_EXTRA_SA_UID = 5000;

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)


/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateStreamState_001
* @tc.desc  : Test UpdateStreamState.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateStreamState_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererChangeInfo->backMute = false;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    collector.clientTracker_.insert(make_pair(2001, proxyObj));

    int32_t clientUid = 1001;
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamUsage = STREAM_USAGE_MUSIC;
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_PAUSE;
    int32_t ret = collector.UpdateStreamState(clientUid, streamSetStateEventInternal);
    EXPECT_EQ(ret, 0);
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_RESUME;
    ret = collector.UpdateStreamState(clientUid, streamSetStateEventInternal);
    EXPECT_EQ(ret, 0);
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_MUTE;
    ret = collector.UpdateStreamState(clientUid, streamSetStateEventInternal);
    EXPECT_EQ(ret, 0);
    streamSetStateEventInternal.streamSetState = StreamSetState::STREAM_UNMUTE;
    ret = collector.UpdateStreamState(clientUid, streamSetStateEventInternal);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: CheckVoiceCallActive_001
* @tc.desc  : Test CheckVoiceCallActive.
*/
HWTEST_F(AudioStreamCollectorUnitTest, CheckVoiceCallActive_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererChangeInfo->rendererState = RENDERER_PREPARED;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    int32_t sessionId = 0;
    bool ret = collector.CheckVoiceCallActive(sessionId);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: CheckVoiceCallActive_002
* @tc.desc  : Test CheckVoiceCallActive.
*/
HWTEST_F(AudioStreamCollectorUnitTest, CheckVoiceCallActive_002, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->clientUID = 1001;
    rendererChangeInfo1->createrUID = 1001;
    rendererChangeInfo1->sessionId = 2001;
    rendererChangeInfo1->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererChangeInfo1->rendererState = RENDERER_NEW;

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo2 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1001;
    rendererChangeInfo2->createrUID = 1001;
    rendererChangeInfo2->sessionId = 2001;
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    rendererChangeInfo2->rendererState = RENDERER_PREPARED;

    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);

    int32_t sessionId = 2001;
    bool ret = collector.CheckVoiceCallActive(sessionId);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: SetLowPowerVolume_001
* @tc.desc  : Test SetLowPowerVolume.
*/
HWTEST_F(AudioStreamCollectorUnitTest, SetLowPowerVolume_001, TestSize.Level1)
{
    AudioStreamCollector collector;

    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    collector.clientTracker_.insert(make_pair(0, proxyObj));

    int32_t streamId = 0;
    float volume = 0.5f;
    int32_t ret = collector.SetLowPowerVolume(streamId, volume);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetLowPowerVolume_001
* @tc.desc  : Test GetLowPowerVolume.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetLowPowerVolume_001, TestSize.Level1)
{
    AudioStreamCollector collector;

    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    collector.clientTracker_.insert(make_pair(0, proxyObj));

    int32_t streamId = 0;
    int32_t ret = collector.GetLowPowerVolume(streamId);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: SetOffloadMode_001
* @tc.desc  : Test SetOffloadMode.
*/
HWTEST_F(AudioStreamCollectorUnitTest, SetOffloadMode_001, TestSize.Level1)
{
    AudioStreamCollector collector;

    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    collector.clientTracker_.insert(make_pair(0, proxyObj));

    int32_t streamId = 0;
    int32_t state = 0;
    bool isAppBack = true;
    int32_t ret = collector.SetOffloadMode(streamId, state, isAppBack);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetSingleStreamVolume_001
* @tc.desc  : Test GetSingleStreamVolume.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetSingleStreamVolume_001, TestSize.Level1)
{
    AudioStreamCollector collector;

    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    collector.clientTracker_.insert(make_pair(0, proxyObj));

    int32_t streamId = 0;
    float ret = collector.GetSingleStreamVolume(streamId);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: WriteRenderStreamReleaseSysEvent_001
* @tc.desc  : Test WriteRenderStreamReleaseSysEvent.
*/
HWTEST_F(AudioStreamCollectorUnitTest, WriteRenderStreamReleaseSysEvent_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->sessionId = 2001;

    EXPECT_NO_THROW(
        collector.WriteRenderStreamReleaseSysEvent(rendererChangeInfo);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetRunningStreamUsageNoUltrasonic_001
* @tc.desc  : Test GetRunningStreamUsageNoUltrasonic.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetRunningStreamUsageNoUltrasonic_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    StreamUsage ret = collector.GetRunningStreamUsageNoUltrasonic();
    EXPECT_EQ(ret, STREAM_USAGE_MUSIC);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetRunningStreamUsageNoUltrasonic_002
* @tc.desc  : Test GetRunningStreamUsageNoUltrasonic.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetRunningStreamUsageNoUltrasonic_002, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererState = RENDERER_RUNNING;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ULTRASONIC;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    StreamUsage ret = collector.GetRunningStreamUsageNoUltrasonic();
    EXPECT_EQ(ret, STREAM_USAGE_INVALID);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetRunningSourceTypeNoUltrasonic_001
* @tc.desc  : Test GetRunningSourceTypeNoUltrasonic.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetRunningSourceTypeNoUltrasonic_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo->capturerState = CAPTURER_RUNNING;
    captureChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    collector.audioCapturerChangeInfos_.push_back(captureChangeInfo);

    SourceType ret = collector.GetRunningSourceTypeNoUltrasonic();
    EXPECT_EQ(ret, SOURCE_TYPE_MIC);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetRunningSourceTypeNoUltrasonic_002
* @tc.desc  : Test GetRunningSourceTypeNoUltrasonic.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetRunningSourceTypeNoUltrasonic_002, TestSize.Level1)
{
    AudioStreamCollector collector;
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo->capturerState = CAPTURER_RUNNING;
    captureChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_ULTRASONIC;
    collector.audioCapturerChangeInfos_.push_back(captureChangeInfo);

    SourceType ret = collector.GetRunningSourceTypeNoUltrasonic();
    EXPECT_EQ(ret, SOURCE_TYPE_INVALID);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateCapturerStream_001
* @tc.desc  : Test UpdateCapturerStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateCapturerStream_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    collector.capturerStatequeue_ = {{{0, 0}, 0}};

    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo->clientUID = 1001;
    captureChangeInfo->createrUID = 1001;
    captureChangeInfo->sessionId = 2001;
    captureChangeInfo->inputDeviceInfo = DEVICE_TYPE_MIC;
    captureChangeInfo->appTokenId = 3001;
    collector.audioCapturerChangeInfos_.push_back(move(captureChangeInfo));

    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.clientUID = 1001;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = 2001;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo = DEVICE_TYPE_INVALID;
    int32_t ret = collector.UpdateCapturerStream(streamChangeInfo);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateCapturerStream_002
* @tc.desc  : Test UpdateCapturerStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateCapturerStream_002, TestSize.Level1)
{
    AudioStreamCollector collector;
    collector.capturerStatequeue_ = {{{0, 0}, 0}};

    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    captureChangeInfo->clientUID = 1001;
    captureChangeInfo->createrUID = 1001;
    captureChangeInfo->sessionId = 2001;
    captureChangeInfo->inputDeviceInfo = DEVICE_TYPE_MIC;
    captureChangeInfo->appTokenId = 3001;
    collector.audioCapturerChangeInfos_.push_back(move(captureChangeInfo));

    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioCapturerChangeInfo.clientUID = 1;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = 1;
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo = DEVICE_TYPE_INVALID;
    int32_t ret = collector.UpdateCapturerStream(streamChangeInfo);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: SendCapturerInfoEvent_003
* @tc.desc  : Test SendCapturerInfoEvent.
*/
HWTEST_F(AudioStreamCollectorUnitTest, SendCapturerInfoEvent_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    EXPECT_NO_THROW(collector.SendCapturerInfoEvent(audioCapturerChangeInfos));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: CheckRendererInfoChanged_001
* @tc.desc  : Test CheckRendererInfoChanged.
*/
HWTEST_F(AudioStreamCollectorUnitTest, CheckRendererInfoChanged_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 1001;
    rendererChangeInfo->rendererInfo.isOffloadAllowed = true;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.sessionId = 1001;
    streamChangeInfo.audioRendererChangeInfo.rendererInfo.isOffloadAllowed = false;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    EXPECT_TRUE(collector.CheckRendererInfoChanged(streamChangeInfo));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: ResetRingerModeMute_002
* @tc.desc  : Test ResetRingerModeMute.
*/
HWTEST_F(AudioStreamCollectorUnitTest, ResetRingerModeMute_002, TestSize.Level4)
{
    AudioStreamCollector collector;
    RendererState rendererState = RENDERER_STOPPED;
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    EXPECT_NO_THROW(collector.ResetRingerModeMute(rendererState, streamUsage));

    rendererState = RENDERER_RELEASED;
    EXPECT_NO_THROW(collector.ResetRingerModeMute(rendererState, streamUsage));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateRendererDeviceInfo_003
* @tc.desc  : Test UpdateRendererDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateRendererDeviceInfo_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto outputDeviceInfo1 = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    auto outputDeviceInfo2 = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    outputDeviceInfo2->deviceType_ = DEVICE_TYPE_MAX;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    EXPECT_EQ(collector.UpdateRendererDeviceInfo(outputDeviceInfo1), SUCCESS);
    EXPECT_EQ(collector.UpdateRendererDeviceInfo(outputDeviceInfo2), SUCCESS);

    collector.audioPolicyServerHandler_.reset();
    EXPECT_EQ(collector.UpdateRendererDeviceInfo(outputDeviceInfo1), SUCCESS);
    EXPECT_EQ(collector.UpdateRendererDeviceInfo(outputDeviceInfo2), SUCCESS);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateCapturerDeviceInfo_003
* @tc.desc  : Test UpdateCapturerDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateCapturerDeviceInfo_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto outputDeviceInfo1 = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    auto outputDeviceInfo2 = std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    outputDeviceInfo2->deviceType_ = DEVICE_TYPE_MAX;
    auto capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
    collector.audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));
    EXPECT_EQ(collector.UpdateCapturerDeviceInfo(outputDeviceInfo1), SUCCESS);
    EXPECT_EQ(collector.UpdateCapturerDeviceInfo(outputDeviceInfo2), SUCCESS);

    collector.audioPolicyServerHandler_.reset();
    EXPECT_EQ(collector.UpdateCapturerDeviceInfo(outputDeviceInfo1), SUCCESS);
    EXPECT_EQ(collector.UpdateCapturerDeviceInfo(outputDeviceInfo2), SUCCESS);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateAppVolume_002
* @tc.desc  : Test UpdateAppVolume.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateAppVolume_002, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->appVolume = 0;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    int32_t appUid = 1000;
    int32_t volume = 7;
    collector.UpdateAppVolume(appUid, volume);
    EXPECT_EQ(rendererChangeInfo->appVolume, 0);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetSessionIdsOnRemoteDeviceByDeviceType_002
* @tc.desc  : Test GetSessionIdsOnRemoteDeviceByDeviceType.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetSessionIdsOnRemoteDeviceByDeviceType_002, TestSize.Level4)
{
    AudioStreamCollector collector;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    std::set<int32_t> sessionIdSet = collector.GetSessionIdsOnRemoteDeviceByDeviceType(deviceType);
    EXPECT_TRUE(sessionIdSet.empty());
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetCurrentCapturerChangeInfos_001
* @tc.desc  : Test GetCurrentCapturerChangeInfos.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetCurrentCapturerChangeInfos_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->clientUID = THP_EXTRA_SA_UID;
    collector.audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));

    std::vector<shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    collector.GetCurrentCapturerChangeInfos(capturerChangeInfos);
    EXPECT_TRUE(capturerChangeInfos.empty());
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: RegisteredRendererTrackerClientDied_001
* @tc.desc  : Test RegisteredRendererTrackerClientDied.
*/
HWTEST_F(AudioStreamCollectorUnitTest, RegisteredRendererTrackerClientDied_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = 1001;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    collector.clientTracker_.insert(std::make_pair(1001, nullptr));
    collector.audioPolicyServerHandler_.reset();

    int32_t uid = 1001;
    int32_t pid = 2001;
    EXPECT_NO_THROW(collector.RegisteredRendererTrackerClientDied(uid, pid));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: RegisteredCapturerTrackerClientDied_001
* @tc.desc  : Test RegisteredCapturerTrackerClientDied.
*/
HWTEST_F(AudioStreamCollectorUnitTest, RegisteredCapturerTrackerClientDied_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->sessionId = 1001;
    collector.audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));
    collector.clientTracker_.insert(std::make_pair(1001, nullptr));
    collector.audioPolicyServerHandler_.reset();

    int32_t uid = 1001;
    EXPECT_NO_THROW(collector.RegisteredCapturerTrackerClientDied(uid));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetAndCompareStreamType_001
* @tc.desc  : Test GetAndCompareStreamType.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetAndCompareStreamType_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    StreamUsage targetUsage = STREAM_USAGE_MEDIA;
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
    rendererInfo.streamUsage = STREAM_USAGE_INVALID;
    EXPECT_TRUE(collector.GetAndCompareStreamType(targetUsage, rendererInfo));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: ResumeStreamState_002
* @tc.desc  : Test ResumeStreamState.
*/
HWTEST_F(AudioStreamCollectorUnitTest, ResumeStreamState_002, TestSize.Level4)
{
    AudioStreamCollector collector;
    int32_t sessionId = 1;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = sessionId;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    auto mockAudioClientTracker = std::make_shared<MockAudioClientTracker>();
    EXPECT_CALL(*mockAudioClientTracker, UnmuteStreamImpl(_)).Times(1);
    collector.clientTracker_.insert(std::make_pair(sessionId, mockAudioClientTracker));
    EXPECT_EQ(collector.ResumeStreamState(), SUCCESS);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: UpdateStreamState_002
* @tc.desc  : Test UpdateStreamState.
*/
HWTEST_F(AudioStreamCollectorUnitTest, UpdateStreamState_002, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);

    int32_t clientUid = 1001;
    StreamSetStateEventInternal streamSetStateEventInternal;
    streamSetStateEventInternal.streamUsage = STREAM_USAGE_ALARM;
    EXPECT_EQ(collector.UpdateStreamState(clientUid, streamSetStateEventInternal), SUCCESS);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: IsStreamActive_001
* @tc.desc  : Test IsStreamActive.
*/
HWTEST_F(AudioStreamCollectorUnitTest, IsStreamActive_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    AudioStreamType volumeType = STREAM_DEFAULT;
    auto rendererChangeInfo1 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->rendererState = RENDERER_NEW;
    auto rendererChangeInfo2 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->rendererState = RENDERER_RUNNING;
    rendererChangeInfo2->rendererInfo.contentType = CONTENT_TYPE_UNKNOWN;
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    EXPECT_FALSE(collector.IsStreamActive(volumeType));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: IsVoiceCallActive_001
* @tc.desc  : Test IsVoiceCallActive.
*/
HWTEST_F(AudioStreamCollectorUnitTest, IsVoiceCallActive_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo1 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->rendererState = RENDERER_PREPARED;
    rendererChangeInfo1->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    EXPECT_TRUE(collector.IsVoiceCallActive());

    auto rendererChangeInfo2 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->rendererState = RENDERER_NEW;
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    auto rendererChangeInfo3 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->rendererState = RENDERER_PREPARED;
    rendererChangeInfo3->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    auto rendererChangeInfo4 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->rendererState = RENDERER_NEW;
    rendererChangeInfo4->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo5 = nullptr;
    collector.audioRendererChangeInfos_.clear();
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo5);
    EXPECT_FALSE(collector.IsVoiceCallActive());
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetRunningStream_001
* @tc.desc  : Test GetRunningStream.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetRunningStream_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererState = RENDERER_PREPARED;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo);
    AudioStreamType certainType = STREAM_DEFAULT;
    int32_t certainChannelCount = 0;
    EXPECT_EQ(collector.GetRunningStream(certainType, certainChannelCount), -1);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetVolumeTypeFromContentUsage_001
* @tc.desc  : Test GetVolumeTypeFromContentUsage.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetVolumeTypeFromContentUsage_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    AudioStreamType streamType = STREAM_DEFAULT;
    streamType = collector.GetVolumeTypeFromContentUsage(contentType, streamUsage);
    EXPECT_EQ(streamType, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: GetAllCapturerSessionIDForUID_001
* @tc.desc  : Test GetAllCapturerSessionIDForUID.
*/
HWTEST_F(AudioStreamCollectorUnitTest, GetAllCapturerSessionIDForUID_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    capturerChangeInfo->clientUID = 1001;

    int32_t uid = 2001;
    std::vector<uint32_t> sessionIDSet = collector.GetAllCapturerSessionIDForUID(uid);
    EXPECT_TRUE(sessionIDSet.empty());
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: IsVoipStreamActive_001
* @tc.desc  : Test IsVoipStreamActive.
*/
HWTEST_F(AudioStreamCollectorUnitTest, IsVoipStreamActive_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->rendererState = RENDERER_RUNNING;
    rendererChangeInfo1->rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    bool ret = collector.IsVoiceCallActive();

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo2 = nullptr;
    collector.audioRendererChangeInfos_.clear();
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    ret = collector.IsVoiceCallActive();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleAppStateChange_003
* @tc.desc  : Test HandleAppStateChange.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleAppStateChange_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    auto rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->clientUID = 1002;
    rendererChangeInfo1->clientPid = 2002;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1002;
    rendererChangeInfo2->clientPid = 2001;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->clientUID = 1001;
    rendererChangeInfo3->clientPid = 2002;
    auto rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->clientUID = 1001;
    rendererChangeInfo4->clientPid = 2001;
    rendererChangeInfo4->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    auto rendererChangeInfo5 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo5->clientUID = 1001;
    rendererChangeInfo5->clientPid = 2001;
    rendererChangeInfo5->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo6 = nullptr;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo5);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo6);

    int32_t uid = 1001;
    int32_t pid = 2001;
    bool mute = true;
    bool notifyMute = true;
    bool hasBackTask = true;
    EXPECT_NO_THROW(collector.HandleAppStateChange(uid, pid, mute, notifyMute, hasBackTask));

    VolumeUtils::SetPCVolumeEnable(true);
    EXPECT_NO_THROW(collector.HandleAppStateChange(uid, pid, mute, notifyMute, hasBackTask));
    VolumeUtils::SetPCVolumeEnable(false);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleAppStateChange_004
* @tc.desc  : Test HandleAppStateChange.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleAppStateChange_004, TestSize.Level4)
{
    AudioStreamCollector collector;
    int32_t sessionId = 3001;
    auto rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->clientUID = 1001;
    rendererChangeInfo1->clientPid = 2001;
    rendererChangeInfo1->sessionId = sessionId;
    rendererChangeInfo1->backMute = false;
    rendererChangeInfo1->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1001;
    rendererChangeInfo2->clientPid = 2001;
    rendererChangeInfo2->sessionId = sessionId;
    rendererChangeInfo2->backMute = true;
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);

    auto mockAudioClientTracker = std::make_shared<MockAudioClientTracker>();
    EXPECT_CALL(*mockAudioClientTracker, MuteStreamImpl(_)).Times(1);
    EXPECT_CALL(*mockAudioClientTracker, UnmuteStreamImpl(_)).Times(1);
    collector.clientTracker_.insert(std::make_pair(sessionId, mockAudioClientTracker));

    int32_t uid = 1001;
    int32_t pid = 2001;
    bool mute = true;
    bool notifyMute = true;
    bool hasBackTask = true;
    collector.HandleAppStateChange(uid, pid, mute, notifyMute, hasBackTask);
    EXPECT_TRUE(rendererChangeInfo1->backMute);
    rendererChangeInfo1->backMute = false;

    mute = false;
    collector.HandleAppStateChange(uid, pid, mute, notifyMute, hasBackTask);
    EXPECT_FALSE(rendererChangeInfo2->backMute);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleForegroundUnmute_001
* @tc.desc  : Test HandleForegroundUnmute.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleForegroundUnmute_001, TestSize.Level4)
{
    AudioStreamCollector collector;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = nullptr;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1002;
    rendererChangeInfo2->clientPid = 2001;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->clientUID = 1001;
    rendererChangeInfo3->clientPid = 2002;
    auto rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->clientUID = 1002;
    rendererChangeInfo4->clientPid = 2002;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);

    int32_t uid = 1001;
    int32_t pid = 2001;
    EXPECT_NO_THROW(collector.HandleForegroundUnmute(uid, pid));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleForegroundUnmute_002
* @tc.desc  : Test HandleForegroundUnmute.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleForegroundUnmute_002, TestSize.Level4)
{
    AudioStreamCollector collector;
    int32_t sessionId = 3001;
    auto rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->clientUID = 1001;
    rendererChangeInfo1->clientPid = 2001;
    rendererChangeInfo1->sessionId = sessionId;
    rendererChangeInfo1->backMute = false;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1001;
    rendererChangeInfo2->clientPid = 2001;
    rendererChangeInfo2->sessionId = sessionId;
    rendererChangeInfo2->backMute = true;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);

    int32_t uid = 1001;
    int32_t pid = 2001;
    auto mockAudioClientTracker = std::make_shared<MockAudioClientTracker>();
    EXPECT_CALL(*mockAudioClientTracker, UnmuteStreamImpl(_)).Times(1);
    collector.clientTracker_.insert(std::make_pair(sessionId, mockAudioClientTracker));
    EXPECT_NO_THROW(collector.HandleForegroundUnmute(uid, pid));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleFreezeStateChange_003
* @tc.desc  : Test HandleFreezeStateChange.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleFreezeStateChange_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = nullptr;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->createrUID = 1013;
    rendererChangeInfo2->clientPid = 2001;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->createrUID = 1013;
    rendererChangeInfo3->clientPid = 2002;
    auto rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->createrUID = 1001;
    rendererChangeInfo4->clientPid = 2001;
    auto rendererChangeInfo5 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo5->createrUID = 1001;
    rendererChangeInfo5->clientPid = 2002;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo5);

    int32_t pid = 2001;
    bool mute = true;
    bool hasSession = true;
    EXPECT_NO_THROW(collector.HandleFreezeStateChange(pid, mute, hasSession));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleFreezeStateChange_004
* @tc.desc  : Test HandleFreezeStateChange.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleFreezeStateChange_004, TestSize.Level4)
{
    AudioStreamCollector collector;
    int32_t sessionId = 3001;
    auto rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->createrUID = 1001;
    rendererChangeInfo1->clientPid = 2001;
    rendererChangeInfo1->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->createrUID = 1001;
    rendererChangeInfo2->clientPid = 2001;
    rendererChangeInfo2->sessionId = sessionId;
    rendererChangeInfo2->backMute = false;
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->createrUID = 1001;
    rendererChangeInfo3->clientPid = 2001;
    rendererChangeInfo3->sessionId = sessionId;
    rendererChangeInfo3->backMute = true;
    rendererChangeInfo3->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);

    int32_t pid = 2001;
    auto mockAudioClientTracker = std::make_shared<MockAudioClientTracker>();
    EXPECT_CALL(*mockAudioClientTracker, MuteStreamImpl(_)).Times(2);
    EXPECT_CALL(*mockAudioClientTracker, UnmuteStreamImpl(_)).Times(2);
    collector.clientTracker_.insert(std::make_pair(sessionId, mockAudioClientTracker));
    EXPECT_NO_THROW(collector.HandleFreezeStateChange(pid, true, true));
    EXPECT_TRUE(rendererChangeInfo2->backMute);
    rendererChangeInfo2->backMute = false;

    EXPECT_NO_THROW(collector.HandleFreezeStateChange(pid, false, false));
    EXPECT_FALSE(rendererChangeInfo3->backMute);
    rendererChangeInfo2->backMute = true;

    EXPECT_NO_THROW(collector.HandleFreezeStateChange(pid, false, true));
    EXPECT_FALSE(rendererChangeInfo3->backMute);
    rendererChangeInfo2->backMute = true;

    EXPECT_NO_THROW(collector.HandleFreezeStateChange(pid, true, false));
    EXPECT_TRUE(rendererChangeInfo2->backMute);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleBackTaskStateChange_003
* @tc.desc  : Test HandleBackTaskStateChange.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleBackTaskStateChange_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = nullptr;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1002;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->clientUID = 1001;
    rendererChangeInfo3->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    auto rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->clientUID = 1001;
    rendererChangeInfo4->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);

    int32_t uid = 1001;
    EXPECT_NO_THROW(collector.HandleBackTaskStateChange(uid, true));
    EXPECT_NO_THROW(collector.HandleBackTaskStateChange(uid, false));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleBackTaskStateChange_004
* @tc.desc  : Test HandleBackTaskStateChange.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleBackTaskStateChange_004, TestSize.Level4)
{
    AudioStreamCollector collector;
    int32_t sessionId = 3001;
    auto rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->clientUID = 1001;
    rendererChangeInfo1->sessionId = sessionId;
    rendererChangeInfo1->backMute = false;
    rendererChangeInfo1->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1001;
    rendererChangeInfo2->sessionId = sessionId;
    rendererChangeInfo2->backMute = true;
    rendererChangeInfo2->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);

    int32_t uid = 1001;
    auto mockAudioClientTracker = std::make_shared<MockAudioClientTracker>();
    EXPECT_CALL(*mockAudioClientTracker, UnmuteStreamImpl(_)).Times(1);
    collector.clientTracker_.insert(std::make_pair(sessionId, mockAudioClientTracker));
    collector.HandleBackTaskStateChange(uid, true);
    EXPECT_FALSE(rendererChangeInfo2->backMute);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleStartStreamMuteState_003
* @tc.desc  : Test HandleStartStreamMuteState.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleStartStreamMuteState_003, TestSize.Level4)
{
    AudioStreamCollector collector;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo1 = nullptr;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1002;
    rendererChangeInfo2->clientPid = 2001;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->clientUID = 1001;
    rendererChangeInfo3->clientPid = 2002;
    auto rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->clientUID = 1002;
    rendererChangeInfo4->clientPid = 2002;
    auto rendererChangeInfo5 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo5->clientUID = 1001;
    rendererChangeInfo5->clientPid = 2001;
    rendererChangeInfo5->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    auto rendererChangeInfo6 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo6->clientUID = 1001;
    rendererChangeInfo6->clientPid = 2001;
    rendererChangeInfo6->rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo5);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo6);

    int32_t uid = 1001;
    int32_t pid = 2001;
    EXPECT_NO_THROW(collector.HandleStartStreamMuteState(uid, pid, true, true));
    EXPECT_NO_THROW(collector.HandleStartStreamMuteState(uid, pid, true, false));
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: HandleStartStreamMuteState_004
* @tc.desc  : Test HandleStartStreamMuteState.
*/
HWTEST_F(AudioStreamCollectorUnitTest, HandleStartStreamMuteState_004, TestSize.Level4)
{
    AudioStreamCollector collector;
    int32_t sessionId = 3001;
    auto rendererChangeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo1->clientUID = 1001;
    rendererChangeInfo1->clientPid = 2001;
    rendererChangeInfo1->createrUID = 1001;
    rendererChangeInfo1->sessionId = sessionId;
    rendererChangeInfo1->backMute = false;
    auto rendererChangeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo2->clientUID = 1001;
    rendererChangeInfo2->clientPid = 2001;
    rendererChangeInfo2->createrUID = 1001;
    rendererChangeInfo2->sessionId = sessionId;
    rendererChangeInfo2->backMute = true;
    auto rendererChangeInfo3 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo3->clientUID = 1001;
    rendererChangeInfo3->clientPid = 2001;
    rendererChangeInfo3->createrUID = 1013;
    rendererChangeInfo3->sessionId = sessionId;
    rendererChangeInfo3->backMute = false;
    auto rendererChangeInfo4 = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo4->clientUID = 1001;
    rendererChangeInfo4->clientPid = 2001;
    rendererChangeInfo4->createrUID = 1013;
    rendererChangeInfo4->sessionId = sessionId;
    rendererChangeInfo4->backMute = true;
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo1);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo2);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo3);
    collector.audioRendererChangeInfos_.push_back(rendererChangeInfo4);

    int32_t uid = 1001;
    int32_t pid = 2001;
    auto mockAudioClientTracker = std::make_shared<MockAudioClientTracker>();
    EXPECT_CALL(*mockAudioClientTracker, MuteStreamImpl(_)).Times(1);
    EXPECT_CALL(*mockAudioClientTracker, UnmuteStreamImpl(_)).Times(2);
    collector.clientTracker_.insert(std::make_pair(sessionId, mockAudioClientTracker));
    collector.HandleStartStreamMuteState(uid, pid, true, false);
    EXPECT_TRUE(rendererChangeInfo1->backMute);
    rendererChangeInfo1->backMute = false;

    collector.HandleStartStreamMuteState(uid, pid, false, false);
    EXPECT_FALSE(rendererChangeInfo2->backMute);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: ExistStreamForPipe_001
* @tc.desc  : Test ExistStreamForPipe.
*/
HWTEST_F(AudioStreamCollectorUnitTest, ExistStreamForPipe_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioPipeType pipeType = AudioPipeType::PIPE_TYPE_UNKNOWN;
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->createrUID = 1001;
    rendererChangeInfo->clientUID = 2001;
    rendererChangeInfo->rendererInfo.pipeType = AudioPipeType::PIPE_TYPE_MULTICHANNEL;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    bool result = collector.ExistStreamForPipe(pipeType);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: IsTransparentCapture_002
* @tc.desc  : Test IsTransparentCapture.
*/
HWTEST_F(AudioStreamCollectorUnitTest, IsTransparentCapture_002, TestSize.Level1)
{
    AudioStreamCollector collector;

    uint32_t clientUid = 5000 - 10;
    bool ret = collector.IsTransparentCapture(clientUid);
    EXPECT_EQ(false, ret);

    clientUid = 5000;
    ret = collector.IsTransparentCapture(clientUid);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: ResetCapturerStreamDeviceInfo_001
* @tc.desc  : Test ResetCapturerStreamDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, ResetCapturerStreamDeviceInfo_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioDeviceDescriptor updataDesc(AudioDeviceDescriptor::DEVICE_INFO);
    updataDesc.deviceType_ = DEVICE_TYPE_EARPIECE;
    updataDesc.macAddress_ = "12345";
    updataDesc.networkId_ = "12345";

    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    (rendererChangeInfo->inputDeviceInfo).deviceType_ = DEVICE_TYPE_EARPIECE;
    (rendererChangeInfo->inputDeviceInfo).macAddress_ = "12345";
    (rendererChangeInfo->inputDeviceInfo).networkId_ = "12345";
    rendererChangeInfo->capturerState = CAPTURER_STOPPED;
    collector.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    EXPECT_NO_THROW(
        collector.ResetCapturerStreamDeviceInfo(updataDesc);
    );
}

/**
* @tc.name  : Test AudioStreamCollector.
* @tc.number: ResetRendererStreamDeviceInfo_001
* @tc.desc  : Test ResetRendererStreamDeviceInfo.
*/
HWTEST_F(AudioStreamCollectorUnitTest, ResetRendererStreamDeviceInfo_001, TestSize.Level1)
{
    AudioStreamCollector collector;
    AudioDeviceDescriptor updataDesc(AudioDeviceDescriptor::DEVICE_INFO);
    updataDesc.deviceType_ = DEVICE_TYPE_EARPIECE;
    updataDesc.macAddress_ = "12345";
    updataDesc.networkId_ = "12345";

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    (rendererChangeInfo->outputDeviceInfo).deviceType_ = DEVICE_TYPE_EARPIECE;
    (rendererChangeInfo->outputDeviceInfo).macAddress_ = "12345";
    (rendererChangeInfo->outputDeviceInfo).networkId_ = "12345";
    rendererChangeInfo->rendererState = RENDERER_STOPPED;
    collector.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    EXPECT_NO_THROW(
        collector.ResetRendererStreamDeviceInfo(updataDesc);
    );
}
} // namespace AudioStandard
} // namespace OHOS
