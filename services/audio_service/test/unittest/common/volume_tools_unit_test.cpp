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

#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "volume_tools.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const size_t MAX_FRAME_SIZE = 100000;
class VolumeToolsUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test IsVolumeValid API
 * @tc.type  : FUNC
 * @tc.number: IsVolumeValid_001
 * @tc.desc  : Test IsVolumeValid interface.
 */
HWTEST(VolumeToolsUnitTest, IsVolumeValid_001, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools;
    volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_001 start");
    ChannelVolumes channelVolumes = {STEREO, {1, 2}, {3, 4}};
    bool ret = volumeTools->IsVolumeValid(channelVolumes);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_001 result:%{public}d", ret);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsVolumeValid API
 * @tc.type  : FUNC
 * @tc.number: IsVolumeValid_002
 * @tc.desc  : Test IsVolumeValid interface, when channelVolumes.channel is less than MONO(1).
 */
HWTEST(VolumeToolsUnitTest, IsVolumeValid_002, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools;
    volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_002 start");
    ChannelVolumes channelVolumes = {static_cast<AudioChannel>(0), {1, 2}, {3, 4}};
    bool ret = volumeTools->IsVolumeValid(channelVolumes);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_001 result:%{public}d", ret);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsVolumeValid API
 * @tc.type  : FUNC
 * @tc.number: IsVolumeValid_003
 * @tc.desc  : Test IsVolumeValid interface, when channelVolumes.channel is bigger than CHANNEL_16(16).
 */
HWTEST(VolumeToolsUnitTest, IsVolumeValid_003, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools;
    volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_003 start");
    ChannelVolumes channelVolumes = {static_cast<AudioChannel>(20), {1, 2}, {3, 4}};
    bool ret = volumeTools->IsVolumeValid(channelVolumes);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_001 result:%{public}d", ret);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsVolumeValid API
 * @tc.type  : FUNC
 * @tc.number: IsVolumeValid_004
 * @tc.desc  : Test IsVolumeValid interface.
 */
HWTEST(VolumeToolsUnitTest, IsVolumeValid_004, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools;
    volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_004 start");
    ChannelVolumes channelVolumes = {STEREO, {-1, 2}, {-1, 4}};
    bool ret = volumeTools->IsVolumeValid(channelVolumes);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_004 result:%{public}d", ret);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsVolumeValid API
 * @tc.type  : FUNC
 * @tc.number: IsVolumeValid_005
 * @tc.desc  : Test IsVolumeValid interface.
 */
HWTEST(VolumeToolsUnitTest, IsVolumeValid_005, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_005 start");
    ChannelVolumes channelVolumes = {STEREO, {1, 65536}, {3, 65537}};
    bool ret = volumeTools->IsVolumeValid(channelVolumes);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_005 result:%{public}d", ret);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_001
 * @tc.desc  : Test Process interface.
 */
HWTEST(VolumeToolsUnitTest, Process_001, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest Process_001 start");
    ChannelVolumes channelVolumes = {STEREO, {1, 2}, {3, 4}};
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = volumeTools->Process(bufferDesc, SAMPLE_U8, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = volumeTools->Process(bufferDesc, SAMPLE_S16LE, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = volumeTools->Process(bufferDesc, SAMPLE_S24LE, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = volumeTools->Process(bufferDesc, SAMPLE_S32LE, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = volumeTools->Process(bufferDesc, SAMPLE_F32LE, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    ret = volumeTools->Process(bufferDesc, INVALID_WIDTH, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_002
 * @tc.desc  : Test Process interface.
 */
HWTEST(VolumeToolsUnitTest, Process_002, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    size_t len = 10;
    std::unique_ptr<float[]> buffer = std::make_unique<float[]>(len);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<float>(i);
    }
    BufferDesc bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), 0, 0};
    ChannelVolumes channelVols = {};
    channelVols.channel = MONO;
    channelVols.volStart[0] = 0;
    channelVols.volEnd[0] = 0;
    int32_t ret = volumeTools->Process(bufferDesc, SAMPLE_F32LE, channelVols);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_003
 * @tc.desc  : Test Process interface.
 */
HWTEST(VolumeToolsUnitTest, Process_003, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest Process_001 start");
    ChannelVolumes channelVolumes = {STEREO, {1, 2}, {3, 4}};
    BufferDesc bufferDesc = {nullptr, 0, 0};
    bufferDesc.dataLength = 0;
    int32_t ret = volumeTools->Process(bufferDesc, SAMPLE_S16LE, channelVolumes);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_004
 * @tc.desc  : Test Process interface.
 */
HWTEST(VolumeToolsUnitTest, Process_004, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest Process_001 start");
    ChannelVolumes channelVols = {};
    channelVols.channel = CHANNEL_UNKNOW;
    channelVols.volStart[0] = 0;
    channelVols.volEnd[0] = 0;
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t ret = volumeTools->Process(bufferDesc, SAMPLE_S16LE, channelVols);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test GetVolDb API
 * @tc.type  : FUNC
 * @tc.number: GetVolDb_001
 * @tc.desc  : Test GetVolDb interface.
 */
HWTEST(VolumeToolsUnitTest, GetVolDb_001, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolDb_001 start");
    double ret = volumeTools->GetVolDb(SAMPLE_U8, 1);
    EXPECT_NE(ret, 0);
    ret = volumeTools->GetVolDb(SAMPLE_S16LE, 1);
    EXPECT_NE(ret, 0);
    ret = volumeTools->GetVolDb(SAMPLE_S24LE, 1);
    EXPECT_NE(ret, 0);
    ret = volumeTools->GetVolDb(SAMPLE_S32LE, 1);
    EXPECT_NE(ret, 0);
    ret = volumeTools->GetVolDb(SAMPLE_F32LE, 1);
    EXPECT_NE(ret, 0);
    ret = volumeTools->GetVolDb(INVALID_WIDTH, 1);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_001
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_001, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest CountVolumeLevel_001 start");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_U8, MONO);
    EXPECT_EQ(ret.channel, MONO);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S16LE, STEREO);
    EXPECT_EQ(ret.channel, STEREO);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S24LE, CHANNEL_3);
    EXPECT_EQ(ret.channel, CHANNEL_3);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S32LE, CHANNEL_4);
    EXPECT_EQ(ret.channel, CHANNEL_4);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_F32LE, CHANNEL_5);
    EXPECT_EQ(ret.channel, CHANNEL_5);
    ret = volumeTools->CountVolumeLevel(bufferDesc, INVALID_WIDTH, CHANNEL_6);
    EXPECT_EQ(ret.channel, CHANNEL_6);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_U8, static_cast<AudioChannel>(CHANNEL_16 + 1));
    EXPECT_EQ(ret.channel, (CHANNEL_16 + 1));
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_002
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_002, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    BufferDesc bufferDesc = {nullptr, 0, 0};
    size_t split = 0;
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_U8, MONO, split);
    EXPECT_EQ(ret.channel, MONO);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S16LE, STEREO, split);
    EXPECT_EQ(ret.channel, STEREO);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S24LE, CHANNEL_3, split);
    EXPECT_EQ(ret.channel, CHANNEL_3);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S32LE, CHANNEL_4, split);
    EXPECT_EQ(ret.channel, CHANNEL_4);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_F32LE, CHANNEL_5, split);
    EXPECT_EQ(ret.channel, CHANNEL_5);
    ret = volumeTools->CountVolumeLevel(bufferDesc, INVALID_WIDTH, CHANNEL_6, split);
    EXPECT_EQ(ret.channel, CHANNEL_6);
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_U8, static_cast<AudioChannel>(CHANNEL_16 + 1), split);
    EXPECT_EQ(ret.channel, (CHANNEL_16 + 1));
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_003
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_003, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    size_t len = 10;
    std::unique_ptr<int8_t[]> buffer = std::make_unique<int8_t[]>(len);
    for (size_t i = 0; i < len; ++i) {
        buffer[i] = static_cast<int8_t>(i);
    }
    size_t split = 1;
    BufferDesc bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), 10 * MAX_FRAME_SIZE, 10 * MAX_FRAME_SIZE};
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_U8, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);

    size_t size = volumeTools->GetByteSize(SAMPLE_U8);
    size_t channel = MONO;
    bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), len * size * channel, len * size * channel};
    split = 11;
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_U8, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_004
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_004, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    size_t len = 10;
    std::unique_ptr<int16_t[]> buffer = std::make_unique<int16_t[]>(len);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int16_t>(i);
    }
    
    BufferDesc bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), 10 * MAX_FRAME_SIZE, 10 * MAX_FRAME_SIZE};
    size_t split = 1;
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S16LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);

    size_t size = volumeTools->GetByteSize(SAMPLE_S16LE);
    size_t channel = MONO;
    bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), len * size * channel, len * size * channel};
    split = 11;
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S16LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_006
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_006, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    size_t len = 10;
    std::unique_ptr<int8_t[]> buffer = std::make_unique<int8_t[]>(len * 3);
    for (size_t i = 0; i < 30; ++i) {
        buffer[i] = static_cast<int8_t>(i);
    }
    BufferDesc bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), 10 * MAX_FRAME_SIZE, 10 * MAX_FRAME_SIZE};
    size_t split = 1;
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S24LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);

    size_t size = volumeTools->GetByteSize(SAMPLE_S24LE);
    size_t channel = MONO;
    bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), len * size * channel, len * size * channel};
    split = 11;
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S24LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_007
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_007, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    size_t len = 10;
    std::unique_ptr<int32_t[]> buffer = std::make_unique<int32_t[]>(len);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<int32_t>(i);
    }
    BufferDesc bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), 10 * MAX_FRAME_SIZE, 10 * MAX_FRAME_SIZE};
    size_t split = 1;
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S32LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
    
    size_t size = volumeTools->GetByteSize(SAMPLE_S32LE);
    size_t channel = MONO;
    bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), len * size * channel, len * size * channel};
    split = 11;
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_S32LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
}

