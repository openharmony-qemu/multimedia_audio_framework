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
#include "audio_pipe_selector_route_selector_unit_test.h"

#include <unordered_map>
#include <unordered_set>

#include "audio_concurrency_parser.h"
#include "audio_core_config_manager.h"
#include "audio_errors.h"
#include "audio_policy_interface.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_enum.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const uint32_t TEST_STREAM_1_SESSION_ID = 100001;
static const uint32_t TEST_STREAM_2_SESSION_ID = 100002;
static const int32_t TEST_APP_UID = 1001;
static const int32_t TEST_APP_PID = 2002;
static const uint32_t TEST_APP_TOKEN_ID = 3003;
static const uint64_t TEST_APP_FULL_TOKEN_ID = 4004;
static const char *TEST_BUNDLE_NAME = "com.test.route.selector";
static const char *TEST_DEVICE_ID = "test_device";

void AudioPipeSelectorRouteSelectorUnitTest::SetUpTestCase(void)
{
    AudioCoreConfigManager &manager = AudioCoreConfigManager::GetInstance();
    manager.Init(true);
    AudioPolicyConfigData &configData = manager.GetAudioPolicyConfigData();

    auto policyAdapterInfo = std::make_shared<PolicyAdapterInfo>();
    policyAdapterInfo->adapterName = "primary";

    auto adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    adapterPipeInfo->name_ = "primary";
    adapterPipeInfo->role_ = PIPE_ROLE_OUTPUT;
    adapterPipeInfo->adapterInfo_ = policyAdapterInfo;
    policyAdapterInfo->pipeInfos.push_back(adapterPipeInfo);

    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = AudioSampleFormat::SAMPLE_S16LE;
    propInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    propInfo->channels_ = AudioChannel::STEREO;
    propInfo->channelLayout_ = AudioChannelLayout::CH_LAYOUT_STEREO;
    propInfo->pipeInfo_ = adapterPipeInfo;
    adapterPipeInfo->streamPropInfos_.push_back(propInfo);

    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->adapterInfo_ = policyAdapterInfo;
    deviceInfo->supportPipeMap_.insert({AUDIO_OUTPUT_FLAG_NORMAL, adapterPipeInfo});
    policyAdapterInfo->deviceInfos.push_back(deviceInfo);

    configData.adapterInfoMap[AudioAdapterType::TYPE_PRIMARY] = policyAdapterInfo;

    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    configData.deviceInfoMap[deviceKey] = deviceInfoSet;
}

class TestAudioRouteSelector : public AudioRouteSelector {
public:
    TestAudioRouteSelector(int32_t ret, int32_t selectResult) : ret_(ret), selectResult_(selectResult) {}
    ~TestAudioRouteSelector() override = default;

    int32_t OnAudioRouteSelect(const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo,
        int32_t &selectResult) override
    {
        lastRouteSelectInfo_ = routeSelectInfo;
        callCount_++;
        selectResult = selectResult_;
        return ret_;
    }

    int32_t ret_;
    int32_t selectResult_;
    int32_t callCount_ = 0;
    std::shared_ptr<AudioRouteSelectInfo> lastRouteSelectInfo_ = nullptr;
};

class UsageAwareAudioRouteSelector : public AudioRouteSelector {
public:
    ~UsageAwareAudioRouteSelector() override = default;

    int32_t OnAudioRouteSelect(const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo,
        int32_t &selectResult) override
    {
        callCount_++;
        if (routeSelectInfo == nullptr) {
            return ERR_INVALID_PARAM;
        }
        auto it = usageToSelectResult_.find(routeSelectInfo->streamUsage);
        selectResult = it == usageToSelectResult_.end() ? ROUTE_SELECT_RESULT_SUCCESS : it->second;
        return SUCCESS;
    }

    std::unordered_map<StreamUsage, int32_t> usageToSelectResult_;
    int32_t callCount_ = 0;
};

static void ResetRouteSelectorTestState()
{
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    AudioPipeManager::GetPipeManager()->switchStreamMap_.clear();
    AudioPipeManager::GetPipeManager()->modemCommunicationIdMap_.clear();
    AudioPipeSelector::GetPipeSelector()->UnsetAudioRouteSelectorCallback();
}

