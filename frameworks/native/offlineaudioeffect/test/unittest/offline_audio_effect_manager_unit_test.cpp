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

#include <iostream>
#include <thread>
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "audio_errors.h"
#include "offline_audio_effect_manager.h"
#include "offline_audio_effect_server_chain.h"
#include "offline_audio_effect_chain_impl.h"
#include "offline_stream_in_client.h"
#include "audio_stream_info.h"
#include "audio_shared_memory.h"
#include "audio_log.h"
#include "v1_0/effect_types.h"
#include "v1_0/ieffect_control.h"
#include "v1_0/ieffect_model.h"
#include "audio_utils.h"

using namespace testing::ext;
using namespace testing;
using namespace std;
namespace OHOS {
namespace AudioStandard {
namespace {
    const std::string INVALID_EFFECT_NAME = "0d000721";

    AudioStreamInfo NORMAL_STREAM_INFO(
        AudioSamplingRate::SAMPLE_RATE_48000, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO, AudioChannelLayout::CH_LAYOUT_STEREO);
}
class IpcOfflineStreamMock : public IIpcOfflineStream {
public:
    MOCK_METHOD(int32_t, CreateOfflineEffectChain, (const std::string &chainName));
    MOCK_METHOD(int32_t, ConfigureOfflineEffectChain, (const AudioStreamInfo& inInfo, const AudioStreamInfo& outInfo));
    MOCK_METHOD(int32_t, SetParamOfflineEffectChain, (const std::vector<uint8_t>& param));
    MOCK_METHOD(int32_t, GetParamOfflineEffectChain, (std::vector<uint8_t>& param));
    MOCK_METHOD(int32_t, PrepareOfflineEffectChain, (shared_ptr<AudioSharedMemory>& inBuffer,
                                                  shared_ptr<AudioSharedMemory>& outBuffer));
    MOCK_METHOD(int32_t, ProcessOfflineEffectChain, (uint32_t inSize, uint32_t outSize));
    MOCK_METHOD(int32_t, ReleaseOfflineEffectChain, ());
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, ());
};
class OfflineAudioEffectManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OfflineAudioEffectManagerUnitTest::SetUpTestCase(void) {}
void OfflineAudioEffectManagerUnitTest::TearDownTestCase(void) {}
void OfflineAudioEffectManagerUnitTest::SetUp(void) {}
void OfflineAudioEffectManagerUnitTest::TearDown(void) {}

class OfflineAudioEffectChainUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    shared_ptr<OfflineAudioEffectChain> chain_ = nullptr;
};

shared_ptr<OfflineAudioEffectManager> g_manager = nullptr;
string g_normalName = "";

void OfflineAudioEffectChainUnitTest::SetUpTestCase(void)
{
    g_manager = make_shared<OfflineAudioEffectManager>();
    vector<string> names;
    names = g_manager->GetOfflineAudioEffectChains();
    if (names.size() > 0) {
        g_normalName = names[names.size() - 1];
    }
}

unique_ptr<OfflineAudioEffectChain> CreateOfflineAudioEffectChainMock(
    const std::string &chainName)
{
    sptr<IpcOfflineStreamMock> mockProxy = new IpcOfflineStreamMock();
    EXPECT_CALL(*mockProxy, PrepareOfflineEffectChain(_, _)).WillOnce(DoAll(
        SetArgReferee<0>(AudioSharedMemory::CreateFromLocal(1, "testEffect")),
        SetArgReferee<1>(AudioSharedMemory::CreateFromLocal(1, "testEffect")),
        Return(0)
    ));
    unique_ptr<OfflineAudioEffectChainImpl> chain = std::make_unique<OfflineAudioEffectChainImpl>();
    chain->chainName_ = chainName;
    chain->offlineStreamInClient_ = make_shared<OfflineStreamInClient>(mockProxy);
    int32_t ret = chain->CreateEffectChain();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "create OfflineEffectChain failed, errcode is %{public}d", ret);
    return chain;
}
  
int32_t EffectProcess(struct IEffectControl *self, const struct AudioEffectBuffer* input,
    struct AudioEffectBuffer* output)
{
    return SUCCESS;
}

int32_t SendCommand(struct IEffectControl *self, uint32_t cmdId, const int8_t* cmdData, uint32_t cmdDataLen,
    int8_t* replyData, uint32_t* replyDataLen)
{
    if (cmdId == AUDIO_EFFECT_COMMAND_GET_PARAM && replyDataLen != nullptr) {
        const char* testData = "test_param";
        uint32_t dataLen = strlen(testData);
        if (*replyDataLen >= dataLen) {
            int32_t ret = memcpy_s(replyData, *replyDataLen, testData, dataLen);
            if (ret == SUCCESS) {
                *replyDataLen = dataLen;
            }
        }
    }
    return SUCCESS;
}

