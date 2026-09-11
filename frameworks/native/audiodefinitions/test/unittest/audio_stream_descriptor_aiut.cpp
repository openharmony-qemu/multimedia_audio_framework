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

#include "aiut_log.h"
#include "audio_stream_descriptor.h"
#include "audio_definitions_unit_test_utils.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace {
constexpr int32_t MAX_STREAM_DESCRIPTORS_SIZE = 1003;
constexpr int32_t TEST_CALLER_UID = 2000;
constexpr int32_t TEST_APP_UID = 3000;
}

namespace OHOS {
namespace AudioStandard {

class AudioStreamDescriptorAiut : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown();

private:
    std::shared_ptr<AudioStreamDescriptor> testRendererStream_;
    std::shared_ptr<AudioStreamDescriptor> testCapturerStream_;
};

void AudioStreamDescriptorAiut::SetUp()
{
    testRendererStream_ = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_PLAYBACK);
    testCapturerStream_ = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_RECORD);
}

void AudioStreamDescriptorAiut::TearDown()
{
    testRendererStream_ = nullptr;
    testCapturerStream_ = nullptr;
}

/**
 * @tc.name  : Test AudioStreamDescriptor copy constructor with null descriptor
 * @tc.number: AudioStreamDescriptor_CopyConstructor_NullDescriptor
 * @tc.desc  : Test copy constructor with null descriptor returns early with default values
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_CopyConstructor_NullDescriptor, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    std::shared_ptr<AudioStreamDescriptor> nullDesc = nullptr;
    AudioStreamDescriptor copiedDesc(nullDesc);
    AIUT_EXPECT_EQ_INFO_LOG(copiedDesc.audioMode_, AUDIO_MODE_PLAYBACK,
        "expected AUDIO_MODE_PLAYBACK, got %d", copiedDesc.audioMode_);
}

/**
 * @tc.name  : Test AudioStreamDescriptor copy constructor with device descriptors
 * @tc.number: AudioStreamDescriptor_CopyConstructor_WithDevices
 * @tc.desc  : Test copy constructor with device descriptors including nullptr entries
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_CopyConstructor_WithDevices, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    testRendererStream_->newDeviceDescs_.push_back(device1);
    testRendererStream_->newDeviceDescs_.push_back(nullptr);
    testRendererStream_->newDeviceDescs_.push_back(device2);
    testRendererStream_->oldDeviceDescs_.push_back(nullptr);
    testRendererStream_->oldDeviceDescs_.push_back(device1);

    AudioStreamDescriptor copiedDesc(testRendererStream_);
    AIUT_EXPECT_EQ_INFO_LOG(copiedDesc.newDeviceDescs_.size(), 2u,
        "expected 2 new devices, got %zu", copiedDesc.newDeviceDescs_.size());
    AIUT_EXPECT_EQ_INFO_LOG(copiedDesc.oldDeviceDescs_.size(), 1u,
        "expected 1 old device, got %zu", copiedDesc.oldDeviceDescs_.size());
}

/**
 * @tc.name  : Test AudioStreamDescriptor CopyToStruct method
 * @tc.number: AudioStreamDescriptor_CopyToStruct
 * @tc.desc  : Test CopyToStruct copies all fields correctly
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_CopyToStruct, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);
    testRendererStream_->SetBundleName("testBundle");
    testRendererStream_->SetStatus(STREAM_STATUS_STARTED);

    AudioStreamDescriptor target;
    testRendererStream_->CopyToStruct(target);
    AIUT_EXPECT_EQ_INFO_LOG(testRendererStream_->GetSessionId(), target.sessionId_,
        "sessionId mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(testRendererStream_->GetAction(), target.streamAction_,
        "streamAction mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(testRendererStream_->GetBundleName(), target.bundleName_,
        "bundleName mismatch");
    AIUT_EXPECT_EQ_INFO_LOG(testRendererStream_->newDeviceDescs_.size(), target.newDeviceDescs_.size(),
        "newDeviceDescs size mismatch");
}

/**
 * @tc.name  : Test AudioStreamDescriptor WriteDeviceDescVectorToParcel with too large vector
 * @tc.number: AudioStreamDescriptor_WriteDeviceDescVectorToParcel_TooLarge
 * @tc.desc  : Test WriteDeviceDescVectorToParcel returns false when size exceeds MAX
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_WriteDeviceDescVectorToParcel_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioStreamDescriptor audioStreamDescriptor;
    Parcel parcel;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs(MAX_STREAM_DESCRIPTORS_SIZE + 1,
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    bool ret = audioStreamDescriptor.WriteDeviceDescVectorToParcel(parcel, descs);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "expected false for too large vector, got %d", ret);
}

/**
 * @tc.name  : Test AudioStreamDescriptor UnmarshallingDeviceDescVector with size -1
 * @tc.number: AudioStreamDescriptor_UnmarshallingDeviceDescVector_NegOne
 * @tc.desc  : Test UnmarshallingDeviceDescVector with size -1 returns empty vector
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_UnmarshallingDeviceDescVector_NegOne, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioStreamDescriptor audioStreamDescriptor;
    Parcel parcel;
    parcel.WriteInt32(-1);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioStreamDescriptor.UnmarshallingDeviceDescVector(parcel, descs);
    AIUT_EXPECT_EQ_INFO_LOG(descs.size(), 0u, "expected 0 devices, got %zu", descs.size());
}

/**
 * @tc.name  : Test AudioStreamDescriptor UnmarshallingDeviceDescVector with too large size
 * @tc.number: AudioStreamDescriptor_UnmarshallingDeviceDescVector_TooLarge
 * @tc.desc  : Test UnmarshallingDeviceDescVector with size > MAX returns empty vector
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_UnmarshallingDeviceDescVector_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioStreamDescriptor audioStreamDescriptor;
    Parcel parcel;
    parcel.WriteInt32(MAX_STREAM_DESCRIPTORS_SIZE + 1);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioStreamDescriptor.UnmarshallingDeviceDescVector(parcel, descs);
    AIUT_EXPECT_EQ_INFO_LOG(descs.size(), 0u, "expected 0 devices, got %zu", descs.size());
}

/**
 * @tc.name  : Test AudioStreamDescriptor GetNewDevicesInfo with nullptr entries
 * @tc.number: AudioStreamDescriptor_GetNewDevicesInfo_NullDesc
 * @tc.desc  : Test GetNewDevicesInfo with nullptr entries in device list
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_GetNewDevicesInfo_NullDesc, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);
    testRendererStream_->newDeviceDescs_.push_back(nullptr);
    std::string info = testRendererStream_->GetNewDevicesInfo();
    AIUT_EXPECT_FALSE_INFO_LOG(info.empty(), "expected non-empty info");
}

/**
 * @tc.name  : Test AudioStreamDescriptor GetDeviceInfo with nullptr descriptor
 * @tc.number: AudioStreamDescriptor_GetDeviceInfo_NullDesc
 * @tc.desc  : Test GetDeviceInfo with nullptr descriptor returns empty string
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_GetDeviceInfo_NullDesc, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    std::shared_ptr<AudioDeviceDescriptor> nullDesc = nullptr;
    std::string info = testRendererStream_->GetDeviceInfo(nullDesc);
    AIUT_EXPECT_TRUE_INFO_LOG(info.empty(), "expected empty info for null desc, got size=%zu", info.size());
}

/**
 * @tc.name  : Test AudioStreamDescriptor GetNewDupDevicesTypeString with devices
 * @tc.number: AudioStreamDescriptor_GetNewDupDevicesTypeString
 * @tc.desc  : Test GetNewDupDevicesTypeString with devices returns non-empty string
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_GetNewDupDevicesTypeString, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->newDupDeviceDescs_.push_back(device);
    std::string result = testRendererStream_->GetNewDupDevicesTypeString();
    AIUT_EXPECT_FALSE_INFO_LOG(result.empty(), "expected non-empty result");
}

/**
 * @tc.name  : Test AudioStreamDescriptor GetNewDupDevicesTypeString with nullptr entries
 * @tc.number: AudioStreamDescriptor_GetNewDupDevicesTypeString_NullDesc
 * @tc.desc  : Test GetNewDupDevicesTypeString with nullptr entries returns empty string
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_GetNewDupDevicesTypeString_NullDesc, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->newDupDeviceDescs_.push_back(nullptr);
    std::string result = testRendererStream_->GetNewDupDevicesTypeString();
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(), "expected empty result for null desc, got size=%zu", result.size());
}

/**
 * @tc.name  : Test AudioStreamDescriptor IsActive when stream status is STARTED
 * @tc.number: AudioStreamDescriptor_IsActive_Started
 * @tc.desc  : Test IsActive returns true when stream status is STARTED
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_IsActive_Started, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->SetStatus(STREAM_STATUS_STARTED);
    bool result = testRendererStream_->IsActive(1000);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "expected true for STARTED status");
}

/**
 * @tc.name  : Test AudioStreamDescriptor IsActive when stream is stopped recently
 * @tc.number: AudioStreamDescriptor_IsActive_Stopped
 * @tc.desc  : Test IsActive returns true when stream was stopped recently
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_IsActive_Stopped, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->SetStatus(STREAM_STATUS_STOPPED);
    testRendererStream_->SetStopTime();
    bool result = testRendererStream_->IsActive(10000);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "expected true for recently stopped stream");
}

/**
 * @tc.name  : Test AudioStreamDescriptor IsActive when stream was stopped long ago
 * @tc.number: AudioStreamDescriptor_IsActive_Expired
 * @tc.desc  : Test IsActive returns false when stream was stopped long ago
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_IsActive_Expired, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->SetStatus(STREAM_STATUS_STOPPED);
    testRendererStream_->ResetStopTime();
    bool result = testRendererStream_->IsActive(1);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false for expired stream");
}

/**
 * @tc.name  : Test AudioStreamDescriptor SetStopTime and ResetStopTime
 * @tc.number: AudioStreamDescriptor_SetStopTime
 * @tc.desc  : Test SetStopTime and ResetStopTime behavior
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_SetStopTime, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->SetStopTime();
    testRendererStream_->ResetStopTime();
    testRendererStream_->SetStatus(STREAM_STATUS_STOPPED);
    bool result = testRendererStream_->IsActive(1);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "expected false after reset stop time");
}

/**
 * @tc.name  : Test AudioStreamDescriptor Dump for capturer stream
 * @tc.number: AudioStreamDescriptor_DumpCapturer
 * @tc.desc  : Test Dump for capturer stream produces non-empty output with SourceType
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_DumpCapturer, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    testCapturerStream_->AddNewDevice(device);
    std::string outDump;
    testCapturerStream_->Dump(outDump);
    AIUT_EXPECT_FALSE_INFO_LOG(outDump.empty(), "expected non-empty dump");
    AIUT_EXPECT_TRUE_INFO_LOG(outDump.find("SourceType") != std::string::npos,
        "expected SourceType in dump");
}

/**
 * @tc.name  : Test AudioStreamDescriptor Dump with nullptr device descriptors
 * @tc.number: AudioStreamDescriptor_DumpDeviceAttrs_NullDesc
 * @tc.desc  : Test Dump with nullptr device descriptors produces non-empty output
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_DumpDeviceAttrs_NullDesc, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->newDeviceDescs_.push_back(nullptr);
    testRendererStream_->oldDeviceDescs_.push_back(nullptr);
    std::string outDump;
    testRendererStream_->Dump(outDump);
    AIUT_EXPECT_FALSE_INFO_LOG(outDump.empty(), "expected non-empty dump with null descs");
}

/**
 * @tc.name  : Test AudioStreamDescriptor GetNewDevicesTypeString with nullptr entries
 * @tc.number: AudioStreamDescriptor_GetNewDevicesTypeString_NullDesc
 * @tc.desc  : Test GetNewDevicesTypeString with nullptr entries returns empty string
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_GetNewDevicesTypeString_NullDesc, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    testRendererStream_->newDeviceDescs_.push_back(nullptr);
    std::string result = testRendererStream_->GetNewDevicesTypeString();
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(), "expected empty result for null desc");
}

/**
 * @tc.name  : Test AudioStreamDescriptor Marshalling and Unmarshalling with devices
 * @tc.number: AudioStreamDescriptor_Marshalling_WithDevices
 * @tc.desc  : Test Marshalling/Unmarshalling roundtrip with device descriptors
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_Marshalling_WithDevices, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    testRendererStream_->AddNewDevice(device);
    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    testRendererStream_->oldDeviceDescs_.push_back(device2);

    Parcel testParcel;
    bool ret = testRendererStream_->Marshalling(testParcel);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "Marshalling failed");
    AudioStreamDescriptor *outStream = AudioStreamDescriptor::Unmarshalling(testParcel);
    AIUT_ASSERT_NE_INFO_LOG(outStream, nullptr, "Unmarshalling returned null");
    if (outStream != nullptr) {
        AIUT_EXPECT_EQ_INFO_LOG(testRendererStream_->newDeviceDescs_.size(), outStream->newDeviceDescs_.size(),
            "newDeviceDescs size mismatch");
        AIUT_EXPECT_EQ_INFO_LOG(testRendererStream_->oldDeviceDescs_.size(), outStream->oldDeviceDescs_.size(),
            "oldDeviceDescs size mismatch");
        delete outStream;
    }
}

/**
 * @tc.name  : Test AudioStreamDescriptor GetRealUid when callerUid is not MEDIA_SERVICE_UID
 * @tc.number: AudioStreamDescriptor_GetRealUid_NonMediaService
 * @tc.desc  : Test GetRealUid returns callerUid when not MEDIA_SERVICE_UID
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_GetRealUid_NonMediaService, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    AudioStreamDescriptor streamDesc;
    streamDesc.callerUid_ = TEST_CALLER_UID;
    streamDesc.appInfo_.appUid = TEST_APP_UID;
    int32_t result = streamDesc.GetRealUid();
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_CALLER_UID, "expected callerUid=%d, got %d", TEST_CALLER_UID, result);
}

/**
 * @tc.name  : Test AudioStreamDescriptor Dump for capturer stream with devices
 * @tc.number: AudioStreamDescriptor_CapturerDump
 * @tc.desc  : Test Dump for capturer stream with devices produces output with OldDevices and NewDevices
 * @tc.owner : audio_framework_common
 */
HWTEST_F(AudioStreamDescriptorAiut, AudioStreamDescriptor_CapturerDump, TestSize.Level2)
{
    AIUT_INFO_LOG("entry");
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    testCapturerStream_->AddNewDevice(device);
    testCapturerStream_->oldDeviceDescs_.push_back(device);
    std::string outDump;
    testCapturerStream_->Dump(outDump);
    AIUT_EXPECT_FALSE_INFO_LOG(outDump.empty(), "expected non-empty dump");
    AIUT_EXPECT_TRUE_INFO_LOG(outDump.find("OldDevices") != std::string::npos,
        "expected OldDevices in dump");
    AIUT_EXPECT_TRUE_INFO_LOG(outDump.find("NewDevices") != std::string::npos,
        "expected NewDevices in dump");
}

} // namespace AudioStandard
} // namespace OHOS