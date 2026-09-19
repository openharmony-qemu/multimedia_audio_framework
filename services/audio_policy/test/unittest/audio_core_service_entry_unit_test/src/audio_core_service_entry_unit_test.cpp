/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "audio_core_service_entry_unit_test.h"

#include "audio_core_config_manager.h"
#include "audio_policy_config_parser.h"

#include <future>
#include <gmock/gmock.h>
#include <thread>
#include <unordered_map>
#include "stream_flag_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t NUM5 = 5;
static constexpr uint32_t TEST_SESSION_ID = 100001;

static std::shared_ptr<AudioCoreService::EventEntry> CreateEventEntry(
    const std::shared_ptr<AudioCoreService> &audioCoreService)
{
    return std::make_shared<AudioCoreService::EventEntry>(
        audioCoreService, audioCoreService->readyConditionNotifier_);
}

static std::string MakeRequiredPipeKey(const std::string &adapterName, uint32_t routeFlag)
{
    return adapterName + "#" + std::to_string(routeFlag);
}

static void InitRequiredPipes(const std::shared_ptr<AudioCoreService> &audioCoreService)
{
    audioCoreService->pipeManager_->SetRequiredPipesReadyCallback([audioCoreService]() {
        audioCoreService->NotifyRequiredPipesReady();
    });
    audioCoreService->pipeManager_->InitRequiredPipes();
}

class MockPolicyAudioXmlNode : public AudioXmlNode {
public:
    MockPolicyAudioXmlNode() = default;
    ~MockPolicyAudioXmlNode() override = default;
    MOCK_METHOD(std::shared_ptr<AudioXmlNode>, GetChildrenNode, (), (override));
    MOCK_METHOD(std::shared_ptr<AudioXmlNode>, GetCopyNode, (), (override));
    MOCK_METHOD(int32_t, Config, (const char *fileName, const char *encoding, int32_t options), (override));
    MOCK_METHOD(void, MoveToNext, (), (override));
    MOCK_METHOD(void, MoveToChildren, (), (override));
    MOCK_METHOD(bool, IsNodeValid, (), (override));
    MOCK_METHOD(bool, HasProp, (const char *propName), (override));
    MOCK_METHOD(int32_t, GetProp, (const char *propName, std::string &result), (override));
    MOCK_METHOD(int32_t, GetContent, (std::string &result), (override));
    MOCK_METHOD(std::string, GetName, (), (override));
    MOCK_METHOD(bool, CompareName, (const char *propName), (override));
    MOCK_METHOD(bool, IsElementNode, (), (override));
    MOCK_METHOD(void, FreeDoc, (), (override));
    MOCK_METHOD(void, FreeXmlString, (char *str), (override));
};

static void SetPipeProp(std::unordered_map<std::string, std::string> &props, bool principal)
{
    props["name"] = principal ? "primary_output" : "primary_input";
    props["role"] = principal ? "output" : "input";
    props["principal"] = principal ? "true" : "false";
    props["supportDevices"] = "";
}

