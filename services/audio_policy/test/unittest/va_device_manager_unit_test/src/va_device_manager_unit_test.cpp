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

#include "va_device_manager_unit_test.h"
#include <list>
#include <set>

 using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void VADeviceManagerUnitTest::SetUpTestCase(void) {}
void VADeviceManagerUnitTest::TearDownTestCase(void) {}
void VADeviceManagerUnitTest::SetUp(void) {}
void VADeviceManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_001
 * @tc.desc  : Test GetInstance.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_001, TestSize.Level1) {
    VADeviceManager &instance1 = VADeviceManager::GetInstance();
    VADeviceManager &instance2 = VADeviceManager::GetInstance();

    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_002
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_002, TestSize.Level1) {
    std::shared_ptr<VADevice> vaDevice = nullptr;
    VADeviceManager vaDeviceManager;
    std::shared_ptr<AudioDeviceDescriptor> desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    EXPECT_EQ(desc, nullptr);
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_003
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_003, TestSize.Level1) {
    VADeviceManager vaDeviceManager;
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "TestDevice";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_BT_SPP;
    vaDevice->configuration_.role_ = VA_DEVICE_ROLE_IN;
    vaDevice->configuration_.address_ = "00:11:22:33:44:55";
    
    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceName_, "TestDevice");
    EXPECT_EQ(desc->displayName_, "TestDevice");
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_BT_SPP);
    EXPECT_EQ(desc->deviceRole_, INPUT_DEVICE);
    EXPECT_EQ(desc->macAddress_, "00:11:22:33:44:55");
    EXPECT_EQ(desc->networkId_, "00:11:22:33:44:55");
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_004
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_004, TestSize.Level1) {
    VADeviceManager vaDeviceManager;
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "TestDevice";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_BT_SPP;
    vaDevice->configuration_.role_ = VA_DEVICE_ROLE_OUT;
    vaDevice->configuration_.address_ = "00:11:22:33:44:55";
    
    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceName_, "TestDevice");
    EXPECT_EQ(desc->displayName_, "TestDevice");
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_BT_SPP);
    EXPECT_EQ(desc->deviceRole_, OUTPUT_DEVICE);
    EXPECT_EQ(desc->macAddress_, "00:11:22:33:44:55");
    EXPECT_EQ(desc->networkId_, "00:11:22:33:44:55");
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_005
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_005, TestSize.Level1) {
    VADeviceManager vaDeviceManager;
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "TestDevice";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_NONE;
    vaDevice->configuration_.role_ = VA_DEVICE_ROLE_IN;
    vaDevice->configuration_.address_ = "00:11:22:33:44:55";
    
    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceName_, "TestDevice");
    EXPECT_EQ(desc->displayName_, "TestDevice");
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(desc->deviceRole_, INPUT_DEVICE);
    EXPECT_EQ(desc->macAddress_, "00:11:22:33:44:55");
    EXPECT_EQ(desc->networkId_, "00:11:22:33:44:55");
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_006
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_006, TestSize.Level1) {
    VADeviceManager vaDeviceManager;
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "TestDevice";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_NONE;
    vaDevice->configuration_.role_ = VA_DEVICE_ROLE_OUT;
    vaDevice->configuration_.address_ = "00:11:22:33:44:55";
    
    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceName_, "TestDevice");
    EXPECT_EQ(desc->displayName_, "TestDevice");
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(desc->deviceRole_, OUTPUT_DEVICE);
    EXPECT_EQ(desc->macAddress_, "00:11:22:33:44:55");
    EXPECT_EQ(desc->networkId_, "00:11:22:33:44:55");
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_007
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_007, TestSize.Level1) {
    VADeviceManager vaDeviceManager;
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "TestDevice";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_NONE;
    vaDevice->configuration_.address_ = "00:11:22:33:44:55";
    
    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceName_, "TestDevice");
    EXPECT_EQ(desc->displayName_, "TestDevice");
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(desc->deviceRole_, DEVICE_ROLE_NONE);
    EXPECT_EQ(desc->macAddress_, "00:11:22:33:44:55");
    EXPECT_EQ(desc->networkId_, "00:11:22:33:44:55");
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerUnitTest_008
 * @tc.desc  : Test ConvertVADeviceToDescriptor.
 */
