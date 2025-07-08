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
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioStreamCollectorUnitTest::SetUpTestCase(void) {}
void AudioStreamCollectorUnitTest::TearDownTestCase(void) {}
void AudioStreamCollectorUnitTest::SetUp(void) {}
void AudioStreamCollectorUnitTest::TearDown(void) {}


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
} // namespace AudioStandard
} // namespace OHOS
