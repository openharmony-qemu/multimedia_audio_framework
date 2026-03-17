#include "audio_router_context_test_base.h"
#include "audio_router_follow_strategy.h"
#include "audio_device_manager.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterFollowStrategyUnitTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterFollowStrategyUnitTest, AudioRouterFollowStrategyUnitTest_001, TestSize.Level1) {
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    
    auto speaker = AudioRouterContextTestUtil::AddDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE, "00:00:00:00:00:01");
    auto earpiece = AudioRouterContextTestUtil::AddDeviceDescriptor(DEVICE_TYPE_EARPIECE, OUTPUT_DEVICE, "00:00:00:00:00:02");
 
    AudioRouterFollowStrategy strategy;
    
    strategy.SetMediaOutputDevice(SYSTEM_UID, 1, earpiece);
    
    auto systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, TEST_STREAM_INVALID_ID, STREAM_USAGE_MEDIA);
    ASSERT_NE(systemResult, nullptr);
    EXPECT_EQ(systemResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(systemResult->deviceId_, earpiece->deviceId_);
    
    auto appResult = strategy.GetMediaOutputDevice(TEST_APP_UID_10001, TEST_STREAM_INVALID_ID, STREAM_USAGE_MEDIA);
    ASSERT_NE(appResult, nullptr);
    EXPECT_EQ(appResult->deviceType_, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(appResult->deviceId_, earpiece->deviceId_);
    
    strategy.SetMediaOutputDevice(SYSTEM_UID, 1, nullptr);
    
    systemResult = strategy.GetMediaOutputDevice(SYSTEM_UID, 1, STREAM_USAGE_MEDIA);
    EXPECT_EQ(systemResult, nullptr);
    
    appResult = strategy.GetMediaOutputDevice(1001, 2, STREAM_USAGE_MEDIA);
    EXPECT_EQ(appResult, nullptr);
}

} 
} 
