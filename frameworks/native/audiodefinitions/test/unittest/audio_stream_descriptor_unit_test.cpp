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

#include "audio_stream_descriptor.h"

#include <cinttypes>

#include <gtest/gtest.h>

#include "audio_common_log.h"
#include "audio_utils.h"
#include "audio_definitions_unit_test_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const int32_t MAX_STREAM_DESCRIPTORS_SIZE = 1003;
static const uint32_t TEST_SET_ROUTE = AUDIO_OUTPUT_FLAG_NORMAL;
static std::string testBundleName = "testBundleName";
static int32_t MEDIA_SERVICE_UID = 1013;
static int32_t TEST_SERVICE_UID = 10;

class AudioStreamDescriptorUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp();
    virtual void TearDown();

private:
    std::shared_ptr<AudioStreamDescriptor> testRendererStream_;
    std::shared_ptr<AudioStreamDescriptor> testCapturerStream_;
};

void AudioStreamDescriptorUnitTest::SetUp()
{
    testRendererStream_ = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_PLAYBACK);
    testCapturerStream_ = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_RECORD);
}

void AudioStreamDescriptorUnitTest::TearDown()
{
    testRendererStream_ = nullptr;
    testCapturerStream_ = nullptr;
}

/**
 * @tc.name   : AudioStreamDescriptor_AllSimpleGet_001
 * @tc.number : AllSimpleGet_001
 * @tc.desc   : Test all simple Get() funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, AllSimpleGet_001, TestSize.Level2)
{
    EXPECT_EQ(true, testRendererStream_->IsPlayback());

    EXPECT_EQ(false, testRendererStream_->IsRecording());

    EXPECT_EQ(TEST_RENDERER_SESSION_ID, testRendererStream_->GetSessionId());

    EXPECT_EQ(false, testRendererStream_->IsRunning());

    EXPECT_EQ(AUDIO_STREAM_ACTION_DEFAULT, testRendererStream_->GetAction());
}

/**
 * @tc.name   : AudioStreamDescriptor_AudioFlag_001
 * @tc.number : AudioFlag_001
 * @tc.desc   : Test audio flag funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, AudioFlag_001, TestSize.Level2)
{
    EXPECT_EQ(AUDIO_FLAG_NONE, testRendererStream_->GetAudioFlag());

    testRendererStream_->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_EQ(false, testRendererStream_->IsUseMoveToConcedeType());
    testRendererStream_->SetAudioFlag(AUDIO_OUTPUT_FLAG_LOWPOWER);
    EXPECT_EQ(true, testRendererStream_->IsUseMoveToConcedeType());
    testRendererStream_->SetAudioFlag(AUDIO_OUTPUT_FLAG_MULTICHANNEL);
    EXPECT_EQ(true, testRendererStream_->IsUseMoveToConcedeType());
}

/**
 * @tc.name   : AudioStreamDescriptor_RendererRoute_001
 * @tc.number : RendererRoute_001
 * @tc.desc   : Test all route funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, RendererRoute_001, TestSize.Level2)
{
    EXPECT_EQ(AUDIO_FLAG_NONE, testRendererStream_->GetRoute());

    EXPECT_EQ(AUDIO_FLAG_NONE, testRendererStream_->GetOldRoute());

    EXPECT_EQ(false, testRendererStream_->IsRouteNormal());

    testRendererStream_->SetRoute(TEST_SET_ROUTE);
    EXPECT_EQ(TEST_SET_ROUTE, testRendererStream_->GetRoute());

    testRendererStream_->SetRoute(AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_EQ(true, testRendererStream_->IsRouteNormal());

    testRendererStream_->SetRoute(AUDIO_OUTPUT_FLAG_LOWPOWER);
    EXPECT_EQ(true, testRendererStream_->IsRouteOffload());
    EXPECT_EQ(true, testRendererStream_->IsNoRunningOffload());
    testRendererStream_->SetStatus(STREAM_STATUS_STARTED);
    EXPECT_EQ(false, testRendererStream_->IsNoRunningOffload());

    testRendererStream_->SetOldRoute(TEST_SET_ROUTE);
    EXPECT_EQ(TEST_SET_ROUTE, testRendererStream_->GetOldRoute());

    testRendererStream_->ResetToNormalRoute(false);
    EXPECT_EQ(true, testRendererStream_->IsRouteNormal());

    testRendererStream_->ResetToNormalRoute(true);
    EXPECT_EQ(true, testRendererStream_->IsRouteNormal());
}

/**
 * @tc.name   : AudioStreamDescriptor_CapturerRoute_001
 * @tc.number : CapturerRoute_001
 * @tc.desc   : Test all simple route funcs by capturer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, CapturerRoute_001, TestSize.Level2)
{
    testCapturerStream_->SetRoute(AUDIO_INPUT_FLAG_NORMAL);
    EXPECT_EQ(true, testCapturerStream_->IsRouteNormal());

    testCapturerStream_->SetRoute(AUDIO_INPUT_FLAG_FAST);
    EXPECT_EQ(false, testCapturerStream_->IsRouteNormal());

    testCapturerStream_->ResetToNormalRoute(false);
    EXPECT_EQ(true, testCapturerStream_->IsRouteNormal());

    testCapturerStream_->ResetToNormalRoute(true);
    EXPECT_EQ(true, testCapturerStream_->IsRouteNormal());
}

/**
 * @tc.name   : AudioStreamDescriptor_RendererDevice_001
 * @tc.number : RendererDevice_001
 * @tc.desc   : Test all simple device funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, RendererDevice_001, TestSize.Level2)
{
    EXPECT_EQ(DEVICE_TYPE_NONE, testRendererStream_->GetMainNewDeviceType());

    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, testRendererStream_->GetMainNewDeviceType());

    device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    device->networkId_ = LOCAL_NETWORK_ID;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(device);
    testRendererStream_->UpdateNewDevice(devices);
    EXPECT_EQ(false, testRendererStream_->IsDeviceRemote());

    device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    device->networkId_ = REMOTE_NETWORK_ID;
    devices.clear();
    devices.push_back(device);
    testRendererStream_->UpdateNewDevice(devices);
    EXPECT_EQ(true, testRendererStream_->IsDeviceRemote());
}

/**
 * @tc.name   : AudioStreamDescriptor_RendererDevice_002
 * @tc.number : RendererDevice_002
 * @tc.desc   : Test device funcs error branches by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, RendererDevice_002, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, testRendererStream_->GetMainNewDeviceType());

    testRendererStream_->AddNewDevice(nullptr);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, testRendererStream_->GetMainNewDeviceType());

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(nullptr);
    testRendererStream_->UpdateNewDevice(devices);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, testRendererStream_->GetMainNewDeviceType());
}

/**
 * @tc.name   : AudioStreamDescriptor_Dump_001
 * @tc.number : Dump_001
 * @tc.desc   : Test dump funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, Dump_001, TestSize.Level3)
{
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);

    std::string outDump;
    testRendererStream_->Dump(outDump);
    EXPECT_NE("", outDump);
}

/**
 * @tc.name   : AudioStreamDescriptor_DeviceString_001
 * @tc.number : DeviceString_001
 * @tc.desc   : Test device string funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, DeviceString_001, TestSize.Level3)
{
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);
    std::string devicesTypeStr = testRendererStream_->GetNewDevicesTypeString();
    EXPECT_NE("", devicesTypeStr);

    std::string devicesInfoStr = testRendererStream_->GetNewDevicesInfo();
    EXPECT_NE("", devicesInfoStr);

    std::string deviceInfoStr = testRendererStream_->GetDeviceInfo(device);
    EXPECT_NE("", deviceInfoStr);
}

/**
 * @tc.name   : AudioStreamDescriptor_BundleName_001
 * @tc.number : BundleName_001
 * @tc.desc   : Test bundle name funcs by renderer AudioStreamDescriptor instance
 */
