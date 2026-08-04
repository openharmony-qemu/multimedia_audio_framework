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

#include "audio_safe_volume_notification_impl_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSafeVolumeNotificationImplUnitTest::SetUpTestCase(void) {}
void AudioSafeVolumeNotificationImplUnitTest::TearDownTestCase(void) {}
void AudioSafeVolumeNotificationImplUnitTest::SetUp(void) {}
void AudioSafeVolumeNotificationImplUnitTest::TearDown(void) {}

void AudioLoudVolumeNotificationImplUnitTest::SetUpTestCase(void) {}
void AudioLoudVolumeNotificationImplUnitTest::TearDownTestCase(void) {}
void AudioLoudVolumeNotificationImplUnitTest::SetUp(void) {}
void AudioLoudVolumeNotificationImplUnitTest::TearDown(void) {}

void AudioLegacyHighVolumeNotificationImplUnitTest::SetUpTestCase(void) {}
void AudioLegacyHighVolumeNotificationImplUnitTest::TearDownTestCase(void) {}
void AudioLegacyHighVolumeNotificationImplUnitTest::SetUp(void) {}
void AudioLegacyHighVolumeNotificationImplUnitTest::TearDown(void) {}

// ========== SetActionButton (static) ==========
// Tested indirectly via PublishSafeVolumeNotification and
// PublishLegacyHighVolumeNotification with valid buttonName.

// ========== AudioSafeVolumeNotificationImpl::GetPixelMap ==========

HWTEST_F(AudioSafeVolumeNotificationImplUnitTest, GetPixelMap_001, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::shared_ptr<Media::PixelMap>(
        reinterpret_cast<Media::PixelMap*>(0x1), [](auto) {});
    bool ret = impl.GetPixelMap();
    EXPECT_FALSE(ret);
}

HWTEST_F(AudioSafeVolumeNotificationImplUnitTest, GetPixelMap_002, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_.reset();
    bool ret = impl.GetPixelMap();
    EXPECT_TRUE(ret);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

// ========== AudioSafeVolumeNotificationImpl::PublishSafeVolumeNotification ==========

HWTEST_F(AudioSafeVolumeNotificationImplUnitTest,
    PublishSafeVolumeNotification_001, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::make_shared<Media::PixelMap>();
    impl.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

HWTEST_F(AudioSafeVolumeNotificationImplUnitTest,
    PublishSafeVolumeNotification_002, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::make_shared<Media::PixelMap>();
    impl.PublishSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

HWTEST_F(AudioSafeVolumeNotificationImplUnitTest,
    PublishSafeVolumeNotification_003, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::make_shared<Media::PixelMap>();
    constexpr int32_t INVALID_ID = -1;
    impl.PublishSafeVolumeNotification(INVALID_ID);
}

HWTEST_F(AudioSafeVolumeNotificationImplUnitTest,
    PublishSafeVolumeNotification_004, TestSize.Level4)
{
    AudioSafeVolumeNotificationImpl impl;
    impl.iconPixelMap_.reset();
    impl.PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
}

// ========== AudioLoudVolumeNotificationImpl::GetPixelMap ==========

HWTEST_F(AudioLoudVolumeNotificationImplUnitTest, GetPixelMap_001, TestSize.Level4)
{
    AudioLoudVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::shared_ptr<Media::PixelMap>(
        reinterpret_cast<Media::PixelMap*>(0x1), [](auto) {});
    bool ret = impl.GetPixelMap();
    EXPECT_FALSE(ret);
}

HWTEST_F(AudioLoudVolumeNotificationImplUnitTest, GetPixelMap_002, TestSize.Level4)
{
    AudioLoudVolumeNotificationImpl impl;
    impl.iconPixelMap_.reset();
    bool ret = impl.GetPixelMap();
    EXPECT_TRUE(ret);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

// ========== AudioLoudVolumeNotificationImpl::PublishLoudVolumeNotification ==========

HWTEST_F(AudioLoudVolumeNotificationImplUnitTest,
    PublishLoudVolumeNotification_001, TestSize.Level4)
{
    AudioLoudVolumeNotificationImpl impl;
    impl.iconPixelMap_ = std::make_shared<Media::PixelMap>();
    impl.PublishLoudVolumeNotification(1);
    EXPECT_NE(impl.iconPixelMap_, nullptr);
}

HWTEST_F(AudioLoudVolumeNotificationImplUnitTest,
    PublishLoudVolumeNotification_002, TestSize.Level4)
{
    AudioLoudVolumeNotificationImpl impl;
    impl.iconPixelMap_.reset();
    impl.PublishLoudVolumeNotification(1);
}

// ========== AudioLegacyHighVolumeNotificationImpl::SetTitleAndText ==========

HWTEST_F(AudioLegacyHighVolumeNotificationImplUnitTest,
    SetTitleAndText_001, TestSize.Level4)
{
    AudioLegacyHighVolumeNotificationImpl impl;
    bool ret = impl.SetTitleAndText(nullptr);
    EXPECT_FALSE(ret);
}

HWTEST_F(AudioLegacyHighVolumeNotificationImplUnitTest,
    SetTitleAndText_002, TestSize.Level4)
{
    AudioLegacyHighVolumeNotificationImpl impl;
    auto content = std::make_shared<Notification::NotificationNormalContent>();
    bool ret = impl.SetTitleAndText(content);
    EXPECT_TRUE(ret);
}

// ========== AudioLegacyHighVolumeNotificationImpl::PublishLegacyHighVolumeNotification ==========

HWTEST_F(AudioLegacyHighVolumeNotificationImplUnitTest,
    PublishLegacyHighVolumeNotification_001, TestSize.Level4)
{
    AudioLegacyHighVolumeNotificationImpl impl;
    impl.PublishLegacyHighVolumeNotification();
}

} // namespace AudioStandard
} // namespace OHOS