static void ExpectParsePipeNode(const std::shared_ptr<MockPolicyAudioXmlNode> &node,
    const std::shared_ptr<MockPolicyAudioXmlNode> &childNode, std::unordered_map<std::string, std::string> &props)
{
    EXPECT_CALL(*node, MoveToChildren()).Times(1);
    EXPECT_CALL(*node, IsNodeValid()).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));
    EXPECT_CALL(*node, IsElementNode()).WillOnce(testing::Return(true));
    EXPECT_CALL(*node, GetCopyNode()).WillOnce(testing::Return(childNode));
    EXPECT_CALL(*node, GetProp(testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([&props](const char *propName, std::string &result) {
            result = props[propName];
            return SUCCESS;
        }));
    EXPECT_CALL(*node, MoveToNext()).Times(1);
    EXPECT_CALL(*childNode, MoveToChildren()).Times(1);
    EXPECT_CALL(*childNode, IsNodeValid()).WillOnce(testing::Return(false));
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_001
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_START;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_002
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    SessionOperation operation = SessionOperation::SESSION_OPERATION_PAUSE;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
    ret = eventEntry->UpdateSessionOperation(sessionId, SESSION_OPERATION_STANDBY);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_003
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_STOP);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_STOP);
    SessionOperation operation = SessionOperation::SESSION_OPERATION_STOP;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_004
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_RELEASE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_RELEASE);
    SessionOperation operation = SessionOperation::SESSION_OPERATION_RELEASE;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_005
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = static_cast<SessionOperation>(NUM5);

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_006
 * @tc.desc  : Test AudioCoreService::EventEntry::OnServiceConnected()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioServiceIndex serviceIndex = HDI_SERVICE_INDEX;

    eventEntry->OnServiceConnected(serviceIndex);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_007
 * @tc.desc  : Test AudioCoreService::EventEntry::OnServiceDisconnected()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioServiceIndex serviceIndex = HDI_SERVICE_INDEX;

    eventEntry->OnServiceDisconnected(serviceIndex);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_008
 * @tc.desc  : Test AudioCoreService::EventEntry::createRendererClient()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    StreamFlagInfo flagInfo;
    uint32_t sessionId = 0;
    std::string networkId = "";

    InitRequiredPipes(audioCoreService);
    eventEntry->NotifyServiceReady();
    auto ret = eventEntry->CreateRendererClient(streamDesc, flagInfo, sessionId, networkId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_009
 * @tc.desc  : Test AudioCoreService::EventEntry::createCapturerClient()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;

    InitRequiredPipes(audioCoreService);
    eventEntry->NotifyServiceReady();
    auto ret = eventEntry->CreateCapturerClient(streamDesc, audioFlag, sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_001
 * @tc.desc  : Test required pipe added state matches adapter and route flag.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto pipeManager = std::make_shared<AudioPipeManager>();
    pipeManager->requiredPipesInited_ = true;
    pipeManager->requiredPipeKeys_.insert(MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL));

    auto inputPipe = std::make_shared<AudioPipeInfo>();
    inputPipe->adapterName_ = "primary";
    inputPipe->name_ = "primary_input";
    inputPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    auto outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->adapterName_ = "primary";
    outputPipe->name_ = "primary_output";
    outputPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;

    pipeManager->AddAudioPipeInfo(inputPipe);
    EXPECT_TRUE(pipeManager->addedRequiredPipeKeys_.empty());

    pipeManager->AddAudioPipeInfo(outputPipe);
    EXPECT_EQ(pipeManager->addedRequiredPipeKeys_.count(MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL)), 1);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_002
 * @tc.desc  : Test required pipe added state waits until all required pipes are added.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto pipeManager = std::make_shared<AudioPipeManager>();
    pipeManager->requiredPipesInited_ = true;
    pipeManager->requiredPipeKeys_.insert(MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL));
    pipeManager->requiredPipeKeys_.insert(MakeRequiredPipeKey("primary", AUDIO_INPUT_FLAG_NORMAL));

    auto outputPipe = std::make_shared<AudioPipeInfo>();
    outputPipe->adapterName_ = "primary";
    outputPipe->name_ = "primary_output";
    outputPipe->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    pipeManager->AddAudioPipeInfo(outputPipe);
    EXPECT_EQ(pipeManager->addedRequiredPipeKeys_.size(), 1);
    pipeManager->AddAudioPipeInfo(outputPipe);
    EXPECT_EQ(pipeManager->addedRequiredPipeKeys_.size(), 1);

    auto inputPipe = std::make_shared<AudioPipeInfo>();
    inputPipe->adapterName_ = "primary";
    inputPipe->name_ = "primary_input";
    inputPipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipeManager->AddAudioPipeInfo(inputPipe);
    EXPECT_EQ(pipeManager->addedRequiredPipeKeys_.size(), 2);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_003
 * @tc.desc  : Test required pipe ready notification is matched by adapter and route flag.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    audioCoreService->pipeManager_->SetRequiredPipesReadyCallback([audioCoreService]() {
        audioCoreService->NotifyRequiredPipesReady();
    });
    audioCoreService->pipeManager_->requiredPipesInited_ = true;
    audioCoreService->pipeManager_->requiredPipeKeys_.insert(
        MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL));
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->name_ = "primary_output";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    eventEntry->NotifyServiceReady();
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    StreamFlagInfo flagInfo;
    uint32_t sessionId = 0;
    std::string networkId = "";

    auto ret = eventEntry->CreateRendererClient(streamDesc, flagInfo, sessionId, networkId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_004
 * @tc.desc  : Test required pipes are initialized from config manager.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_004, TestSize.Level1)
{
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto oldRequiredPipes = configManager.requiredPipes_;
    configManager.requiredPipes_.clear();
    configManager.requiredPipes_.push_back({ "primary", AUDIO_OUTPUT_FLAG_NORMAL });

    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->name_ = "primary_output";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    audioCoreService->pipeManager_->curPipeList_.push_back(pipeInfo);

    InitRequiredPipes(audioCoreService);

    EXPECT_EQ(audioCoreService->pipeManager_->requiredPipeKeys_.count(
        MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL)), 1);
    EXPECT_EQ(
        audioCoreService->pipeManager_->addedRequiredPipeKeys_.count(
            MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL)), 1);
    configManager.requiredPipes_ = oldRequiredPipes;
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_005
 * @tc.desc  : Test required pipe config manager interface.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_005, TestSize.Level1)
{
    AudioCoreConfigManager configManager;
    RequiredPipeInfo pipeInfo = { "primary", AUDIO_INPUT_FLAG_NORMAL };

    configManager.AddRequiredPipe(pipeInfo);
    auto pipes = configManager.GetRequiredPipes();

    ASSERT_EQ(pipes.size(), 1);
    EXPECT_EQ(pipes.front().adapterName, pipeInfo.adapterName);
    EXPECT_EQ(pipes.front().routeFlag, pipeInfo.routeFlag);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_006
 * @tc.desc  : Test ready condition wait returns failed when waiter has invalid data.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    MultiConditionNotifier notifier;
    notifier.data_ = nullptr;
    AudioCoreService::EventEntry eventEntry(audioCoreService, notifier);

    EXPECT_EQ(eventEntry.WaitForReadyConditions(), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_007
 * @tc.desc  : Test ready condition wait returns retry when wait count exceeds limit.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);
    std::vector<std::future<int32_t>> waitTasks;

    for (int32_t i = 0; i < READY_CONDITION_WAIT_LIMIT; ++i) {
        waitTasks.emplace_back(std::async(std::launch::async, [eventEntry]() {
            return eventEntry->WaitForReadyConditions();
        }));
    }
    for (int32_t i = 0; i < 100; ++i) {
        if (eventEntry->readyConditionWaiter_.waitCount_.load() >= READY_CONDITION_WAIT_LIMIT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (eventEntry->readyConditionWaiter_.waitCount_.load() < READY_CONDITION_WAIT_LIMIT) {
        audioCoreService->readyConditionNotifier_.NotifySuccess(READY_CONDITION_MASK);
        for (auto &task : waitTasks) {
            EXPECT_EQ(task.get(), SUCCESS);
        }
        FAIL() << "wait tasks did not reach wait limit";
        return;
    }

    EXPECT_EQ(eventEntry->WaitForReadyConditions(), ERR_RETRY_IN_CLIENT);
    audioCoreService->readyConditionNotifier_.NotifySuccess(READY_CONDITION_MASK);

    for (auto &task : waitTasks) {
        EXPECT_EQ(task.get(), SUCCESS);
    }
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_008
 * @tc.desc  : Test ready condition wait returns failed when waiting times out.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    EXPECT_EQ(eventEntry->WaitForReadyConditions(), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_009
 * @tc.desc  : Test required pipe is notified by AudioPipeManager add event.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    audioCoreService->pipeManager_->SetRequiredPipesReadyCallback([audioCoreService]() {
        audioCoreService->NotifyRequiredPipesReady();
    });
    audioCoreService->pipeManager_->requiredPipesInited_ = true;
    audioCoreService->pipeManager_->requiredPipeKeys_.insert(
        MakeRequiredPipeKey("primary", AUDIO_OUTPUT_FLAG_NORMAL));

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->name_ = "primary_output";
    pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    eventEntry->NotifyServiceReady();
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    StreamFlagInfo flagInfo;
    uint32_t sessionId = 0;
    std::string networkId = "";
    EXPECT_EQ(eventEntry->CreateRendererClient(streamDesc, flagInfo, sessionId, networkId), SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_010
 * @tc.desc  : Test required pipe ready notification is matched by input pipe.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    audioCoreService->pipeManager_->SetRequiredPipesReadyCallback([audioCoreService]() {
        audioCoreService->NotifyRequiredPipesReady();
    });
    audioCoreService->pipeManager_->requiredPipesInited_ = true;
    audioCoreService->pipeManager_->requiredPipeKeys_.insert(
        MakeRequiredPipeKey("primary", AUDIO_INPUT_FLAG_NORMAL));

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    pipeInfo->name_ = "primary_input";
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    eventEntry->NotifyServiceReady();
    audioCoreService->pipeManager_->AddAudioPipeInfo(pipeInfo);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    EXPECT_EQ(eventEntry->CreateCapturerClient(streamDesc, audioFlag, sessionId), SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_011
 * @tc.desc  : Test required pipe config is parsed when XML flag is true.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_011, TestSize.Level1)
{
    AudioCoreConfigManager configManager;
    AudioPolicyConfigParser parser(configManager);
    auto adapterInfo = std::make_shared<PolicyAdapterInfo>();
    adapterInfo->adapterName = "primary";
    auto node = std::make_shared<MockPolicyAudioXmlNode>();
    auto childNode = std::make_shared<MockPolicyAudioXmlNode>();
    std::unordered_map<std::string, std::string> props;
    SetPipeProp(props, true);
    ExpectParsePipeNode(node, childNode, props);

    parser.ParsePipes(node, adapterInfo);
    auto pipes = configManager.GetRequiredPipes();

    ASSERT_EQ(adapterInfo->pipeInfos.size(), 1);
    EXPECT_TRUE(adapterInfo->pipeInfos.front()->principal_);
    ASSERT_EQ(pipes.size(), 1);
    EXPECT_EQ(pipes.front().adapterName, "primary");
    EXPECT_EQ(pipes.front().routeFlag, AUDIO_FLAG_NONE);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_RequiredPipe_012
 * @tc.desc  : Test required pipe config is not saved when XML flag is false.
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_RequiredPipe_012, TestSize.Level1)
{
    AudioCoreConfigManager configManager;
    AudioPolicyConfigParser parser(configManager);
    auto adapterInfo = std::make_shared<PolicyAdapterInfo>();
    adapterInfo->adapterName = "primary";
    auto node = std::make_shared<MockPolicyAudioXmlNode>();
    auto childNode = std::make_shared<MockPolicyAudioXmlNode>();
    std::unordered_map<std::string, std::string> props;
    SetPipeProp(props, false);
    ExpectParsePipeNode(node, childNode, props);

    parser.ParsePipes(node, adapterInfo);
    auto pipes = configManager.GetRequiredPipes();

    ASSERT_EQ(adapterInfo->pipeInfos.size(), 1);
    EXPECT_FALSE(adapterInfo->pipeInfos.front()->principal_);
    EXPECT_TRUE(pipes.empty());
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_010
 * @tc.desc  : Test AudioCoreService::EventEntry::SetDefaultOutputDevice()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    uint32_t sessionID = 0;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    bool isRunning = true;

    auto ret = eventEntry->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_011
 * @tc.desc  : Test AudioCoreService::EventEntry::GetModuleNameBySessionId()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_011, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;

    auto ret = eventEntry->GetModuleNameBySessionId(sessionId);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_012
 * @tc.desc  : Test AudioCoreService::EventEntry::GetProcessDeviceInfoBySessionId()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_012, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    AudioDeviceDescriptor deviceInfo;
    AudioStreamInfo info;
    bool isUltraFast = false;
    auto ret = eventEntry->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, isUltraFast);
    EXPECT_EQ(ret, SUCCESS);

    ret = eventEntry->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, isUltraFast, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_013
 * @tc.desc  : Test AudioCoreService::EventEntry::GenerateSessionId()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_013, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    auto ret = eventEntry->GenerateSessionId();
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_014
 * @tc.desc  : Test AudioCoreService::EventEntry::OnDeviceInfoUpdated()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_014, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioDeviceDescriptor desc;
    desc.isEnable_ = true;
    DeviceInfoUpdateCommand command = ENABLE_UPDATE;

    eventEntry->OnDeviceInfoUpdated(desc, command);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_015
 * @tc.desc  : Test AudioCoreService::EventEntry::SetAudioScene()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_015, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioScene audioScene = AUDIO_SCENE_RINGING;

    auto ret = eventEntry->SetAudioScene(audioScene);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: OnDeviceStatusUpdated_001
 * @tc.desc  : Test AudioCoreService::EventEntry::OnDeviceStatusUpdated()
 */
HWTEST(AudioCoreServiceEntryTest, OnDeviceStatusUpdated_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioDeviceDescriptor desc;
    bool isConnected = true;

    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: OnDeviceStatusUpdated_002
 * @tc.desc  : Test AudioCoreService::EventEntry::OnDeviceStatusUpdated()
 */
HWTEST(AudioCoreServiceEntryTest, OnDeviceStatusUpdated_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DStatusInfo statusInfo;
    bool isStop = true;

    eventEntry->OnDeviceStatusUpdated(statusInfo, isStop);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: OnDeviceStatusUpdated_003
 * @tc.desc  : Test AudioCoreService::EventEntry::OnDeviceStatusUpdated()
 */
HWTEST(AudioCoreServiceEntryTest, OnDeviceStatusUpdated_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.deviceCategory_ = BT_WATCH;
    bool isConnected = true;

    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: OnDeviceStatusUpdated_004
 * @tc.desc  : Test AudioCoreService::EventEntry::OnDeviceStatusUpdated()
 */
HWTEST(AudioCoreServiceEntryTest, OnDeviceStatusUpdated_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.deviceCategory_ = BT_HEADPHONE;
    bool isConnected = true;

    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_018
 * @tc.desc  : Test AudioCoreService::EventEntry::OnMicrophoneBlockedUpdate()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_018, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DeviceType devType = DEVICE_TYPE_SPEAKER;
    DeviceBlockStatus status = DEVICE_BLOCKED;

    eventEntry->OnMicrophoneBlockedUpdate(devType, status);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_019
 * @tc.desc  : Test AudioCoreService::EventEntry::OnPnpDeviceStatusUpdated()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_019, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_SPEAKER;
    bool isConnected = true;

    eventEntry->OnPnpDeviceStatusUpdated(desc, isConnected);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_020
 * @tc.desc  : Test AudioCoreService::EventEntry::OnServiceConnected()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_020, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioServiceIndex serviceIndex = HDI_SERVICE_INDEX;

    eventEntry->OnServiceConnected(serviceIndex);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_021
 * @tc.desc  : Test AudioCoreService::EventEntry::OnForcedDeviceSelected()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_021, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DeviceType devType = DEVICE_TYPE_SPEAKER;
    std::string macAddress = "macAddress";
    eventEntry->OnPrivacyDeviceSelected(devType, macAddress);
    eventEntry->OnForcedDeviceSelected(devType, macAddress);

    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
    auto devDesc = make_shared<AudioDeviceDescriptor>();
    devDesc->deviceId_ = 114914;
    devDesc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    devDesc->macAddress_ = macAddress;
    devDesc->deviceRole_ = OUTPUT_DEVICE;
    devMan.AddNewDevice(devDesc);
    auto devDesc2 = make_shared<AudioDeviceDescriptor>();
    devDesc2->deviceId_ = 114915;
    devDesc2->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    devDesc2->macAddress_ = macAddress;
    devDesc2->deviceRole_ = INPUT_DEVICE;
    devMan.AddNewDevice(devDesc2);
    AudioDeviceStatus::GetInstance().OnPrivacyDeviceSelected(devType, macAddress);
    EXPECT_NE(devMan.FindConnectedDeviceById(devDesc->deviceId_), nullptr);
}

class MockRouter : public RouterBase {
public:
    RouterType routerType_ = ROUTER_TYPE_NONE;
    std::shared_ptr<AudioDeviceDescriptor> mediaRenderRet_;
    std::shared_ptr<AudioDeviceDescriptor> callRenderRet_;
    std::shared_ptr<AudioDeviceDescriptor> callCaptureRet_;
    std::shared_ptr<AudioDeviceDescriptor> recordCaptureRet_;
    MockRouter() = default;
    MockRouter(RouterType type = ROUTER_TYPE_DEFAULT,
        std::shared_ptr<AudioDeviceDescriptor> mediaRenderRet = nullptr,
        std::shared_ptr<AudioDeviceDescriptor> callRenderRet = nullptr,
        std::shared_ptr<AudioDeviceDescriptor> callCaptureRet = nullptr,
        std::shared_ptr<AudioDeviceDescriptor> recordCaptureRet = nullptr)
        : routerType_(type),
          mediaRenderRet_(std::move(mediaRenderRet)),
          callRenderRet_(std::move(callRenderRet)),
          callCaptureRet_(std::move(callCaptureRet)),
          recordCaptureRet_(std::move(recordCaptureRet)) {}

    std::shared_ptr<AudioDeviceDescriptor> GetMediaRenderDevice(StreamUsage, int32_t, const uint32_t) override
    {
        return mediaRenderRet_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetCallRenderDevice(StreamUsage, int32_t, const uint32_t) override
    {
        return callRenderRet_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType, int32_t, const uint32_t) override
    {
        return callCaptureRet_;
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage, int32_t) override
    {
        static const std::vector<std::shared_ptr<AudioDeviceDescriptor>> emptyVector;
        return emptyVector;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType, int32_t, const uint32_t) override
    {
        return recordCaptureRet_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetToneRenderDevice(StreamUsage, int32_t) override
    {
        return std::shared_ptr<AudioDeviceDescriptor>();
    }

    RouterType GetRouterType() override
    {
        return routerType_;
    }
};

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_041
 * @tc.desc  : Test AudioCoreService::EventEntry::OnForcedDeviceSelected()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_041, TestSize.Level1)
{
    DeviceType devType = DEVICE_TYPE_USB_HEADSET;
    std::string macAddress = "macAddress";
    auto &audioRouter = AudioRouterCenter::GetAudioRouterCenter();
    audioRouter.renderConfigMap_[STREAM_USAGE_VOICE_COMMUNICATION] == "CallRenderRouters";
    AudioDeviceStatus &audioDeviceStatus =  AudioDeviceStatus::GetInstance();
    auto devDesc = make_shared<AudioDeviceDescriptor>();
    devDesc->deviceId_ = 114914;
    devDesc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    devDesc->macAddress_ = macAddress;
    devDesc->deviceRole_ = OUTPUT_DEVICE;
    audioRouter.callRenderRouters_.emplace_back(std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT,
        nullptr, devDesc, nullptr, nullptr));
    audioDeviceStatus.audioDeviceManager_.commRenderPrivacyDevices_.push_back(devDesc);
    audioDeviceStatus.OnPrivacyDeviceSelected(devType, macAddress);
    audioDeviceStatus.audioDeviceManager_.commRenderPrivacyDevices_.pop_back();
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_022
 * @tc.desc  : Test AudioCoreService::EventEntry::SetDefaultOutputDevice()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_022, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    uint32_t sessionID = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    bool isRunning = true;

    auto ret = eventEntry->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_023
 * @tc.desc  : Test AudioCoreService::EventEntry::SetDeviceActive()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_023, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    InternalDeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool active = true;
    int32_t pid = 0;

    auto ret = eventEntry->SetDeviceActive(deviceType, active, pid);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_024
 * @tc.desc  : Test AudioCoreService::EventEntry::SetCallDeviceActive()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_024, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    InternalDeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool active = true;
    std::string address = "address";

    auto ret = eventEntry->SetCallDeviceActive(deviceType, active, address);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_025
 * @tc.desc  : Test AudioCoreService::EventEntry::RegisterTracker()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_025, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioMode mode;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object;
    int32_t apiVersion = 0;

    auto ret = eventEntry->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_026
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateTracker()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_026, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_PAUSE);
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioMode mode;
    AudioStreamChangeInfo streamChangeInfo;

    auto ret = eventEntry->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_028
 * @tc.desc  : Test AudioCoreService::EventEntry::ConnectServiceAdapter()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_028, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    auto ret = eventEntry->ConnectServiceAdapter();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_029
 * @tc.desc  : Test AudioCoreService::EventEntry::SelectOutputDevice()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_029, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioRendererFilter->uid = 1001;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;

    auto deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDescriptor->macAddress_ = "00:11:22:33:44:55";
    deviceDescriptor->isEnable_ = true;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(deviceDescriptor);

    auto ret = eventEntry->SelectOutputDevice(audioRendererFilter, selectedDesc, 1);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_030
 * @tc.desc  : Test AudioCoreService::EventEntry::SelectInputDevice()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_030, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);


    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    audioCapturerFilter->uid = 1001;
    audioCapturerFilter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    audioCapturerFilter->capturerInfo.capturerFlags = 0;


    auto inputDevice = std::make_shared<AudioDeviceDescriptor>();
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->deviceType_ = DEVICE_TYPE_MIC;
    inputDevice->macAddress_ = "AA:BB:CC:DD:EE:FF";
    inputDevice->networkId_ = "local";
    inputDevice->isEnable_ = true;


    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(inputDevice);

    auto ret = eventEntry->SelectInputDevice(audioCapturerFilter, selectedDesc);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_031
 * @tc.desc  : Test AudioCoreService::EventEntry::OnCapturerSessionAdded()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_031, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionInfo sessionInfo;
    AudioStreamInfo streamInfo;
    auto ret = eventEntry->OnCapturerSessionAdded(sessionId, sessionInfo, streamInfo);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_032
 * @tc.desc  : Test AudioCoreService::EventEntry::IsArmUsbDevice()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_032, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioDeviceDescriptor deviceDesc;

    auto ret = eventEntry->IsArmUsbDevice(deviceDesc);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_033
 * @tc.desc  : Test AudioCoreService::EventEntry::GetCurrentRendererChangeInfos()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_033, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;

    auto ret = eventEntry->GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_034
 * @tc.desc  : Test AudioCoreService::EventEntry::GetCurrentCapturerChangeInfos()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_034, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;

    auto ret = eventEntry->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos,
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_035
 * @tc.desc  : Test AudioCoreService::EventEntry::GetExcludedDevices()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_035, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;

    auto ret = eventEntry->GetExcludedDevices(audioDevUsage);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_038
 * @tc.desc  : Test AudioCoreService::EventEntry::SetSessionDefaultOutputDevice
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_038, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DeviceType type = DEVICE_TYPE_DEFAULT;
    auto ret = eventEntry->SetSessionDefaultOutputDevice(0, type);
    EXPECT_TRUE((ret == 0) || (ret == ERR_NOT_SUPPORTED));
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_039
 * @tc.desc  : Test AudioCoreService::EventEntry::GetSessionDefaultOutputDevice
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_039, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    DeviceType type = DEVICE_TYPE_INVALID;
    auto ret = eventEntry->GetSessionDefaultOutputDevice(0, type);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_040
 * @tc.desc  : Test AudioCoreService::SelectA2dpType
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_040, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);

    audioCoreService->SelectA2dpType(audioStreamDescriptor, false);
    std::shared_ptr<AudioDeviceDescriptor> temp = audioStreamDescriptor->newDeviceDescs_.front();
    EXPECT_NE(temp, nullptr);
    EXPECT_NE(temp->a2dpOffloadFlag_, A2DP_OFFLOAD);
}

/**
 * @tc.name  : Test FetchOutputDeviceAndRoute.
 * @tc.number: FetchOutputDeviceAndRoute_041
 * @tc.desc  : Test AudioCoreService::EventEntry::FetchOutputDeviceAndRoute
 */
HWTEST(AudioCoreServiceEntryTest, FetchOutputDeviceAndRoute_041, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    std::string caller;
    AudioStreamDeviceChangeReasonExt reason;

    auto ret =  eventEntry->FetchOutputDeviceAndRoute(caller, reason);
    EXPECT_NE(ret, ERR_UNKNOWN);
}

/**
 * @tc.name  : Test UpdateSessionOperation.
 * @tc.number: UpdateSessionOperation_042
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation
 */
HWTEST(AudioCoreServiceEntryTest, UpdateSessionOperation_042, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_START;
    SessionOperationMsg opMsg = SESSION_OP_MSG_REMOVE_PIPE;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation, opMsg);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreServUpdateSessionOperationice.
 * @tc.number: AudioCoreService_043
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation
 */
HWTEST(AudioCoreServiceEntryTest, UpdateSessionOperation_043, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = CreateEventEntry(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = SessionOperation::SESSION_OPERATION_START;
    SessionOperationMsg opMsg = (SessionOperationMsg)3;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation, opMsg);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateStreamDevicesForStart.
 * @tc.number: UpdateStreamDevicesForStart_001
 * @tc.desc  : Test isPCVolumeEnable_ && !isFirstScreenOn_
 */
HWTEST(AudioCoreServiceEntryTest, UpdateStreamDevicesForStart_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    VolumeUtils::SetPCVolumeEnable(true);
    audioCoreService->isFirstScreenOn_ = false;
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->UpdateStreamDevicesForStart(audioStreamDescriptor, "test",
        AudioRouterCenter::OutputDeviceMap{});
    EXPECT_NE(audioStreamDescriptor->newDeviceDescs_.size(), 0);
}

/**
 * @tc.name  : Test UpdateStreamDevicesForStart.
 * @tc.number: UpdateStreamDevicesForStart_002
 * @tc.desc  : Test isPCVolumeEnable_ && isFirstScreenOn_ and is inject mode
 */
HWTEST(AudioCoreServiceEntryTest, UpdateStreamDevicesForStart_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    VolumeUtils::SetPCVolumeEnable(true);
    audioCoreService->isFirstScreenOn_ = true;
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->rendererTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    audioCoreService->UpdateStreamDevicesForStart(audioStreamDescriptor, "test",
        AudioRouterCenter::OutputDeviceMap{});
    EXPECT_NE(audioStreamDescriptor->newDeviceDescs_.size(), 0);
}

/**
 * @tc.name  : Test UpdateStreamDevicesForStart.
 * @tc.number: UpdateStreamDevicesForStart_003
 * @tc.desc  : Test !isPCVolumeEnable_ && isFirstScreenOn_
 */
HWTEST(AudioCoreServiceEntryTest, UpdateStreamDevicesForStart_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    VolumeUtils::SetPCVolumeEnable(false);
    audioCoreService->isFirstScreenOn_ = true;
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->UpdateStreamDevicesForStart(audioStreamDescriptor, "test",
        AudioRouterCenter::OutputDeviceMap{});
    EXPECT_EQ(audioStreamDescriptor->newDeviceDescs_.size(), 0);
}

/**
 * @tc.name  : Test UpdateStreamDevicesForStart.
 * @tc.number: UpdateStreamDevicesForStart_004
 * @tc.desc  : Test !isPCVolumeEnable_ && !isFirstScreenOn_
 */
HWTEST(AudioCoreServiceEntryTest, UpdateStreamDevicesForStart_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    VolumeUtils::SetPCVolumeEnable(false);
    audioCoreService->isFirstScreenOn_ = false;
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioCoreService->UpdateStreamDevicesForStart(audioStreamDescriptor, "test",
        AudioRouterCenter::OutputDeviceMap{});
    EXPECT_EQ(audioStreamDescriptor->newDeviceDescs_.size(), 0);
}

/**
 * @tc.name  : TriggerVoipPriorityReload_001
 * @tc.number: TriggerVoipPriorityReload_001
 * @tc.desc  : Test EventEntry::TriggerVoipPriorityReload with a started stream.
 */
HWTEST(AudioCoreServiceEntryTest, TriggerVoipPriorityReload_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(false);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    auto eventEntry = CreateEventEntry(audioCoreService);
    ASSERT_NE(eventEntry, nullptr);

    EXPECT_NO_THROW(eventEntry->TriggerVoipPriorityReload(TEST_SESSION_ID, true));
    auto streamDescAfter = pipeManager->GetStreamDescById(TEST_SESSION_ID);
    ASSERT_NE(streamDescAfter, nullptr);
    EXPECT_TRUE(streamDescAfter->voipPriorityLowered_.load());
    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : TriggerVoipPriorityReload_002
 * @tc.number: TriggerVoipPriorityReload_002
 * @tc.desc  : Test EventEntry::TriggerVoipPriorityReload skips reload for a non-running stream.
 */
HWTEST(AudioCoreServiceEntryTest, TriggerVoipPriorityReload_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();
    constexpr uint32_t sentinelSessionId = 100099;
    audioCoreService->SetOpenedNormalSourceSessionId(sentinelSessionId);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->streamStatus_ = STREAM_STATUS_RELEASED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(true);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    auto eventEntry = CreateEventEntry(audioCoreService);
    ASSERT_NE(eventEntry, nullptr);

    EXPECT_NO_THROW(eventEntry->TriggerVoipPriorityReload(TEST_SESSION_ID, false));
    auto streamDescAfter = pipeManager->GetStreamDescById(TEST_SESSION_ID);
    ASSERT_NE(streamDescAfter, nullptr);
    EXPECT_FALSE(streamDescAfter->voipPriorityLowered_.load());
    EXPECT_EQ(audioCoreService->GetOpenedNormalSourceSessionId(), sentinelSessionId);
    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : ReleaseMultiChannelPipe_MultiChannelReopened_001
 * @tc.number: ReleaseMultiChannelPipe_MultiChannelReopened_001
 * @tc.desc  : Test ReleaseMultiChannelPipe returns ERROR when isMultiChannelOpened_ is true (reopened).
 */
HWTEST(AudioCoreServiceEntryTest, ReleaseMultiChannelPipe_MultiChannelReopened_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->isMultiChannelOpened_.store(true);
    auto eventEntry = CreateEventEntry(audioCoreService);
    ASSERT_NE(eventEntry, nullptr);
    int32_t ret = eventEntry->ReleaseMultiChannelPipe(0, 0);
    EXPECT_EQ(ret, ERROR);
    EXPECT_EQ(audioCoreService->isMultiChannelInRelease_.load(), false);
}
} // namespace AudioStandard
} // namespace OHOS