HWTEST_F(AudioStreamDescriptorUnitTest, BundleName_001, TestSize.Level3)
{
    testRendererStream_->SetBunduleName(testBundleName);
    EXPECT_EQ(testBundleName, testRendererStream_->GetBundleName());
}

/**
 * @tc.name   : AudioStreamDescriptor_Marshalling_001
 * @tc.number : Marshalling_001
 * @tc.desc   : Test marshall and unmarshall funcs
 */
HWTEST_F(AudioStreamDescriptorUnitTest, Marshalling_001, TestSize.Level3)
{
    Parcel testParcel;
    testRendererStream_->Marshalling(testParcel);
    AudioStreamDescriptor *outStream = AudioStreamDescriptor::Unmarshalling(testParcel);
    EXPECT_EQ(testRendererStream_->IsPlayback(), outStream->IsPlayback());
    EXPECT_EQ(testRendererStream_->GetSessionId(), outStream->GetSessionId());
    EXPECT_EQ(testRendererStream_->GetAction(), outStream->GetAction());
    EXPECT_EQ(testRendererStream_->GetRoute(), outStream->GetRoute());
    delete outStream;
}

/**
 * @tc.name   : Test WriteDeviceDescVectorToParcel
 * @tc.number : WriteDeviceDescVectorToParcel_001
 * @tc.desc   : Test WriteDeviceDescVectorToParcel
 */