static std::shared_ptr<AudioDeviceDescriptor> MakeSpeakerDeviceDesc()
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->deviceRole_ = OUTPUT_DEVICE;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    return deviceDesc;
}

static std::shared_ptr<AudioDeviceDescriptor> MakeMicDeviceDesc()
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_MIC;
    deviceDesc->deviceRole_ = INPUT_DEVICE;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    return deviceDesc;
}

static std::shared_ptr<AudioStreamDescriptor> MakeRouteSelectorTestStreamDesc(AudioMode audioMode,
    uint32_t sessionId = TEST_STREAM_1_SESSION_ID)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = audioMode;
    streamDesc->bundleName_ = TEST_BUNDLE_NAME;
    streamDesc->appInfo_.appUid = TEST_APP_UID;
    streamDesc->appInfo_.appPid = TEST_APP_PID;
    streamDesc->appInfo_.appTokenId = TEST_APP_TOKEN_ID;
    streamDesc->appInfo_.appFullTokenId = TEST_APP_FULL_TOKEN_ID;
    streamDesc->appInfo_.deviceId = TEST_DEVICE_ID;
    streamDesc->sessionId_ = sessionId;
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    if (audioMode == AUDIO_MODE_PLAYBACK) {
        streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
        streamDesc->newDeviceDescs_.push_back(MakeSpeakerDeviceDesc());
    } else {
        streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
        streamDesc->newDeviceDescs_.push_back(MakeMicDeviceDesc());
    }
    return streamDesc;
}

static AudioPipeSelector::AudioRouteSelectorCallback MakeRouteSelectorCallback(
    const std::shared_ptr<AudioRouteSelector> &selector)
{
    return [selector](const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo, int32_t &selectResult) -> int32_t {
        CHECK_AND_RETURN_RET_LOG(selector != nullptr, ERR_OPERATION_FAILED, "selector is nullptr");
        return selector->OnAudioRouteSelect(routeSelectInfo, selectResult);
    };
}