int32_t SendCommandError(struct IEffectControl *self, uint32_t cmdId, const int8_t* cmdData, uint32_t cmdDataLen,
    int8_t* replyData, uint32_t* replyDataLen)
{
    return ERROR;
}

int32_t GetEffectDescriptor(struct IEffectControl *self, struct EffectControllerDescriptor* desc)
{
    return SUCCESS;
}

int32_t EffectReverse(struct IEffectControl *self, const struct AudioEffectBuffer* input,
    struct AudioEffectBuffer* output)
{
    return SUCCESS;
}

int32_t GetVersion(struct IEffectControl *self, uint32_t* majorVer, uint32_t* minorVer)
{
    return SUCCESS;
}

int32_t IsSupplyEffectLibs(struct IEffectModel *self, bool* supply)
{
    return SUCCESS;
}

int32_t GetAllEffectDescriptors(struct IEffectModel *self, struct EffectControllerDescriptor* descs,
    uint32_t* descsLen)
{
    return SUCCESS;
}

int32_t CreateEffectController(struct IEffectModel *self, const struct EffectInfo* info,
    struct IEffectControl** contoller, struct ControllerId* id)
{
    return SUCCESS;
}

int32_t DestroyEffectController(struct IEffectModel *self, const struct ControllerId* id)
{
    return SUCCESS;
}

int32_t GetEffectDescriptor(struct IEffectModel *self, const char* effectId,
    struct EffectControllerDescriptor* desc)
{
    return SUCCESS;
}

int32_t GetVersion(struct IEffectModel *self, uint32_t* majorVer, uint32_t* minorVer)
{
    return SUCCESS;
}

void OfflineAudioEffectChainUnitTest::TearDownTestCase(void)
{
    g_manager = nullptr;
}

void OfflineAudioEffectChainUnitTest::SetUp(void)
{
    chain_ = CreateOfflineAudioEffectChainMock(g_normalName);
}

void OfflineAudioEffectChainUnitTest::TearDown(void)
{
    if (chain_ != nullptr) {
        chain_->Release();
        chain_ = nullptr;
    }
}

class OfflineAudioEffectServerChainUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<OfflineAudioEffectServerChain> serverChain = nullptr;
    IEffectControl *mockControl = nullptr;
};
void OfflineAudioEffectServerChainUnitTest::SetUpTestCase(void) {}
void OfflineAudioEffectServerChainUnitTest::TearDownTestCase(void) {}

void OfflineAudioEffectServerChainUnitTest::SetUp(void)
{
    serverChain = std::make_shared<OfflineAudioEffectServerChain>("test");

    if (!mockControl) {
        mockControl = new IEffectControl();
    }
    mockControl->EffectProcess = EffectProcess;
    mockControl->SendCommand = SendCommand;
    mockControl->GetEffectDescriptor = GetEffectDescriptor;
    mockControl->EffectReverse = EffectReverse;
    mockControl->GetVersion = GetVersion;
}

void OfflineAudioEffectServerChainUnitTest::TearDown(void)
{
    if (mockControl) {
        delete mockControl;
        mockControl = nullptr;
    }
    serverChain.reset();
}
/**
 * @tc.name  : Test GetOfflineAudioEffectChains API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectManager_001
 * @tc.desc  : Test OfflineAudioEffectManager interface.
 */
HWTEST(OfflineAudioEffectManagerUnitTest, OfflineAudioEffectManager_001, TestSize.Level0)
{
    auto manager = make_shared<OfflineAudioEffectManager>();
    EXPECT_NE(nullptr, manager);
    EXPECT_GE(manager->GetOfflineAudioEffectChains().size(), 0);
}

/**
 * @tc.name  : Test CreateOfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectManager_002
 * @tc.desc  : Test OfflineAudioEffectManager interface.
 */
HWTEST(OfflineAudioEffectManagerUnitTest, OfflineAudioEffectManager_002, TestSize.Level0)
{
    auto chain = CreateOfflineAudioEffectChainMock(g_normalName);
    EXPECT_NE(nullptr, chain);
    EXPECT_EQ(SUCCESS, chain->Prepare());
}

