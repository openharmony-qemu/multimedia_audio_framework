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
#include <streambuf>
#include <memory>
#include "gtest/gtest.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"
#include "pro_audio_service_adapter_unit_test.h"
using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
static std::string g_rootPath = "/data/";

void ProAudioServiceAdapterUnitTest::SetUpTestCase(void)
{}
void ProAudioServiceAdapterUnitTest::TearDownTestCase(void)
{}
void ProAudioServiceAdapterUnitTest::SetUp(void)
{
    IdHandler::GetInstance();
    HdiAdapterManager::GetInstance();
    std::unique_ptr<ProAudioServiceCallbackTest> cb = std::make_unique<ProAudioServiceCallbackTest>();
    impl_ = AudioServiceAdapter::CreateAudioAdapter(std::move(cb), true);
    impl_->Connect();
    HPAE::IHpaeManager::GetHpaeManager().Init();
}

void ProAudioServiceAdapterUnitTest::TearDown(void)
{
    HPAE::IHpaeManager::GetHpaeManager().DeInit();
}

ProAudioServiceAdapterUnitTest::ProAudioServiceAdapterUnitTest()
{
}

ProAudioServiceAdapterUnitTest::~ProAudioServiceAdapterUnitTest()
{
}

void ProAudioServiceAdapterUnitTest::Init()
{
}

AudioModuleInfo ProAudioServiceAdapterUnitTest::InitSinkAudioModeInfo()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = "Speaker_File";
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

AudioModuleInfo ProAudioServiceAdapterUnitTest::InitSourceAudioModeInfo()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = "mic";
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

/**
 * @tc.name: Pro_Audio_OpenAudioPort_001
 * @tc.desc: test open audio port sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_OpenAudioPort_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
}

/**
 * @tc.name: Pro_Audio_OpenAudioPort_002
 * @tc.desc: test open audio port source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_OpenAudioPort_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
}

/**
 * @tc.name: Pro_Audio_CloseAudioPort_001
 * @tc.desc: test close audio port
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_CloseAudioPort_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->CloseAudioPort(portId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name: Pro_Audio_CloseAudioPort_002
 * @tc.desc: test close audio port source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_CloseAudioPort_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->CloseAudioPort(portId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name: Pro_Audio_SetDefaultSink_001
 * @tc.desc: test set default sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetDefaultSink_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SetDefaultSink(moduleInfo.name);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetDefaultSource_001
 * @tc.desc: test set default source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetDefaultSource_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SetDefaultSource(moduleInfo.name);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSinkMute_001
 * @tc.desc: test set sink mute
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSinkMute_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SetSinkMute(moduleInfo.name, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSinkMute_002
 * @tc.desc: test set sink unmute
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSinkMute_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SetSinkMute(moduleInfo.name, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSourceMute_001
 * @tc.desc: test set source mute
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSourceMute_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SetSourceOutputMute(portId, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSourceMute_002
 * @tc.desc: test set source unmute
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSourceMute_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SetSourceOutputMute(portId, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SuspendedSink_001
 * @tc.desc: test suspended sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SuspendedSink_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SuspendAudioDevice(moduleInfo.name, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SuspendedSink_002
 * @tc.desc: test suspended sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SuspendedSink_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SuspendAudioDevice(moduleInfo.name, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SuspendedSource_001
 * @tc.desc: test suspended source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SuspendedSource_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SuspendAudioDevice(moduleInfo.name, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SuspendedSource_002
 * @tc.desc: test suspended source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SuspendedSource_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    int32_t ret = impl_->SuspendAudioDevice(moduleInfo.name, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_GetAllSinks_001
 * @tc.desc: test get all sinks
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_GetAllSinks_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    std::vector<SinkInput> sinkInputs = impl_->GetAllSinkInputs();
    EXPECT_EQ(0, sinkInputs.size());
}

/**
 * @tc.name: Pro_Audio_GetAllSources_001
 * @tc.desc: test get all sources
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_GetAllSources_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(0, portId);
    std::vector<SourceOutput> sourceOutputs = impl_->GetAllSourceOutputs();
    EXPECT_EQ(0, sourceOutputs.size());
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_001
 * @tc.desc: test reload audio port sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(portId, 0);
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_002
 * @tc.desc: test reload audio port source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_EQ(portId, -1);
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_003
 * @tc.desc: test reload audio port source
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_003, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    moduleInfo.lib = "libmodule-inner-capturer-sink.z.so";
    int32_t portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_EQ(portId, -1);
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_004
 * @tc.desc: test reload audio port sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_004, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(portId, 0);
    portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(portId, 0);
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_005
 * @tc.desc: test reload audio port sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_005, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(portId, 0);
    int32_t ret = impl_->CloseAudioPort(portId);
    EXPECT_EQ(ERROR, ret);
    portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_GE(portId, 0);
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_006
 * @tc.desc: test reload audio port sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_006, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_EQ(portId, -1);
    portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_EQ(portId, -1);
}

/**
 * @tc.name: Pro_Audio_ReloadAudioPort_007
 * @tc.desc: test reload audio port sink
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_ReloadAudioPort_007, TestSize.Level1)
{
    AudioModuleInfo moduleInfo = InitSourceAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_EQ(portId, -1);
    int32_t ret = impl_->CloseAudioPort(portId);
    EXPECT_EQ(ERROR, ret);
    portId = impl_->ReloadAudioPort(moduleInfo.lib, moduleInfo);
    EXPECT_EQ(portId, -1);
}
}  // namespace AudioStandard
}  // namespace OHOS