HWTEST_F(VADeviceManagerUnitTest, VADeviceManagerUnitTest_008, TestSize.Level1) {
    VADeviceManager vaDeviceManager;
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "TestDevice";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_BT_SPP;
    vaDevice->configuration_.role_ = VA_DEVICE_ROLE_IN;
    vaDevice->configuration_.address_ = "00:11:22:33:44:55";

    VAAudioStreamProperty streamProp;
    streamProp.encoding_ = ENCODING_PCM;
    streamProp.sampleFormat_ = INVALID_WIDTH;
    streamProp.channelLayout_ = CH_LAYOUT_2POINT1;
    streamProp.sampleRate_ = 44100;
    streamProp.samplesPerCycle_ = 1024;
    vaDevice->configuration_.properties_.push_back(streamProp);

    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    auto streamInfo = desc->audioStreamInfo_.front();
    ASSERT_NE(desc, nullptr);
    ASSERT_EQ(desc->audioStreamInfo_.size(), 1);
    EXPECT_EQ(streamInfo.encoding, ENCODING_PCM);
    EXPECT_EQ(streamInfo.format, INVALID_WIDTH);
    EXPECT_EQ(*streamInfo.channelLayout.begin(), CH_LAYOUT_2POINT1);
    EXPECT_EQ(*streamInfo.samplingRate.begin(), 44100);
}