/**
 * @tc.name: NotifyAudioRouteSelect_001
 * @tc.desc: Test NotifyAudioRouteSelect when callback is registered.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, NotifyAudioRouteSelect_001, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_SUCCESS);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_RECORD);
    int32_t selectResult = AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE;
    int32_t ret = audioPipeSelector->NotifyAudioRouteSelect(streamDesc, selectResult);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(selectResult, AudioRouteSelector::ROUTE_SELECT_RESULT_SUCCESS);
    ASSERT_NE(routeSelector->lastRouteSelectInfo_, nullptr);
    EXPECT_EQ(routeSelector->callCount_, 1);
    EXPECT_EQ(routeSelector->lastRouteSelectInfo_->audioMode, AUDIO_MODE_RECORD);
    EXPECT_EQ(routeSelector->lastRouteSelectInfo_->streamUsage, STREAM_USAGE_UNKNOWN);
    EXPECT_EQ(routeSelector->lastRouteSelectInfo_->sourceType, SOURCE_TYPE_MIC);
    EXPECT_EQ(routeSelector->lastRouteSelectInfo_->routeFlag, streamDesc->routeFlag_);
    EXPECT_EQ(routeSelector->lastRouteSelectInfo_->appInfo.appUid, streamDesc->appInfo_.appUid);
    EXPECT_EQ(routeSelector->lastRouteSelectInfo_->bundleName, streamDesc->bundleName_);

    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name: NotifyAudioRouteSelect_002
 * @tc.desc: Test NotifyAudioRouteSelect when callback is not registered.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, NotifyAudioRouteSelect_002, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK);
    int32_t selectResult = AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE;
    int32_t ret = audioPipeSelector->NotifyAudioRouteSelect(streamDesc, selectResult);

    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(selectResult, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
}

/**
 * @tc.name: ApplyRouteSelectDecision_001
 * @tc.desc: Test ApplyRouteSelectDecision fallback to normal route for record stream.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ApplyRouteSelectDecision_001, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_FALLBACK_NORMAL);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_RECORD);
    EXPECT_TRUE(audioPipeSelector->ApplyRouteSelectDecision(streamDesc, true));
    EXPECT_EQ(streamDesc->routeFlag_, AUDIO_INPUT_FLAG_NORMAL);

    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name: ApplyRouteSelectDecision_002
 * @tc.desc: Test ApplyRouteSelectDecision reject create for new stream.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ApplyRouteSelectDecision_002, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK);
    EXPECT_FALSE(audioPipeSelector->ApplyRouteSelectDecision(streamDesc, true));

    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name: ApplyRouteSelectDecision_003
 * @tc.desc: Test ApplyRouteSelectDecision reject create fallback to normal for existing stream.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ApplyRouteSelectDecision_003, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK);
    EXPECT_TRUE(audioPipeSelector->ApplyRouteSelectDecision(streamDesc, false));
    EXPECT_EQ(streamDesc->routeFlag_, AUDIO_OUTPUT_FLAG_NORMAL);

    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name: ApplyRouteSelectDecision_004
 * @tc.desc: Test ApplyRouteSelectDecision when callback returns error.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ApplyRouteSelectDecision_004, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        ERR_OPERATION_FAILED, AudioRouteSelector::ROUTE_SELECT_RESULT_FALLBACK_NORMAL);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_RECORD);
    EXPECT_TRUE(audioPipeSelector->ApplyRouteSelectDecision(streamDesc, true));
    EXPECT_EQ(routeSelector->callCount_, 1);

    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name: ApplyRouteSelectDecision_005
 * @tc.desc: Test ApplyRouteSelectDecision keeps default route flag when callback is not registered.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ApplyRouteSelectDecision_005, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK);
    uint32_t defaultRouteFlag = audioPipeSelector->GetRouteFlagByStreamDesc(streamDesc);

    EXPECT_TRUE(audioPipeSelector->ApplyRouteSelectDecision(streamDesc, true));
    EXPECT_FALSE(streamDesc->GetRouteSelectRejectedFlag());
    EXPECT_EQ(streamDesc->routeFlag_, defaultRouteFlag);
}

/**
 * @tc.name: ScanPipeListForStreamDesc_001
 * @tc.desc: Test ScanPipeListForStreamDesc returns false when streamDesc is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ScanPipeListForStreamDesc_001, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfoList;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;

    EXPECT_FALSE(audioPipeSelector->ScanPipeListForStreamDesc(pipeInfoList, streamDesc));
}

/**
 * @tc.name: FetchPipeAndExecute_003
 * @tc.desc: Test FetchPipeAndExecute returns empty result when streamDesc is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, FetchPipeAndExecute_003, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;

    std::vector<std::shared_ptr<AudioPipeInfo>> result = audioPipeSelector->FetchPipeAndExecute(streamDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: FetchPipesAndExecute_RouteSelector_001
 * @tc.desc: Test FetchPipesAndExecute falls back reject-create streams to normal in reroute path.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, FetchPipesAndExecute_RouteSelector_001, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<UsageAwareAudioRouteSelector>();
    routeSelector->usageToSelectResult_[STREAM_USAGE_ALARM] = AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE;
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    ASSERT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto acceptedStream = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK, TEST_STREAM_1_SESSION_ID);
    acceptedStream->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    auto rejectedStream = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK, TEST_STREAM_2_SESSION_ID);
    rejectedStream->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {acceptedStream, rejectedStream};

    auto result = audioPipeSelector->FetchPipesAndExecute(streamDescs);

    ASSERT_EQ(streamDescs.size(), 2u);
    EXPECT_FALSE(acceptedStream->GetRouteSelectRejectedFlag());
    EXPECT_FALSE(rejectedStream->GetRouteSelectRejectedFlag());
    EXPECT_EQ(rejectedStream->routeFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_EQ(routeSelector->callCount_, 2);
    ASSERT_EQ(result.size(), 1u);
    ASSERT_NE(result[0], nullptr);
    ASSERT_EQ(result[0]->streamDescriptors_.size(), 2u);
}

/**
 * @tc.name: DecideFinalRouteFlag_ExistingStream_001
 * @tc.desc: Test existing stream reject-create falls back to normal instead of being removed.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, DecideFinalRouteFlag_ExistingStream_001, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    auto routeSelector = std::make_shared<TestAudioRouteSelector>(
        SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
    auto callback = MakeRouteSelectorCallback(routeSelector);
    ASSERT_NE(callback, nullptr);
    ASSERT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto existingStream = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK, TEST_STREAM_1_SESSION_ID);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {existingStream};
    audioPipeSelector->DecideFinalRouteFlag(streamDescs);

    ASSERT_EQ(streamDescs.size(), 1u);
    EXPECT_EQ(streamDescs[0]->sessionId_, TEST_STREAM_1_SESSION_ID);
    EXPECT_FALSE(streamDescs[0]->GetRouteSelectRejectedFlag());
    EXPECT_EQ(streamDescs[0]->routeFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_EQ(routeSelector->callCount_, 1);
}

/**
 * @tc.name: RouteSelectorCallback_Expired_001
 * @tc.desc: Test expired weak_ptr callback falls back to original route-select behavior.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, RouteSelectorCallback_Expired_001, TestSize.Level1)
{
    ResetRouteSelectorTestState();
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::weak_ptr<AudioRouteSelector> weakSelector;
    {
        auto routeSelector = std::make_shared<TestAudioRouteSelector>(
            SUCCESS, AudioRouteSelector::ROUTE_SELECT_RESULT_REJECT_CREATE);
        weakSelector = routeSelector;
    }
    AudioPipeSelector::AudioRouteSelectorCallback callback =
        [weakSelector](const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo, int32_t &selectResult) -> int32_t {
        auto selector = weakSelector.lock();
        CHECK_AND_RETURN_RET_LOG(selector != nullptr, ERR_CALLBACK_NOT_REGISTERED, "selector is nullptr");
        return selector->OnAudioRouteSelect(routeSelectInfo, selectResult);
    };
    ASSERT_EQ(audioPipeSelector->SetAudioRouteSelectorCallback(callback), SUCCESS);

    auto streamDesc = MakeRouteSelectorTestStreamDesc(AUDIO_MODE_PLAYBACK, TEST_STREAM_1_SESSION_ID);
    int32_t selectResult = AudioRouteSelector::ROUTE_SELECT_RESULT_SUCCESS;
    int32_t ret = audioPipeSelector->NotifyAudioRouteSelect(streamDesc, selectResult);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_REGISTERED);
    EXPECT_EQ(audioPipeSelector->UnsetAudioRouteSelectorCallback(), SUCCESS);
}

/**
 * @tc.name  : ProcessRendererAndCapturerConcurrency_001
 * @tc.number: ProcessRendererAndCapturerConcurrency_001
 * @tc.desc  : Test ProcessRendererAndCapturerConcurrency skips downgrade when action is PLAY_BOTH.
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ProcessRendererAndCapturerConcurrency_001, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();

    // Setup a FAST_VOIP capturer pipe
    std::shared_ptr<AudioPipeInfo> voipPipe = std::make_shared<AudioPipeInfo>();
    voipPipe->routeFlag_ = AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST;
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(voipPipe);

    // Incoming FAST renderer stream
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->sessionId_ = 200001;

    // When action is PLAY_BOTH, the stream should NOT be downgraded
    audioPipeSelector->ProcessRendererAndCapturerConcurrency(streamDesc, PLAY_BOTH);
    EXPECT_EQ(streamDesc->routeFlag_, AUDIO_OUTPUT_FLAG_FAST);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
 * @tc.name  : ProcessRendererAndCapturerConcurrency_002
 * @tc.number: ProcessRendererAndCapturerConcurrency_002
 * @tc.desc  : Test ProcessRendererAndCapturerConcurrency downgrades FAST to NORMAL when
 *             action is not PLAY_BOTH and FAST_VOIP capturer pipe exists.
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ProcessRendererAndCapturerConcurrency_002, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();

    // Setup a FAST_VOIP capturer pipe
    std::shared_ptr<AudioPipeInfo> voipPipe = std::make_shared<AudioPipeInfo>();
    voipPipe->routeFlag_ = AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST;
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(voipPipe);

    // Incoming FAST renderer stream
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->sessionId_ = 200002;

    // When action is CONCEDE_INCOMING, the stream SHOULD be downgraded
    audioPipeSelector->ProcessRendererAndCapturerConcurrency(streamDesc, CONCEDE_INCOMING);
    EXPECT_EQ(streamDesc->routeFlag_, AUDIO_OUTPUT_FLAG_NORMAL);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
 * @tc.name  : ProcessRendererAndCapturerConcurrency_003
 * @tc.number: ProcessRendererAndCapturerConcurrency_003
 * @tc.desc  : Test ProcessRendererAndCapturerConcurrency does not downgrade when
 *             no FAST_VOIP capturer pipe exists.
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ProcessRendererAndCapturerConcurrency_003, TestSize.Level1)
{
    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();

    // No FAST_VOIP capturer pipe - only a normal output pipe
    std::shared_ptr<AudioPipeInfo> normalPipe = std::make_shared<AudioPipeInfo>();
    normalPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(normalPipe);

    // Incoming FAST renderer stream
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->sessionId_ = 200003;

    // No FAST_VOIP capturer, stream should NOT be downgraded even with non-PLAY_BOTH action
    audioPipeSelector->ProcessRendererAndCapturerConcurrency(streamDesc, CONCEDE_INCOMING);
    EXPECT_EQ(streamDesc->routeFlag_, AUDIO_OUTPUT_FLAG_FAST);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
 * @tc.name  : ProcessConcurrency_002
 * @tc.number: ProcessConcurrency_002
 * @tc.desc  : Test ProcessConcurrency with FAST renderer + FAST_VOIP capturer,
 *             when action is PLAY_BOTH the renderer should not be downgraded.
 */
HWTEST_F(AudioPipeSelectorRouteSelectorUnitTest, ProcessConcurrency_002, TestSize.Level1)
{
    // Configure concurrency: PIPE_TYPE_OUT_LOWLATENCY vs PIPE_TYPE_IN_VOIP -> PLAY_BOTH
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> ruleMap = {
        {{PIPE_TYPE_OUT_LOWLATENCY, PIPE_TYPE_IN_VOIP}, PLAY_BOTH}
    };
    AudioConcurrencyManager &audioConcurrencyManager = AudioConcurrencyManager::GetInstance();
    audioConcurrencyManager.concurrencyConfigMap_ = ruleMap;
    AudioPipeManager::GetPipeManager()->curPipeList_.clear();

    // Setup a FAST_VOIP capturer pipe with a stream on it
    std::shared_ptr<AudioPipeInfo> voipPipe = std::make_shared<AudioPipeInfo>();
    voipPipe->routeFlag_ = AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST;
    voipPipe->adapterName_ = "test_adapter";
    auto voipStream = std::make_shared<AudioStreamDescriptor>();
    voipStream->sessionId_ = 300001;
    voipPipe->streamDescriptors_.push_back(voipStream);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(voipPipe);

    // Existing FAST_VOIP capturer stream
    auto existingStream = std::make_shared<AudioStreamDescriptor>();
    existingStream->routeFlag_ = AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST;
    existingStream->audioMode_ = AUDIO_MODE_RECORD;
    existingStream->sessionId_ = 300002;

    // Incoming FAST renderer stream
    auto incomingStream = std::make_shared<AudioStreamDescriptor>();
    incomingStream->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    incomingStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    incomingStream->sessionId_ = 300003;

    auto audioPipeSelector = AudioPipeSelector::GetPipeSelector();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamsToMove;
    bool ret = audioPipeSelector->ProcessConcurrency(existingStream, incomingStream, streamsToMove);

    // With PLAY_BOTH action, ProcessRendererAndCapturerConcurrency should skip,
    // FAST renderer should NOT be downgraded
    EXPECT_EQ(incomingStream->routeFlag_, AUDIO_OUTPUT_FLAG_FAST);
    EXPECT_FALSE(ret);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}
} // namespace AudioStandard
} // namespace OHOS