HWTEST_F(AudioStreamDescriptorUnitTest, WriteDeviceDescVectorToParcel_001, TestSize.Level1)
{
    AudioStreamDescriptor audioStreamDescriptor;
    Parcel parcel;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs(MAX_STREAM_DESCRIPTORS_SIZE,
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    EXPECT_TRUE(audioStreamDescriptor.WriteDeviceDescVectorToParcel(parcel, descs));
}


/**
 * @tc.name   : Test IsSamePidUid
 * @tc.number : IsSamePidUid_001
 * @tc.desc   : Test IsSamePidUid
 */
HWTEST_F(AudioStreamDescriptorUnitTest, IsSamePidUid_001, TestSize.Level1)
{
    AudioStreamDescriptor audioStreamDescriptor;
    audioStreamDescriptor.callerUid_ = 1;
    audioStreamDescriptor.callerPid_ = 1;

    bool ret = audioStreamDescriptor.IsSamePidUid(0, 0);
    EXPECT_EQ(ret, false);

    ret = audioStreamDescriptor.IsSamePidUid(0, 1);
    EXPECT_EQ(ret, false);

    ret = audioStreamDescriptor.IsSamePidUid(1, 0);
    EXPECT_EQ(ret, false);

    ret = audioStreamDescriptor.IsSamePidUid(1, 1);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name   : Test GetRealUid
 * @tc.number : GetRealUid_001
 * @tc.desc   : Test GetRealUid
 */
HWTEST_F(AudioStreamDescriptorUnitTest, GetRealUid_001, TestSize.Level4)
{
    AudioStreamDescriptor audioStreamDescriptor;
    audioStreamDescriptor.callerUid_ = MEDIA_SERVICE_UID;
    audioStreamDescriptor.appInfo_.appUid = TEST_SERVICE_UID;

    auto ret = audioStreamDescriptor.GetRealUid();
    EXPECT_EQ(ret, TEST_SERVICE_UID);

    audioStreamDescriptor.callerUid_ = TEST_SERVICE_UID;
    ret = audioStreamDescriptor.GetRealUid();
    EXPECT_EQ(ret, TEST_SERVICE_UID);
}

/**
 * @tc.name   : AudioStreamDescriptor_IsMediaScene_001
 * @tc.number : IsMediaScene_001
 * @tc.desc   : Test IsMediaScene() with various streamUsage values
 */
HWTEST_F(AudioStreamDescriptorUnitTest, IsMediaScene_001, TestSize.Level2)
{
    testRendererStream_->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    EXPECT_TRUE(testRendererStream_->IsMediaScene());

    testRendererStream_->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;
    EXPECT_TRUE(testRendererStream_->IsMediaScene());

    testRendererStream_->rendererInfo_.streamUsage = STREAM_USAGE_GAME;
    EXPECT_TRUE(testRendererStream_->IsMediaScene());

    testRendererStream_->rendererInfo_.streamUsage = STREAM_USAGE_AUDIOBOOK;
    EXPECT_TRUE(testRendererStream_->IsMediaScene());

    testRendererStream_->rendererInfo_.streamUsage = STREAM_USAGE_NOTIFICATION;
    EXPECT_FALSE(testRendererStream_->IsMediaScene());

    testRendererStream_->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;
    EXPECT_FALSE(testRendererStream_->IsMediaScene());
}

} // namespace AudioStandard
} // namespace OHOS