/**
 * @tc.name  : Test OfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_001
 * @tc.desc  : Test OfflineAudioEffectChain interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_001, TestSize.Level1)
{
    int32_t ret = chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(SUCCESS, chain_->Prepare());
}

/**
 * @tc.name  : Test OfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_002
 * @tc.desc  : Test OfflineAudioEffectChain interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_002, TestSize.Level1)
{
    EXPECT_EQ(SUCCESS, chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO));
    EXPECT_EQ(SUCCESS, chain_->Prepare());
}

/**
 * @tc.name  : Test OfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_003
 * @tc.desc  : Test OfflineAudioEffectChain interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_003, TestSize.Level1)
{
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    EXPECT_EQ(SUCCESS, chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO));
    EXPECT_EQ(SUCCESS, chain_->Prepare());
    EXPECT_EQ(SUCCESS, chain_->GetEffectBufferSize(inSize, outSize));
    EXPECT_GT(inSize, 0);
    EXPECT_GT(outSize, 0);
    uint8_t *inBuffer = new uint8_t[inSize];
    uint8_t *outBuffer = new uint8_t[outSize];
    for (uint32_t i = 0; i < inSize; i++) {
            inBuffer[i] = 1;
        }
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(nullptr, inSize, outBuffer, outSize));
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(inBuffer, inSize + 1, outBuffer, outSize));
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(inBuffer, inSize, nullptr, outSize));
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(inBuffer, inSize, outBuffer, outSize + 1));
    delete []inBuffer;
    delete []outBuffer;
}

/**
 * @tc.name  : Test OfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_004
 * @tc.desc  : Test OfflineAudioEffectChain interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_004, TestSize.Level1)
{
    std::vector<uint8_t> param(0);
    int32_t ret = chain_->SetParam(param);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(SUCCESS, chain_->Prepare());
}

/**
 * @tc.name  : Test OfflineAudioEffectChain GetParam API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_005
 * @tc.desc  : Test OfflineAudioEffectChain GetParam interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_005, TestSize.Level1)
{
    // Call Prepare first to satisfy the EXPECT_CALL in CreateOfflineAudioEffectChainMock
    int32_t ret = chain_->Prepare();
    EXPECT_EQ(SUCCESS, ret);

    // Now test GetParam
    std::vector<uint8_t> param;
    ret = chain_->GetParam(param);
    EXPECT_EQ(SUCCESS, ret);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, Create_001, TestSize.Level1)
{
    int32_t ret = serverChain->Create();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, SetConfig_001, TestSize.Level1)
{
    AudioStreamInfo inInfo;
    inInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    inInfo.encoding = AudioEncodingType::ENCODING_PCM;
    inInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    inInfo.channels = AudioChannel::MONO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    outInfo.encoding = AudioEncodingType::ENCODING_PCM;
    outInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    outInfo.channels = AudioChannel::MONO;

    int32_t ret = serverChain->SetConfig(inInfo, outInfo);
    EXPECT_EQ(ret, ERROR);
    serverChain->controller_ = mockControl;
    ret = serverChain->SetConfig(inInfo, outInfo);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, SetParam_001, TestSize.Level1)
{
    std::vector<uint8_t> param(0);

    int32_t ret = serverChain->SetParam(param);
    EXPECT_EQ(ret, ERROR);
    serverChain->controller_ = mockControl;
    ret = serverChain->SetParam(param);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OfflineAudioEffectServerChain GetParam API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_001
 * @tc.desc  : Test OfflineAudioEffectServerChain GetParam interface.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_001, TestSize.Level1)
{
    std::vector<uint8_t> param;

    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, ERROR);

    serverChain->controller_ = mockControl;
    ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_GT(param.size(), 0);
    // 验证返回的数据内容是否正确
    EXPECT_EQ(param.size(), strlen("test_param"));
    EXPECT_EQ(memcmp(param.data(), "test_param", param.size()), 0);
}

/**
 * @tc.name  : Test OfflineAudioEffectServerChain GetParam with error command
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_002
 * @tc.desc  : Test OfflineAudioEffectServerChain GetParam interface with SendCommand error.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_002, TestSize.Level1)
{
    std::vector<uint8_t> param;
    serverChain->controller_ = mockControl;
    mockControl->SendCommand = SendCommandError;
    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, ERROR);
    mockControl->SendCommand = SendCommand;
}

/**
 * @tc.name  : Test OfflineAudioEffectServerChain Process with metadata encoding
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_Process_Metadata_001
 * @tc.desc  : Test OfflineAudioEffectServerChain Process with metadata encoding.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, Process_Metadata_001, TestSize.Level1)
{
    AudioStreamInfo inInfo;
    inInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    inInfo.encoding = AudioEncodingType::ENCODING_METADATA;
    inInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    inInfo.channels = AudioChannel::MONO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    outInfo.encoding = AudioEncodingType::ENCODING_PCM;
    outInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    outInfo.channels = AudioChannel::MONO;

    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->SetConfig(inInfo, outInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(serverChain->savedEncodingType_, AudioEncodingType::ENCODING_METADATA);
}

/**
 * @tc.name  : Test OfflineAudioEffectServerChain Process with PCM encoding
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_Process_PCM_001
 * @tc.desc  : Test OfflineAudioEffectServerChain Process with PCM encoding.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, Process_PCM_001, TestSize.Level1)
{
    AudioStreamInfo inInfo;
    inInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    inInfo.encoding = AudioEncodingType::ENCODING_PCM;
    inInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    inInfo.channels = AudioChannel::MONO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    outInfo.encoding = AudioEncodingType::ENCODING_PCM;
    outInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    outInfo.channels = AudioChannel::MONO;

    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->SetConfig(inInfo, outInfo);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(serverChain->savedEncodingType_, AudioEncodingType::ENCODING_PCM);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetEffectBufferSize_001, TestSize.Level1)
{
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    int32_t ret = serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetEffectBufferSize_002, TestSize.Level1)
{
    serverChain->inBufferSize_= 1;
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    int32_t ret = serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetEffectBufferSize_003, TestSize.Level1)
{
    serverChain->inBufferSize_ = 1;
    serverChain->outBufferSize_ = 1;
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    int32_t ret = serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, Prepare_001, TestSize.Level1)
{
    serverChain->firstProcess_  = true;
    serverChain->Prepare(nullptr, nullptr);
    EXPECT_EQ(serverChain->firstProcess_, true);
    serverChain->controller_ = mockControl;
    mockControl->SendCommand = SendCommandError;
    serverChain->Prepare(nullptr, nullptr);
    EXPECT_EQ(serverChain->firstProcess_, true);
    mockControl->SendCommand = SendCommand;
    int32_t ret = serverChain->Prepare(nullptr, nullptr);
    EXPECT_EQ(serverChain->firstProcess_, false);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, Release_001, TestSize.Level1)
{
    int32_t ret = serverChain->Release();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetParam with param size exceeding PARAM_MAX_SIZE
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_SizeTooLarge
 * @tc.desc  : Test GetParam with param.size() > PARAM_MAX_SIZE.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_SizeTooLarge, TestSize.Level1)
{
    std::vector<uint8_t> param(4097); // PARAM_MAX_SIZE + 1
    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetParam with empty param
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_EmptyParam
 * @tc.desc  : Test GetParam with empty param vector.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_EmptyParam, TestSize.Level1)
{
    std::vector<uint8_t> param;
    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(param.size(), strlen("test_param"));
    EXPECT_EQ(memcmp(param.data(), "test_param", param.size()), 0);
}

/**
 * @tc.name  : Test GetParam with non-empty param
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_NonEmptyParam
 * @tc.desc  : Test GetParam with non-empty param vector to cover if (param.size() > 0) branch.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_NonEmptyParam, TestSize.Level1)
{
    std::vector<uint8_t> param = {'t', 'e', 's', 't'}; // Non-empty param
    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(param.size(), strlen("test_param"));
    EXPECT_EQ(memcmp(param.data(), "test_param", param.size()), 0);
}

/**
 * @tc.name  : Test GetParam with replyLen zero
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_ReplyLenZero
 * @tc.desc  : Test GetParam when SendCommand returns replyLen = 0.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_ReplyLenZero, TestSize.Level1)
{
    std::vector<uint8_t> param;
    serverChain->controller_ = mockControl;

    // Mock SendCommand to return replyLen = 0
    mockControl->SendCommand = [](struct IEffectControl *self, uint32_t cmdId,
                                   const int8_t* cmdData, uint32_t cmdDataLen,
                                   int8_t* replyData, uint32_t* replyDataLen) -> int32_t {
        if (replyDataLen) *replyDataLen = 0;
        return SUCCESS;
    };

    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, ERROR);

    // Restore original SendCommand
    mockControl->SendCommand = SendCommand;
}

/**
 * @tc.name  : Test GetParam verifies returned data
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_GetParam_VerifyData
 * @tc.desc  : Test GetParam verifies returned data content.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetParam_VerifyData, TestSize.Level1)
{
    std::vector<uint8_t> param;
    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->GetParam(param);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(param.size(), strlen("test_param"));
    EXPECT_EQ(memcmp(param.data(), "test_param", param.size()), 0);
}

/**
 * @tc.name  : Test SetParam with param size exceeding PARAM_MAX_SIZE
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_SetParam_SizeTooLarge
 * @tc.desc  : Test SetParam with param.size() >= PARAM_MAX_SIZE.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, SetParam_SizeTooLarge, TestSize.Level1)
{
    std::vector<uint8_t> param(4096); // PARAM_MAX_SIZE
    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->SetParam(param);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetParam with size just under PARAM_MAX_SIZE
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectServerChain_SetParam_SizeBoundary
 * @tc.desc  : Test SetParam with param.size() = PARAM_MAX_SIZE - 1.
 */
HWTEST_F(OfflineAudioEffectServerChainUnitTest, SetParam_SizeBoundary, TestSize.Level1)
{
    std::vector<uint8_t> param(4095); // PARAM_MAX_SIZE - 1
    serverChain->controller_ = mockControl;
    int32_t ret = serverChain->SetParam(param);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