/**
 * @tc.name  : ConvertVADeviceToDescriptor_009
 * @tc.desc  : Test ConvertVADeviceToDescriptor with nullptr VA device
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, ConvertVADeviceToDescriptor_009, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(nullptr);
    EXPECT_EQ(desc, nullptr);
}

/**
 * @tc.name  : ConvertVADeviceToDescriptor_010
 * @tc.desc  : Test ConvertVADeviceToDescriptor with NONE type and NONE role
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, ConvertVADeviceToDescriptor_010, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "test_none";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_NONE;
    vaDevice->configuration_.role_ = static_cast<VADeviceRole>(0);
    vaDevice->configuration_.address_ = "test_addr_none";
    vaDevice->configuration_.isUnidirectional_ = false;

    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(desc->deviceRole_, DEVICE_ROLE_NONE);
}

/**
 * @tc.name  : ConvertVADeviceToDescriptor_011
 * @tc.desc  : Test ConvertVADeviceToDescriptor with OUTPUT role
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, ConvertVADeviceToDescriptor_011, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "test_out";
    vaDevice->configuration_.type_ = VA_DEVICE_TYPE_BT_SPP;
    vaDevice->configuration_.role_ = VA_DEVICE_ROLE_OUT;
    vaDevice->configuration_.address_ = "test_addr_out";
    vaDevice->configuration_.isUnidirectional_ = true;

    auto desc = vaDeviceManager.ConvertVADeviceToDescriptor(vaDevice);
    ASSERT_NE(desc, nullptr);
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_BT_SPP);
    EXPECT_EQ(desc->deviceRole_, OUTPUT_DEVICE);
    EXPECT_TRUE(desc->isUnidirectional_);
}

/**
 * @tc.name  : CalculateBufferSize_001
 * @tc.desc  : Test CalculateBufferSize with valid inputs
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, CalculateBufferSize_001, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    VAAudioStreamProperty streamProp;
    streamProp.sampleFormat_ = SAMPLE_S16LE;
    streamProp.channelLayout_ = CH_LAYOUT_STEREO;
    streamProp.sampleRate_ = 48000;
    streamProp.samplesPerCycle_ = 960;

    uint32_t bufferSize = vaDeviceManager.CalculateBufferSize(streamProp);
    EXPECT_GT(bufferSize, 0u);
}

/**
 * @tc.name  : CalculateBufferSize_002
 * @tc.desc  : Test CalculateBufferSize with zero samplesPerCycle (calculated from sampleRate)
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, CalculateBufferSize_002, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    VAAudioStreamProperty streamProp;
    streamProp.sampleFormat_ = SAMPLE_S16LE;
    streamProp.channelLayout_ = CH_LAYOUT_STEREO;
    streamProp.sampleRate_ = 48000;
    streamProp.samplesPerCycle_ = 0;

    uint32_t bufferSize = vaDeviceManager.CalculateBufferSize(streamProp);
    EXPECT_GT(bufferSize, 0u);
}

/**
 * @tc.name  : CalculateBufferSize_003
 * @tc.desc  : Test CalculateBufferSize with invalid samplesPerCycle (too large)
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, CalculateBufferSize_003, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    VAAudioStreamProperty streamProp;
    streamProp.sampleFormat_ = SAMPLE_S16LE;
    streamProp.channelLayout_ = CH_LAYOUT_STEREO;
    streamProp.sampleRate_ = 48000;
    streamProp.samplesPerCycle_ = 999999;

    uint32_t bufferSize = vaDeviceManager.CalculateBufferSize(streamProp);
    EXPECT_EQ(bufferSize, 0u);
}

/**
 * @tc.name  : CalculateBufferSize_004
 * @tc.desc  : Test CalculateBufferSize with unknown channel layout
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, CalculateBufferSize_004, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    VAAudioStreamProperty streamProp;
    streamProp.sampleFormat_ = SAMPLE_S16LE;
    streamProp.channelLayout_ = CH_LAYOUT_UNKNOWN;
    streamProp.sampleRate_ = 48000;
    streamProp.samplesPerCycle_ = 960;

    uint32_t bufferSize = vaDeviceManager.CalculateBufferSize(streamProp);
    EXPECT_EQ(bufferSize, 0u);
}

/**
 * @tc.name  : OnDevicesConnected_001
 * @tc.desc  : Test OnDevicesConnected with nullptr VA device
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, OnDevicesConnected_001, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    sptr<IVADeviceController> controller = nullptr;
    vaDeviceManager.OnDevicesConnected(nullptr, controller);
    EXPECT_FALSE(vaDeviceManager.connectedVADeviceMap_["1"]);
}

/**
 * @tc.name  : OnDevicesDisconnected_001
 * @tc.desc  : Test OnDevicesDisconnected with nullptr VA device
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, OnDevicesDisconnected_001, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    vaDeviceManager.OnDevicesDisconnected(nullptr);
    EXPECT_FALSE(vaDeviceManager.connectedVADeviceMap_["1"]);
}

/**
 * @tc.name  : SetNoiseReductionMode_001
 * @tc.desc  : Test SetNoiseReductionMode with nullptr VA device
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, SetNoiseReductionMode_001, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    vaDeviceManager.SetNoiseReductionMode(nullptr, 0);
    EXPECT_FALSE(vaDeviceManager.connectedVADeviceMap_["1"]);
}

/**
 * @tc.name  : UpdateNoiseReductionCapability_001
 * @tc.desc  : Test UpdateNoiseReductionCapability with nullptr VA device
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, UpdateNoiseReductionCapability_001, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    vaDeviceManager.UpdateNoiseReductionCapability(nullptr, desc);
    EXPECT_TRUE(desc);
}

/**
 * @tc.name  : UpdateNoiseReductionCapability_002
 * @tc.desc  : Test UpdateNoiseReductionCapability with nullptr descriptor
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, UpdateNoiseReductionCapability_002, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "test_nr";
    vaDeviceManager.UpdateNoiseReductionCapability(vaDevice, nullptr);
    EXPECT_TRUE(vaDevice);
}

/**
 * @tc.name  : UpdateNoiseReductionCapability_003
 * @tc.desc  : Test UpdateNoiseReductionCapability with empty supported modes
 * @tc.type  : FUNC
 */
HWTEST_F(VADeviceManagerUnitTest, UpdateNoiseReductionCapability_003, TestSize.Level1)
{
    auto &vaDeviceManager = VADeviceManager::GetInstance();
    auto vaDevice = std::make_shared<VADevice>();
    vaDevice->configuration_.name_ = "test_nr_empty";
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    vaDeviceManager.UpdateNoiseReductionCapability(vaDevice, desc);
    EXPECT_TRUE(desc);
}

} // namespace AudioStandard
} // namespace OHOS
