#include "audio_engine_log.h"
#include "down_mixer.h"
#include "channel_converter.h"
#include <vector>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t TEST_FORMAT_SIZE = 4;
constexpr uint32_t TEST_FORMAT_SIZE = 4;
constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true
class ChannelConverterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void ChannelConverterTest::SetUp() {}

void ChannelConverterTest::TearDown() {}

TEST_F(ChannelConverterTest, ProcessTest)
{
    // test upmix
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MONO;
    inChannelInfo.channelLayout = CH_LAYOUT_MONO;
    outChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    ChannelConverter channelConverter;
    std::vector<float> in(TEST_BUFFER_LEN * MONO, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), DMIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), DMIX_ERR_SUCCESS);
    

    // test downmix
    inChannelInfo.numChannels = CHANNEL_6;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    std::vector<float> in.resize(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), DMIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), DMIX_ERR_SUCCESS);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS