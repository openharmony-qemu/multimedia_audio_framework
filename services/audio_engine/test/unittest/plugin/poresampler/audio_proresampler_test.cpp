#include <vector>
#include <map>
#include "audio_engine_log.h"
#include "audio_proresampler_.h"
#include "audio_stream_info.h"


namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const static std::vector<uint32_t>  TEST_CHANNELS = {MONO, STEREO, CHANNEL_6};

const static std::map<uint32_t, uint32_t> TEST_SAMPLE_RATE_COMBINATION = { // {input, output} combination
    {SAMPLE_RATE_24000, SAMPLE_RATE_48000},
    {SAMPLE_RATE_16000, SAMPLE_RATE_48000},
    {SAMPLE_RATE_44100, SAMPLE_RATE_192000},
    {SAMPLE_RATE_48000, SAMPLE_RATE_24000},
    {SAMPLE_RATE_48000, SAMPLE_RATE_16000},
    {SAMPLE_RATE_192000, SAMPLE_RATE_44100},
};

constexpr uint32_t QUALICY_ZERO = 0;
constexpr uint32_t QUALICY_ONE = 1;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t FRAME_LEN_40MS = 40;
constexpr uint32_t MS_PER_SECOND = 1000;
constexpr int32_t EOK = 0;

class AudioProResamplerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioProResamplerTest::SetUp() {}

void AudioProResamplerTest::TearDown() {}

TEST_F(AudioProResamplerTest, InitTest)
{
    // test invalid input
    SingleStagePolyphaseResamplerState state  = nullptr;
    int32_t err = RESAMPLER_ERR_SUCCESS;
    int32_t ret = SingleStagePolyphaseResamplerInit(&state, SAMPLE_RATE_24000, SAMPLE_RATE_48000, QUALICY_ZERO, &err);
    EXPECT_EQ(state, nullptr);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);

    // test valid input
    ret = SingleStagePolyphaseResamplerInit(&state, SAMPLE_RATE_24000, SAMPLE_RATE_48000, QUALICY_ONE, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_SUCCESS);

    // test 11025 input
    ProResampler resampler1(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALICY_ONE);

    // test other input
    ProResampler resampler2(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALICY_ONE);
}

TEST_F(AudioProResamplerTest, ProcessTest)
{
    // test all input/output combination
    for (uint32_t channels: TEST_CHANNELS) {
        for (auto pair: TEST_SAMPLE_RATE_COMBINATION) {
            uint32_t inRate = pair.first;
            uint32_t outRate = pair.second;
            uint32_t inFrameLen = inRate * FRAME_LEN_20MS / MS_PER_SECOND;
            uint32_t outFrameLen = outRate * FRAME_LEN_20MS / MS_PER_SECOND;
            ProResampler resampler(inRate, outRate, channels, QUALICY_ONE);
            vector<float> in(inFrameLen * channels);
            vector<float> out(outFrameLen * channels);
            int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
            EXPECT_EQ(ret, EOK);
        }
    }

    // test 11025 spetial case
    ProResampler resampler(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALICY_ONE);
    uint32_t inFrameLen = SAMPLE_RATE_11025 * FRAME_LEN_40MS / MS_PER_SECOND;
    uint32_t outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND;
    vector<float> in(inFrameLen * STEREO);
    vector<float> out(outFrameLen * STEREO);
    // Process first 40ms frame, send first half of data to output
    int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
    inFrameLen = 0;
    // no new data in, send stored 20ms
    ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
    // no data left, send 0s
    ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS