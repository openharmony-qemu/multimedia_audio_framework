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

#include "audio_device_common_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioDeviceCommonUnitTest::SetUpTestCase(void) {}
void AudioDeviceCommonUnitTest::TearDownTestCase(void) {}
void AudioDeviceCommonUnitTest::SetUp(void) {}
void AudioDeviceCommonUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioDeviceCommon.
* @tc.number: AudioDeviceCommon_001
* @tc.desc  : Test GetHasDpFlag interface.
*/
HWTEST_F(AudioDeviceCommonUnitTest, AudioDeviceCommon_001, TestSize.Level1)
{
    AudioDeviceCommon& audioDeviceCommon = AudioDeviceCommon::GetInstance();
    audioDeviceCommon.audioPolicyServerHandler_ = nullptr;
    AudioDeviceDescriptor deviceDescriptor;
    audioDeviceCommon.OnPreferredOutputDeviceUpdated(deviceDescriptor);

    DeviceType deviceType = DEVICE_TYPE_NONE;
    audioDeviceCommon.OnPreferredInputDeviceUpdated(deviceType, "");

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = static_cast<StreamUsage>(1000);
    audioDeviceCommon.GetPreferredOutputDeviceDescInner(rendererInfo, "");

    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    audioDeviceCommon.GetPreferredInputDeviceDescInner(captureInfo, "");
}
} // namespace AudioStandard
} // namespace OHOS
