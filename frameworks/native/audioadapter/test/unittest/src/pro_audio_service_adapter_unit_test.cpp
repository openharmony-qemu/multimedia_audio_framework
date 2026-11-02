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
 * @tc.name: Pro_Audio_SetSourceOutputStreamMuteByStreamId_001
 * @tc.desc: test set source mute
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSourceOutputStreamMuteByStreamId_001, TestSize.Level1)
{
    int32_t sessionId = 100001;
    int32_t ret = impl_->SetSourceOutputStreamMuteByStreamId(sessionId, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSourceOutputStreamMuteByStreamId_002
 * @tc.desc: test set source unmute
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSourceOutputStreamMuteByStreamId_002, TestSize.Level1)
{
    int32_t sessionId = 100002;
    int32_t ret = impl_->SetSourceOutputStreamMuteByStreamId(sessionId, false);
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
    EXPECT_GE(portId, 0);
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
    EXPECT_GE(portId, 0);
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
    EXPECT_GE(portId, 0);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffect_001
 * @tc.desc: test set system volume to effect without device type
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffect_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.5f;
    int32_t ret = impl_->SetSystemVolumeToEffect(streamType, volume);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffect_002
 * @tc.desc: test set system volume to effect with different stream type
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffect_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_RING;
    float volume = 0.8f;
    int32_t ret = impl_->SetSystemVolumeToEffect(streamType, volume);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_001
 * @tc.desc: test set system volume to effect with device type for speaker
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.6f;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_002
 * @tc.desc: test set system volume to effect with device type for bluetooth A2DP
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.7f;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_003
 * @tc.desc: test set system volume to effect with device type for wired headset
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_003, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.4f;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_004
 * @tc.desc: test set system volume to effect with device type for different stream type
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_004, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_NOTIFICATION;
    float volume = 0.9f;
    DeviceType deviceType = DEVICE_TYPE_EARPIECE;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_005
 * @tc.desc: test set system volume to effect with device type for USB headset
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_005, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_SYSTEM;
    float volume = 0.55f;
    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_006
 * @tc.desc: test set system volume to effect with device type max volume
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_006, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 1.0f;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_SetSystemVolumeToEffectWithDeviceType_007
 * @tc.desc: test set system volume to effect with device type min volume
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_SetSystemVolumeToEffectWithDeviceType_007, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.0f;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = impl_->SetSystemVolumeToEffectWithDeviceType(streamType, volume, deviceType);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_001
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for SPEAKER
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_001,
    TestSize.Level1)
{
    bool ret = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_6POINT0_FRONT, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_002
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for BLUETOOTH_A2DP
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_002,
    TestSize.Level1)
{
    bool ret = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_STEREO, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_003
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for WIRED_HEADSET
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_003,
    TestSize.Level1)
{
    bool ret = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_MONO, DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_004
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for NEARLINK
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_004,
    TestSize.Level1)
{
    bool ret = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_2POINT1, DEVICE_TYPE_NEARLINK);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_005
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for USB_HEADSET
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_005,
    TestSize.Level1)
{
    bool ret = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_5POINT1, DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for multiple channel layouts
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest, Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001,
    TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret1 = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_MONO, deviceType);
    bool ret2 = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_STEREO, deviceType);
    bool ret3 = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_5POINT1, deviceType);
    bool ret4 = impl_->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_7POINT1, deviceType);

    EXPECT_EQ(ret1, false);
    EXPECT_EQ(ret2, false);
    EXPECT_EQ(ret3, false);
    EXPECT_EQ(ret4, false);
}

/**
 * @tc.name: Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_MultipleDevices_001
 * @tc.desc: test IsChannelLayoutSupportedForDspEffect with device type for multiple device types
 * @tc.type: FUNC
 */
HWTEST_F(ProAudioServiceAdapterUnitTest,
    Pro_Audio_IsChannelLayoutSupportedForDspEffectWithDeviceType_MultipleDevices_001, TestSize.Level1)
{
    AudioChannelLayout channelLayout = CH_LAYOUT_STEREO;
    bool ret1 = impl_->IsChannelLayoutSupportedForDspEffect(channelLayout, DEVICE_TYPE_SPEAKER);
    bool ret2 = impl_->IsChannelLayoutSupportedForDspEffect(channelLayout, DEVICE_TYPE_BLUETOOTH_A2DP);
    bool ret3 = impl_->IsChannelLayoutSupportedForDspEffect(channelLayout, DEVICE_TYPE_WIRED_HEADSET);
    bool ret4 = impl_->IsChannelLayoutSupportedForDspEffect(channelLayout, DEVICE_TYPE_USB_HEADSET);

    EXPECT_EQ(ret1, false);
    EXPECT_EQ(ret2, false);
    EXPECT_EQ(ret3, false);
    EXPECT_EQ(ret4, false);
}

}  // namespace AudioStandard
}  // namespace OHOS
