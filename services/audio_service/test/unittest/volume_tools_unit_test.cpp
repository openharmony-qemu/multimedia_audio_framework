/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_info.h"
#include "audio_ring_cache.h"
#include "audio_process_config.h"
#include "linear_pos_time_model.h"
#include "oh_audio_buffer.h"
#include <gtest/gtest.h>
#include "pa_renderer_stream_impl.h"
#include "policy_handler.h"
#include "pa_adapter_manager.h"
#include "audio_capturer_private.h"
#include "audio_system_manager.h"
#include <cmath>
#include "volume_tools.h"
#include "volume_tools_c.h"
#include <vector>
#include <cstdint>
#include <cstring>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
static const size_t MIN_FRAME_SIZE = 1;
static const int32_t INT32_VOLUME_MIN = 0; // 0, min volume
static const uint32_t VOLUME_SHIFT = 16;
static constexpr int32_t INT32_VOLUME_MAX = 1 << VOLUME_SHIFT;
class VolumeToolsUnitTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
    BufferDesc ConstructBufferDesc(uint8_t *buffer, size_t bufLength,
        size_t dataLength, uint8_t *metaBuffer, size_t metaLength);
};

void VolumeToolsUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void VolumeToolsUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test GetInt32Vol.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_001
 * @tc.desc  : Test GetInt32Vol.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_001, TestSize.Level1)
{
    float volFloat = 0.5;
    int32_t ret = VolumeTools::GetInt32Vol(volFloat);
    int32_t expected = static_cast<int32_t>(volFloat * INT32_VOLUME_MAX);
    EXPECT_EQ(ret, expected);
}

/**
 * @tc.name  : Test GetInt32Vol.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_002
 * @tc.desc  : Test GetInt32Vol.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_002, TestSize.Level1)
{
    float volFloat = -1.0f;
    int32_t ret = VolumeTools::GetInt32Vol(volFloat);
    EXPECT_EQ(ret, INT32_VOLUME_MIN);
}

/**
 * @tc.name  : Test GetInt32Vol.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_003
 * @tc.desc  : Test GetInt32Vol.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_003, TestSize.Level1)
{
    float volFloat = 2.0f;
    int32_t ret = VolumeTools::GetInt32Vol(volFloat);
    EXPECT_EQ(ret, INT32_VOLUME_MAX);
}

/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_004
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_004, TestSize.Level1)
{
    AudioChannel channel = MONO;
    int32_t volStart = -1;
    int32_t volEnd = 1;
    ChannelVolumes ret = VolumeTools::GetChannelVolumes(channel, volStart, volEnd);
    EXPECT_NE(ret.volStart[0], volStart);
}
/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_005
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_005, TestSize.Level1)
{
    AudioChannel channel = MONO;
    int32_t volStart = 1;
    int32_t volEnd = -1;
    ChannelVolumes ret = VolumeTools::GetChannelVolumes(channel, volStart, volEnd);
    EXPECT_NE(ret.volStart[0], volStart);
}
/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_006
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_006, TestSize.Level1)
{
    AudioChannel channel = MONO;
    int32_t volStart = 1;
    int32_t volEnd = 1;
    ChannelVolumes ret = VolumeTools::GetChannelVolumes(channel, volStart, volEnd);
    EXPECT_EQ(ret.volStart[0], volStart);
}

/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_007
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_007, TestSize.Level1)
{
    AudioChannel channel = MONO;
    float volStart = -1.0f;
    float volEnd = 1.0f;
    ChannelVolumes ret = VolumeTools::GetChannelVolumes(channel, volStart, volEnd);
    EXPECT_NE(ret.volStart[0], volStart);
}
/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_008
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_008, TestSize.Level1)
{
    AudioChannel channel = MONO;
    float volStart = 1.0f;
    float volEnd = -1.0f;
    ChannelVolumes ret = VolumeTools::GetChannelVolumes(channel, volStart, volEnd);
    EXPECT_NE(ret.volStart[0], volStart);
}
/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_009
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_009, TestSize.Level1)
{
    AudioChannel channel = MONO;
    float volStart = 1.0f;
    float volEnd = 1.0f;
    ChannelVolumes ret = VolumeTools::GetChannelVolumes(channel, volStart, volEnd);
    EXPECT_EQ(ret.volStart[0], 65536);
}

/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_010
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_010, TestSize.Level1)
{
    BufferDesc desc = {};
    ChannelVolumes volMaps;
    AudioSampleFormat format = SAMPLE_S24LE;
    int32_t ret = VolumeTools::Process(desc, format, volMaps);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetChannelVolumes.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_011
 * @tc.desc  : Test GetChannelVolumes.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_011, TestSize.Level1)
{
    uint8_t *buffer = nullptr;
    BufferDesc desc;
    desc.buffer = buffer;
    desc.bufLength = (MIN_FRAME_SIZE - 1) * 3 * STEREO + 1;
    desc.dataLength = (MIN_FRAME_SIZE - 1) * 3 * STEREO + 3;
    desc.metaBuffer = nullptr;
    desc.metaLength = 0;
    ChannelVolumes volMaps;
    AudioSampleFormat format = SAMPLE_S24LE;
    int32_t ret = VolumeTools::Process(desc, format, volMaps);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test Process.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_012
 * @tc.desc  : Test Process.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_012, TestSize.Level1)
{
    uint8_t *buffer = nullptr;
    BufferDesc desc;
    desc.buffer = buffer;
    desc.bufLength = (MIN_FRAME_SIZE - 1) * 3 * STEREO + 3;
    desc.dataLength = (MIN_FRAME_SIZE - 1) * 3 * STEREO + 1;
    desc.metaBuffer = nullptr;
    desc.metaLength = 0;
    ChannelVolumes volMaps;
    AudioSampleFormat format =  SAMPLE_S24LE;
    int32_t ret = VolumeTools::Process(desc, format, volMaps);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test Process.
 * @tc.type  : FUNC
 * @tc.number: PaRenderer_013
 * @tc.desc  : Test Process.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_013, TestSize.Level1)
{
    uint8_t *buffer = new uint8_t[(MIN_FRAME_SIZE) * 3 * STEREO];
    for (size_t i = 0; i < (MIN_FRAME_SIZE) * 3 * STEREO; ++i) {
        buffer[i] = static_cast<uint8_t>(i);
    }
    BufferDesc desc;
    desc.buffer = buffer;
    desc.bufLength = (MIN_FRAME_SIZE - 1) * 3 * STEREO + 6;
    desc.dataLength = (MIN_FRAME_SIZE - 1) * 3 * STEREO + 6;
    desc.metaBuffer = nullptr;
    desc.metaLength = 0;
    ChannelVolumes volMaps;
    AudioSampleFormat format =  SAMPLE_S24LE;
    int32_t ret = VolumeTools::Process(desc, format, volMaps);
    EXPECT_EQ(ret, SUCCESS);
    delete[] buffer;
}

/**
 * @tc.name  : Test IsZeroVolume.
 * @tc.type  : FUNC
 * @tc.number: VolumeTools_014
 * @tc.desc  : Test IsZeroVolume.
 */
HWTEST_F(VolumeToolsUnitTest, VolumeTools_014, TestSize.Level1)
{
    EXPECT_EQ(VolumeTools::IsZeroVolume(1e-10f), true);
    EXPECT_EQ(VolumeTools::IsZeroVolume(-1e-10f), true);
    EXPECT_EQ(VolumeTools::IsZeroVolume(0.5f), false);
    EXPECT_EQ(VolumeTools::IsZeroVolume(-0.5f), false);
}
}
}
