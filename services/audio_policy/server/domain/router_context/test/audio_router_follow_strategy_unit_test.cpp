#include "audio_router_follow_strategy_unit_test.h"
#include "audio_router_follow_strategy.h"
#include "audio_router_context_test_util.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioRouterFollowStrategyUnitTest::SetUpTestCase(void) {}
void AudioRouterFollowStrategyUnitTest::TearDownTestCase(void) {}
void AudioRouterFollowStrategyUnitTest::SetUp(void) {}
void AudioRouterFollowStrategyUnitTest::TearDown(void) {}

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_001, TestSize.Level1) {
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    auto speaker = AudioRouterContextTestUtil::CreateDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    deviceManager.AddNewDevice(speaker);
    
    auto earpiece = AudioRouterContextTestUtil::CreateDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");
    deviceManager.AddNewDevice(earpiece);
    
    AudioRouterFollowStrategy strategy;
    
    strategy.SetMediaOutputDevice(SYSTEM_UID, 1, earpiece);
    
    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, 1, STREAM_USAGE_MEDIA);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);
    
    auto appResult = strategy.GetMediaOutputDevice(1001, 2, STREAM_USAGE_MEDIA);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_EARPIECE);
    
    strategy.SetMediaOutputDevice(SYSTEM_UID, 1, nullptr);
    
    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, 1, STREAM_USAGE_MEDIA);
    EXPECT_EQ(systemResult, nullptr);
    
    appResult = strategy.GetMediaOutputDevice(1001, 2, STREAM_USAGE_MEDIA);
    EXPECT_EQ(appResult, nullptr);
}

} 
} 