/**
 * @tc.name  : Test CountVolumeLevel API
 * @tc.type  : FUNC
 * @tc.number: CountVolumeLevel_008
 * @tc.desc  : Test CountVolumeLevel interface.
 */
HWTEST(VolumeToolsUnitTest, CountVolumeLevel_008, TestSize.Level1)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    size_t len = 10;
    std::unique_ptr<float[]> buffer = std::make_unique<float[]>(len);
    for (size_t i = 0; i < 10; ++i) {
        buffer[i] = static_cast<float>(i);
    }
    BufferDesc bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), 10 * MAX_FRAME_SIZE, 10 * MAX_FRAME_SIZE};
    size_t split = 1;
    ChannelVolumes ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_F32LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
    
    size_t size = volumeTools->GetByteSize(SAMPLE_F32LE);
    size_t channel = MONO;
    bufferDesc = {reinterpret_cast<uint8_t *>(buffer.get()), len * size * channel, len * size * channel};
    split = 11;
    ret = volumeTools->CountVolumeLevel(bufferDesc, SAMPLE_F32LE, MONO, split);
    EXPECT_EQ(ret.volStart[0], 0);
}

/**
 * @tc.name  : Test IsVolumeValid API
 * @tc.type  : FUNC
 * @tc.number: IsVolumeValid_006
 * @tc.desc  : Test IsVolumeValid interface.
 */
HWTEST(VolumeToolsUnitTest, IsVolumeValid_006, TestSize.Level4)
{
    std::shared_ptr<VolumeTools> volumeTools = std::make_shared<VolumeTools>();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_006 start");
    ChannelVolumes channelVolumes = {STEREO, {1, 65536}, {3, 65537}};
    channelVolumes.channel = CHANNEL_UNKNOW;
    bool ret = volumeTools->IsVolumeValid(channelVolumes);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsVolumeValid_006 result:%{public}d", ret);
    EXPECT_EQ(ret, false);
}
} // namespace AudioStandard
} // namespace OHOS