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

#include "audio_safe_volume_notification_impl_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSafeVolumeNotificationImplUnitTest::SetUpTestCase(void) {}
void AudioSafeVolumeNotificationImplUnitTest::TearDownTestCase(void) {}
void AudioSafeVolumeNotificationImplUnitTest::SetUp(void) {}
void AudioSafeVolumeNotificationImplUnitTest::TearDown(void) {}

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_001
 * @tc.desc  : Test AudioHapticManagerImpl SetTitleAndText interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_001, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    bool ret = impl.SetTitleAndText(RESTORE_VOLUME_NOTIFICATION_ID, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_002
 * @tc.desc  : Test AudioHapticManagerImpl SetTitleAndText interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_002, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    auto normal = std::make_shared<Notification::NotificationNormalContent>();
    bool ret = impl.SetTitleAndText(RESTORE_VOLUME_NOTIFICATION_ID, normal);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_003
 * @tc.desc  : Test AudioHapticManagerImpl SetTitleAndText interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_003, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    auto normal = std::make_shared<Notification::NotificationNormalContent>();
    bool ret = impl.SetTitleAndText(INCREASE_VOLUME_NOTIFICATION_ID, normal);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_004
 * @tc.desc  : Test AudioHapticManagerImpl SetTitleAndText interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_004, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    auto normal = std::make_shared<Notification::NotificationNormalContent>();

    constexpr int32_t INVALID_ID = -1;
    bool ret = impl.SetTitleAndText(INVALID_ID, normal);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_005
 * @tc.desc  : Test AudioHapticManagerImpl GetButtonName interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_005, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    std::string result = impl.GetButtonName(RESTORE_VOLUME_NOTIFICATION_ID);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_006
 * @tc.desc  : Test AudioHapticManagerImpl GetButtonName interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_006, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    std::string result = impl.GetButtonName(INCREASE_VOLUME_NOTIFICATION_ID);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_007
 * @tc.desc  : Test AudioHapticManagerImpl GetButtonName interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_007, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    constexpr uint32_t INVALID_ID = -1;
    std::string result = impl.GetButtonName(INVALID_ID);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_008
 * @tc.desc  : Test AudioHapticManagerImpl GetPixelMap interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_008, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::shared_ptr<Media::PixelMap>(reinterpret_cast<Media::PixelMap*>(0x1), [](auto) {});
    bool ret = impl.GetPixelMap();
    EXPECT_FALSE(ret);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_009
 * @tc.desc  : Test AudioHapticManagerImpl GetPixelMap interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_009, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_.reset();
    bool ret = impl.GetPixelMap();
    EXPECT_TRUE(ret);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_010
 * @tc.desc  : Test AudioHapticManagerImpl PublishSafeVolumeNotification interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_010, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    auto pixelMap = std::make_shared<Media::PixelMap>();
    impl.iconPixelMap_ = pixelMap;
    constexpr int32_t INVALID_ID = -1;
    impl.PublishSafeVolumeNotification(INVALID_ID);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

/**
 * @tc.name  : Test AudioSafeVolumeNotificationImpl.
 * @tc.number: AudioSafeVolumeNotificationImpl_011
 * @tc.desc  : Test AudioHapticManagerImpl PublishSafeVolumeNotification interface.
 */
HWTEST(AudioSafeVolumeNotificationImplUnitTest, AudioSafeVolumeNotificationImpl_011, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    auto pixelMap = std::make_shared<Media::PixelMap>();
    impl.iconPixelMap_ = pixelMap;
    impl.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
