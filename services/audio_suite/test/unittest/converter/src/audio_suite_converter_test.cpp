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

#include "audio_stream_info.h"
#include "audio_suite_converter_test.h"
#include "audio_format_converter.h"
#include "audio_suite_log.h"
#include "audio_suite_unittest_tools.h"
#include <securec.h>
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <numeric>
#include "securec.h"

using namespace testing::ext;
using OHOS::AudioStandard::AudioSuite::AudioFormat;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t DEFAULT_BUFFER_SIZE = 1024;
constexpr uint32_t FILE_BUFFER_SIZE = 8000;
constexpr uint32_t EXTRA_LARGE_BUFFER_SIZE = 50000;
constexpr uint32_t LOOP_NUMBER = 10000000;
constexpr uint32_t HASH_BUFFER_SIZE = 8192;
static int32_t g_requestDataCallCount = 0;
// MD5 constants
constexpr uint32_t MD5_BLOCK_SIZE = 64;
constexpr uint32_t MD5_DIGEST_SIZE = 16;
constexpr uint32_t MD5_STATE_SIZE = 4;
constexpr uint32_t MD5_BITS_LEFT_BYTE = 3;
constexpr uint32_t MD5_WORD_COUNT = 16;
constexpr uint32_t MD5_BITS_PER_BYTE = 8;
constexpr uint32_t MD5_BITS_PER_WORD = 32;
constexpr uint32_t STATE_A_INDEX = 0;
constexpr uint32_t STATE_B_INDEX = 1;
constexpr uint32_t STATE_C_INDEX = 2;
constexpr uint32_t STATE_D_INDEX = 3;

// Padding byte
constexpr uint8_t MD5_PADDING_BYTE = 0x80;
constexpr size_t MD5_LENGTH_BYTES = 8;
// MD5 initial state
constexpr uint32_t MD5_INIT_STATE_0 = 0x67452301;
constexpr uint32_t MD5_INIT_STATE_1 = 0xEFCDAB89;
constexpr uint32_t MD5_INIT_STATE_2 = 0x98BADCFE;
constexpr uint32_t MD5_INIT_STATE_3 = 0x10325476;
constexpr uint32_t MD5_R1_K8 = 0x698098D8U;
constexpr uint32_t MD5_R2_K10 = 0xF4D50D87U;
constexpr int BYTE_WIDTH = 2;
constexpr int DEFAULT_CHANNEL_COUNT = 2;

static int32_t g_callbackCallCount = 0;
static int32_t g_callbackDataSize = DEFAULT_BUFFER_SIZE;

static int32_t g_totalBytesRead = 0;
static uint32_t g_outputCapacity = 0;
static int32_t g_totalBytesWritten = 0;

class ReadFileCallback : public AudioSuite::FormatConverterDataCallback {
public:
    explicit ReadFileCallback(FILE* file) : inputFile_(file) {}

    int32_t OnRequestData(const void** data, AudioSuite::InputDataStatus* status) override
    {
        thread_local uint8_t fileBuffer[FILE_BUFFER_SIZE];
        
        size_t bytesRead = fread(fileBuffer, 1, sizeof(fileBuffer), inputFile_);
        
        if (data != nullptr) {
            *data = fileBuffer;
        }
        if (status != nullptr) {
            if (bytesRead < sizeof(fileBuffer)) {
                *status = AudioSuite::InputDataStatus::DATA_FINISHED;
            } else {
                *status = AudioSuite::InputDataStatus::HAVE_DATA;
            }
        }
        
        g_callbackCallCount++;
        g_totalBytesRead += bytesRead;
        return static_cast<int32_t>(bytesRead);
    }

private:
    FILE* inputFile_;
};

// MD5 implementation
struct MD5Context {
    uint32_t state[MD5_STATE_SIZE];
    uint32_t count[2];
    uint8_t buffer[MD5_BLOCK_SIZE];
};

// Parameters structure for batch processing
struct ProcessBatchParams {
    AudioSuite::AudioFormatConverter* converter;
    const std::vector<uint8_t>* inputData;
    uint32_t inputSampleSize;
    uint32_t outputSampleSize;
    uint32_t bytesPerCall;
    uint32_t totalInputBytes;
    FILE* outputFile;
};

static void BinHashInit(MD5Context* ctx);
static void BinHashUpdate(MD5Context* ctx, const void* data, size_t len);
static void BinHashFinal(uint8_t digest[16], MD5Context* ctx);
static void BinHashTransform(uint32_t state[4], const uint8_t block[64]);
static void BinHashEncodeBits(uint8_t bits[8], const uint32_t count[2]);
static void BinHashEncodeDigest(uint8_t digest[16], const uint32_t state[4]);
static void BinHashProcessBuffer(MD5Context* ctx, const uint8_t** data, size_t* len, uint32_t originalCount);
static void BinHashDecodeBlock(uint32_t x[16], const uint8_t block[64]);
static void BinHashRoundOne(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);
static void BinHashRoundTwo(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);
static void BinHashRoundThree(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);
static void BinHashRoundFour(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);

static void BinHashInit(MD5Context* ctx)
{
    int32_t stateTwo = 2;
    int32_t stateThree = 3;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
    ctx->state[0] = MD5_INIT_STATE_0;
    ctx->state[1] = MD5_INIT_STATE_1;
    ctx->state[stateTwo] = MD5_INIT_STATE_2;
    ctx->state[stateThree] = MD5_INIT_STATE_3;
}

static void BinHashEncodeBits(uint8_t bits[MD5_LENGTH_BYTES], const uint32_t count[2])
{
    int32_t moveTwo = 2;
    int32_t moveThree = 3;
    for (size_t i = 0; i < MD5_LENGTH_BYTES; i++) {
        bits[i] = static_cast<uint8_t>((count[i >> moveTwo] >> ((i & moveThree) << moveThree)) & 0xFF);
    }
}

static void BinHashEncodeDigest(uint8_t digest[MD5_DIGEST_SIZE], const uint32_t state[MD5_STATE_SIZE])
{
    int32_t moveRightTwo = 2;
    int32_t moveThree = 3;
    for (size_t i = 0; i < MD5_DIGEST_SIZE; i++) {
        digest[i] = static_cast<uint8_t>((state[i >> moveRightTwo] >> ((i & moveThree) << moveThree)) & 0xFF);
    }
}

static void BinHashProcessBuffer(MD5Context* ctx, const uint8_t** data, size_t* len, uint32_t originalCount)
{
    uint8_t* p = ctx->buffer + ((originalCount >> 3) & (MD5_BLOCK_SIZE - 1));
    size_t remaining = MD5_BLOCK_SIZE - ((originalCount >> 3) & (MD5_BLOCK_SIZE - 1));

    if (*len < remaining) {
        std::copy(*data, *data + *len, p);
        return;
    }
    std::copy(*data, *data + remaining, p);
    BinHashTransform(ctx->state, ctx->buffer);
    *data += remaining;
    *len -= remaining;
}

static void BinHashUpdate(MD5Context* ctx, const void* data, size_t len)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t i = ctx->count[0];
    int32_t moveRight = 3;
    if ((ctx->count[0] += static_cast<uint32_t>(len) << MD5_BITS_LEFT_BYTE) < i) {
        ctx->count[1]++;
    }
    ctx->count[1] += static_cast<uint32_t>(len >> (MD5_BITS_PER_WORD - MD5_BITS_LEFT_BYTE));

    if ((i >> moveRight) & (MD5_BLOCK_SIZE - 1)) {
        BinHashProcessBuffer(ctx, &bytes, &len, i);
    }

    while (len >= MD5_BLOCK_SIZE) {
        BinHashTransform(ctx->state, bytes);
        bytes += MD5_BLOCK_SIZE;
        len -= MD5_BLOCK_SIZE;
    }

    std::copy(bytes, bytes + len, ctx->buffer);
}

static void BinHashFinal(uint8_t digest[MD5_DIGEST_SIZE], MD5Context* ctx)
{
    uint8_t bits[MD5_LENGTH_BYTES];
    size_t padLen;

    BinHashEncodeBits(bits, ctx->count);
    int32_t moveOne = 1;
    int32_t moveThree = 3;
    padLen = MD5_BLOCK_SIZE - ((ctx->count[0] >> moveThree) & (MD5_BLOCK_SIZE - moveOne));
    if (padLen < MD5_LENGTH_BYTES) {
        padLen += MD5_BLOCK_SIZE;
    }

    static const uint8_t padding[MD5_BLOCK_SIZE] = { MD5_PADDING_BYTE };
    BinHashUpdate(ctx, padding, padLen - MD5_LENGTH_BYTES);
    BinHashUpdate(ctx, bits, MD5_LENGTH_BYTES);

    BinHashEncodeDigest(digest, ctx->state);
    memset_s(ctx, sizeof(*ctx), 0, sizeof(*ctx));
}

// MD5 operation parameters structure
struct MD5OpParams {
    uint32_t x;
    int s;
    uint32_t ac;
};

// MD5 helper functions (replacing macros)
static inline uint32_t BinHashF(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | (~x & z);
}

static inline uint32_t BinHashG(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & z) | (y & ~z);
}

static inline uint32_t BinHashH(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

static inline uint32_t BinHashI(uint32_t x, uint32_t y, uint32_t z)
{
    return y ^ (x | ~z);
}

static inline uint32_t BinHashRotateLeft(uint32_t x, int n)
{
    return (x << n) | (x >> (MD5_BITS_PER_WORD - n));
}

static inline void BinHashFF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += BinHashF(b, c, d) + params.x + params.ac;
    a = BinHashRotateLeft(a, params.s);
    a += b;
}

static inline void BinHashGG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += BinHashG(b, c, d) + params.x + params.ac;
    a = BinHashRotateLeft(a, params.s);
    a += b;
}

static inline void BinHashHH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += BinHashH(b, c, d) + params.x + params.ac;
    a = BinHashRotateLeft(a, params.s);
    a += b;
}

static inline void BinHashII(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += BinHashI(b, c, d) + params.x + params.ac;
    a = BinHashRotateLeft(a, params.s);
    a += b;
}


static void BinHashDecodeBlock(uint32_t x[MD5_WORD_COUNT], const uint8_t block[MD5_BLOCK_SIZE])
{
    int32_t moveOne = 1;
    int32_t moveTwo = 2;
    int32_t moveThree = 3;
    for (size_t i = 0; i < MD5_WORD_COUNT; i++) {
        x[i] = static_cast<uint32_t>(block[i * MD5_STATE_SIZE]) |
               (static_cast<uint32_t>(block[i * MD5_STATE_SIZE + moveOne]) << MD5_BITS_PER_BYTE) |
               (static_cast<uint32_t>(block[i * MD5_STATE_SIZE + moveTwo]) << (MD5_BITS_PER_BYTE * moveTwo)) |
               (static_cast<uint32_t>(block[i * MD5_STATE_SIZE + moveThree]) << (MD5_BITS_PER_BYTE * moveThree));
    }
}

static void BinHashRoundOne(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    BinHashFF(a, b, c, d, {x[0], 7, 0xD76AA478});
    BinHashFF(d, a, b, c, {x[1], 12, 0xE8C7B756});
    BinHashFF(c, d, a, b, {x[2], 17, 0x242070DB});
    BinHashFF(b, c, d, a, {x[3], 22, 0xC1BDCEEE});
    BinHashFF(a, b, c, d, {x[4], 7, 0xF57C0FAF});
    BinHashFF(d, a, b, c, {x[5], 12, 0x4787C62A});
    BinHashFF(c, d, a, b, {x[6], 17, 0xA8304613});
    BinHashFF(b, c, d, a, {x[7], 22, 0xFD469501});
    BinHashFF(a, b, c, d, {x[8], 7, MD5_R1_K8});
    BinHashFF(d, a, b, c, {x[9], 12, 0x8B44F7AF});
    BinHashFF(c, d, a, b, {x[10], 17, 0xFFFF5BB1});
    BinHashFF(b, c, d, a, {x[11], 22, 0x895CD7BE});
    BinHashFF(a, b, c, d, {x[12], 7, 0x6B901122});
    BinHashFF(d, a, b, c, {x[13], 12, 0xFD987193});
    BinHashFF(c, d, a, b, {x[14], 17, 0xA679438E});
    BinHashFF(b, c, d, a, {x[15], 22, 0x49B40821});
}

static void BinHashRoundTwo(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    BinHashGG(a, b, c, d, {x[1], 5, 0xF61E2562});
    BinHashGG(d, a, b, c, {x[6], 9, 0xC040B340});
    BinHashGG(c, d, a, b, {x[11], 14, 0x265E5A51});
    BinHashGG(b, c, d, a, {x[0], 20, 0xE9B6C7AA});
    BinHashGG(a, b, c, d, {x[5], 5, 0xD62F105D});
    BinHashGG(d, a, b, c, {x[10], 9, 0x02441453});
    BinHashGG(c, d, a, b, {x[15], 14, 0xD8A1E681});
    BinHashGG(b, c, d, a, {x[4], 20, 0xE7D3FBC8});
    BinHashGG(a, b, c, d, {x[9], 5, 0x21E1CDE6});
    BinHashGG(d, a, b, c, {x[14], 9, 0xC33707D6});
    BinHashGG(c, d, a, b, {x[3], 14, MD5_R2_K10});
    BinHashGG(b, c, d, a, {x[8], 20, 0x455A14ED});
    BinHashGG(a, b, c, d, {x[13], 5, 0xA9E3E905});
    BinHashGG(d, a, b, c, {x[2], 9, 0xFCEFA3F8});
    BinHashGG(c, d, a, b, {x[7], 14, 0x676F02D9});
    BinHashGG(b, c, d, a, {x[12], 20, 0x8D2A4C8A});
}

static void BinHashRoundThree(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    BinHashHH(a, b, c, d, {x[5], 4, 0xFFFA3942});
    BinHashHH(d, a, b, c, {x[8], 11, 0x8771F681});
    BinHashHH(c, d, a, b, {x[11], 16, 0x6D9D6122});
    BinHashHH(b, c, d, a, {x[14], 23, 0xFDE5380C});
    BinHashHH(a, b, c, d, {x[1], 4, 0xA4BEEA44});
    BinHashHH(d, a, b, c, {x[4], 11, 0x4BDECFA9});
    BinHashHH(c, d, a, b, {x[7], 16, 0xF6BB4B60});
    BinHashHH(b, c, d, a, {x[10], 23, 0xBEBFBC70});
    BinHashHH(a, b, c, d, {x[13], 4, 0x289B7EC6});
    BinHashHH(d, a, b, c, {x[0], 11, 0xEAA127FA});
    BinHashHH(c, d, a, b, {x[3], 16, 0xD4EF3085});
    BinHashHH(b, c, d, a, {x[6], 23, 0x04881D05});
    BinHashHH(a, b, c, d, {x[9], 4, 0xD9D4D039});
    BinHashHH(d, a, b, c, {x[12], 11, 0xE6DB99E5});
    BinHashHH(c, d, a, b, {x[15], 16, 0x1FA27CF8});
    BinHashHH(b, c, d, a, {x[2], 23, 0xC4AC5665});
}

static void BinHashRoundFour(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    BinHashII(a, b, c, d, {x[0], 6, 0xF4292244});
    BinHashII(d, a, b, c, {x[7], 10, 0x432AFF97});
    BinHashII(c, d, a, b, {x[14], 15, 0xAB9423A7});
    BinHashII(b, c, d, a, {x[5], 21, 0xFC93A039});
    BinHashII(a, b, c, d, {x[12], 6, 0x655B59C3});
    BinHashII(d, a, b, c, {x[3], 10, 0x8F0CCC92});
    BinHashII(c, d, a, b, {x[10], 15, 0xFFEFF47D});
    BinHashII(b, c, d, a, {x[1], 21, 0x85845DD1});
    BinHashII(a, b, c, d, {x[8], 6, 0x6FA87E4F});
    BinHashII(d, a, b, c, {x[15], 10, 0xFE2CE6E0});
    BinHashII(c, d, a, b, {x[6], 15, 0xA3014314});
    BinHashII(b, c, d, a, {x[13], 21, 0x4E0811A1});
    BinHashII(a, b, c, d, {x[4], 6, 0xF7537E82});
    BinHashII(d, a, b, c, {x[11], 10, 0xBD3AF235});
    BinHashII(c, d, a, b, {x[2], 15, 0x2AD7D2BB});
    BinHashII(b, c, d, a, {x[9], 21, 0xEB86D391});
}

static void BinHashTransform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t x[16];

    BinHashDecodeBlock(x, block);
    BinHashRoundOne(a, b, c, d, x);
    BinHashRoundTwo(a, b, c, d, x);
    BinHashRoundThree(a, b, c, d, x);
    BinHashRoundFour(a, b, c, d, x);

    state[STATE_A_INDEX] += a;
    state[STATE_B_INDEX] += b;
    state[STATE_C_INDEX] += c;
    state[STATE_D_INDEX] += d;
}

static std::string CalculateBinHash(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    MD5Context ctx;
    BinHashInit(&ctx);

    char buffer[HASH_BUFFER_SIZE];
    while (file) {
        file.read(buffer, sizeof(buffer));
        BinHashUpdate(&ctx, buffer, file.gcount());
    }

    uint8_t digest[MD5_DIGEST_SIZE];
    BinHashFinal(digest, &ctx);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_SIZE; i++) {
        ss << std::hex << std::setw(BYTE_WIDTH) << std::setfill('0') << static_cast<unsigned int>(digest[i]);
    }

    file.close();
    return ss.str();
}

static void CompareOutputWithReference(const std::string& outputFilePath, const std::string& compareFileMd5)
{
    std::string outputMd5 = CalculateBinHash(outputFilePath);
    EXPECT_EQ(outputMd5.empty(), false);
    EXPECT_EQ(outputMd5, compareFileMd5);
}

struct OHConverterTestInfo {
    std::string inputFileName;
    std::string outputFileName;
    std::string compareFileMd5;
    AudioSamplingRate inputSampleRate;
    AudioChannelLayout inputChannelLayout;
    AudioSampleFormat inputSampleFormat;
    AudioSamplingRate outputSampleRate;
    AudioChannelLayout outputChannelLayout;
    AudioSampleFormat outputSampleFormat;
};

static const std::string CONVERTER_TEST_DIR = "/data/audiosuite/converter/";

static OHConverterTestInfo g_converterTestCases[] = {
    {"in_converter_44100_2_s16le.pcm", "converterOut1.pcm", "c1addbbc8df0b0b988dd812a69775150",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE},
    {"in_converter_44100_2_s16le.pcm", "converterOut2.pcm", "5558d7e09c08fdd6b3dda71569177432",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_11025_1_s16.pcm", "converterOut3.pcm", "c274111c0facfae79fea46c3a7a6153b",
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_22050_2_s24le.pcm", "converterOut4.pcm", "1c6b59462f449cf0ff6002f0c9b031af",
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S24LE},
    {"in_converter_44100_2_s16le.pcm", "converterOut5.pcm", "311b73417d67674c7bf0bf7eafc75529",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_44100_2_s16le.pcm", "converterOut6.pcm", "b97b488de2cf524b0deb472ee801f1fb",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_192000, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE},

    {"ch_in_001.pcm", "ch_out_001.pcm", "7ad1dea7cfa24bc384abf10361f68589",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_002.pcm", "ch_out_002.pcm", "8e7bf216800b562fdb3bdecf4d3d5e13",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_003.pcm", "ch_out_003.pcm", "4f4965f5dc0fa47ddad4f6719acca5b1",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_004.pcm", "ch_out_004.pcm", "60263be9555026e50a0900e937240f20",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_005.pcm", "ch_out_005.pcm", "0b4283429309cd3b6683ebb57aff2644",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_2POINT1,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_2POINT1,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_006.pcm", "ch_out_006.pcm", "8ba80cd0123365f56a753d71b3001ae2",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_5POINT1,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_5POINT1,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_007.pcm", "ch_out_007.pcm", "e8c09270ba16186d9b7590db485c51b0",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_7POINT1,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_7POINT1,
        AudioSampleFormat::SAMPLE_U8},
    {"ch_in_008.pcm", "ch_out_008.pcm", "819e720a1f9f128f98f8a61a4ce2a202",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
        AudioSampleFormat::SAMPLE_F32LE},
    {"ch_in_009.pcm", "ch_out_009.pcm", "8e7bf216800b562fdb3bdecf4d3d5e13",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_010.pcm", "ch_out_010.pcm", "4f5b26721600ce7cee23216790608948",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S32LE},
    {"ch_in_011.pcm", "ch_out_011.pcm", "bab0c4502b43f48206bdd19609767bfa",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_012.pcm", "ch_out_012.pcm", "8fdba280a550e5b252b21b739256e957",
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_012.pcm", "ch_out_013.pcm", "bda38fc6a7bb24a3a9f84afd6a3d82c0",
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_176400, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_013.pcm", "ch_out_014.pcm", "39a4e48422731dc73f8a28f50541b524",
        AudioSamplingRate::SAMPLE_RATE_176400, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_16000, AudioChannelLayout::CH_LAYOUT_3POINT1POINT2,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_014.pcm", "ch_out_015.pcm", "efe2906b3f3f4f19055f22e282b500dc",
        AudioSamplingRate::SAMPLE_RATE_192000, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_8000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
};

static OHConverterTestInfo g_converterTestCasesEqual[] = {
    {"in_converter_44100_2_s16le.pcm", "converterOut6.pcm", "b97b488de2cf524b0deb472ee801f1fb",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_192000, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE},
};
 
static OHConverterTestInfo g_converterTestCasesSmall[] = {
    {"in_converter_44100_2_s16le.pcm", "converterOut6.pcm", "d41d8cd98f00b204e9800998ecf8427e",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_192000, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE},
};

static OHConverterTestInfo g_converterAlignTestCases[] = {
    {"in_converter_44100_2_s16le.pcm", "converterAlignOut.pcm", "d791abb7b78bf83453c19d96867d1f46",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE},
    {"in_converter_44100_2_s16le.pcm", "converterAlignOut1.pcm", "2bce9b821087155bde224acd6c259ad5",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_44100_2_s16le.pcm", "converterAlignOut2.pcm", "f22b51f71ce072b3dcb51602bb04e993",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_11025_1_s16.pcm", "converterAlignOut3.pcm", "c274111c0facfae79fea46c3a7a6153b",
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_11025, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_22050_2_s24le.pcm", "converterAlignOut4.pcm", "1b88042b3b133b097c509b681b21ba09",
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S24LE},
    {"in_converter_44100_2_s16le.pcm", "converterAlignOut5.pcm", "8a19dcbd1c48b1d41928c844c9b09d05",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"in_converter_44100_2_s16le.pcm", "converterAlignOut6.pcm", "bf2f211c5177201c592a60166fe10b1e",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_192000, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_001.pcm", "ch_out_Align_001.pcm", "7ad1dea7cfa24bc384abf10361f68589",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_002.pcm", "ch_out_Align_002.pcm", "8e7bf216800b562fdb3bdecf4d3d5e13",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_003.pcm", "ch_out_Align_003.pcm", "4f4965f5dc0fa47ddad4f6719acca5b1",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_004.pcm", "ch_out_Align_004.pcm", "60263be9555026e50a0900e937240f20",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_MONO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_005.pcm", "ch_out_Align_005.pcm", "0b4283429309cd3b6683ebb57aff2644",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_2POINT1,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_2POINT1,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_006.pcm", "ch_out_Align_006.pcm", "8ba80cd0123365f56a753d71b3001ae2",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_5POINT1,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_5POINT1,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_007.pcm", "ch_out_Align_007.pcm", "e8c09270ba16186d9b7590db485c51b0",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_7POINT1,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_7POINT1,
        AudioSampleFormat::SAMPLE_U8},
    {"ch_in_008.pcm", "ch_out_Align_008.pcm", "819e720a1f9f128f98f8a61a4ce2a202",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
        AudioSampleFormat::SAMPLE_F32LE},
    {"ch_in_009.pcm", "ch_out_Align_009.pcm", "8e7bf216800b562fdb3bdecf4d3d5e13",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_010.pcm", "ch_out_Align_010.pcm", "4f5b26721600ce7cee23216790608948",
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_48000, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S32LE},
    {"ch_in_011.pcm", "ch_out_Align_011.pcm", "ee774cbbdc9a8298b9017251b2c0fae8",
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_012.pcm", "ch_out_Align_012.pcm", "40dc24ea492d185d32ea5df3fdb44203",
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_44100, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_012.pcm", "ch_out_Align_013.pcm", "f764c5e6de5478c42f8533bcc25f5a7f",
        AudioSamplingRate::SAMPLE_RATE_22050, AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_176400, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
    {"ch_in_013.pcm", "ch_out_Align_014.pcm", "37bf9cf33f2d8ee11b3a259f727b6e0f",
        AudioSamplingRate::SAMPLE_RATE_176400, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE,
        AudioSamplingRate::SAMPLE_RATE_16000, AudioChannelLayout::CH_LAYOUT_3POINT1POINT2,
        AudioSampleFormat::SAMPLE_S24LE},
    {"ch_in_014.pcm", "ch_out_Align_015.pcm", "1ecebe2e484847af7ff6f398a8348454",
        AudioSamplingRate::SAMPLE_RATE_192000, AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        AudioSampleFormat::SAMPLE_S24LE,
        AudioSamplingRate::SAMPLE_RATE_8000, AudioChannelLayout::CH_LAYOUT_STEREO,
        AudioSampleFormat::SAMPLE_S16LE},
};

static AudioFormat SetupInputFormat(const OHConverterTestInfo& info)
{
    AudioFormat format;
    format.audioChannelInfo.channelLayout = info.inputChannelLayout;
    format.audioChannelInfo.numChannels = 0; // Let Create derive from channelLayout
    format.format = info.inputSampleFormat;
    format.rate = info.inputSampleRate;
    format.encodingType = AudioSuite::ENCODING_TYPE_RAW;
    return format;
}

static AudioFormat SetupOutputFormat(const OHConverterTestInfo& info)
{
    AudioFormat format;
    format.audioChannelInfo.channelLayout = info.outputChannelLayout;
    format.audioChannelInfo.numChannels = 0; // Let Create derive from channelLayout
    format.format = info.outputSampleFormat;
    format.rate = info.outputSampleRate;
    format.encodingType = AudioSuite::ENCODING_TYPE_RAW;
    return format;
}

static std::shared_ptr<AudioSuite::AudioFormatConverter> CreateConverter(const OHConverterTestInfo& info)
{
    AudioFormat inputFormat = SetupInputFormat(info);
    AudioFormat outputFormat = SetupOutputFormat(info);
    auto converter = AudioSuite::AudioFormatConverter::Create(inputFormat, outputFormat);
    if (converter == nullptr) {
        return nullptr;
    }
    return converter;
}

static std::shared_ptr<AudioSuite::AudioFormatConverter> CreateConverterWithAlignment(const OHConverterTestInfo& info)
{
    AudioFormat inputFormat = SetupInputFormat(info);
    AudioFormat outputFormat = SetupOutputFormat(info);
    auto converter = AudioSuite::AudioFormatConverter::Create(inputFormat, outputFormat, 1, true);
    if (converter == nullptr) {
        return nullptr;
    }
    return converter;
}

static FILE* OpenInputFile(const OHConverterTestInfo& info)
{
    std::string inputFilePath = CONVERTER_TEST_DIR + info.inputFileName;
    return fopen(inputFilePath.c_str(), "rb");
}

static FILE* OpenOutputFile(const OHConverterTestInfo& info)
{
    std::string outputFilePath = CONVERTER_TEST_DIR + info.outputFileName;
    return fopen(outputFilePath.c_str(), "wb");
}

static bool ProcessConversion(AudioSuite::AudioFormatConverter* converter, FILE* outputFile)
{
    uint8_t outputBuffer[EXTRA_LARGE_BUFFER_SIZE] = {0};
    g_totalBytesWritten = 0;
    int32_t loopNumber = LOOP_NUMBER;
    while (loopNumber > 0) {
        uint32_t outputSize = 0;
        int32_t ret = converter->Process(outputBuffer, g_outputCapacity, &outputSize);
        if (ret != SUCCESS) {
            break;
        }
        
        if (outputSize == 0) {
            break;
        }
        
        size_t bytesWritten = fwrite(outputBuffer, 1, outputSize, outputFile);
        g_totalBytesWritten += bytesWritten;
        loopNumber--;
    }
    
    return g_totalBytesWritten > 0;
}

static void VerifyResults(const OHConverterTestInfo& info)
{
    std::string outputFilePath = CONVERTER_TEST_DIR + info.outputFileName;
    std::string compareFileMd5 = info.compareFileMd5;
    CompareOutputWithReference(outputFilePath, compareFileMd5);
}

static bool RunOHConverterTest(const OHConverterTestInfo& info)
{
    auto converter = CreateConverter(info);
    if (converter == nullptr) {
        return false;
    }
    
    FILE* inputFile = OpenInputFile(info);
    if (inputFile == nullptr) {
        converter->Destroy();
        return false;
    }
    
    auto fileCallback = std::make_shared<ReadFileCallback>(inputFile);
    converter->SetInputCallback(fileCallback);
    
    FILE* outputFile = OpenOutputFile(info);
    if (outputFile == nullptr) {
        fclose(inputFile);
        converter->Destroy();
        return false;
    }
    
    ProcessConversion(converter.get(), outputFile);
    
    int32_t result = fclose(inputFile);
    if (result != 0) {
        return false;
    }
    result = fclose(outputFile);
    if (result != 0) {
        return false;
    }
    
    VerifyResults(info);
    
    converter->Destroy();
    return true;
}

static bool RunOHConverterTestWithAlignment(const OHConverterTestInfo& info)
{
    auto converter = CreateConverterWithAlignment(info);
    if (converter == nullptr) {
        return false;
    }
    
    FILE* inputFile = OpenInputFile(info);
    if (inputFile == nullptr) {
        converter->Destroy();
        return false;
    }
    
    auto fileCallback = std::make_shared<ReadFileCallback>(inputFile);
    converter->SetInputCallback(fileCallback);
    
    FILE* outputFile = OpenOutputFile(info);
    if (outputFile == nullptr) {
        fclose(inputFile);
        converter->Destroy();
        return false;
    }
    
    ProcessConversion(converter.get(), outputFile);
    
    int32_t result = fclose(inputFile);
    if (result != 0) {
        return false;
    }
    result = fclose(outputFile);
    if (result != 0) {
        return false;
    }
    
    VerifyResults(info);
    
    converter->Destroy();
    return true;
}

static int32_t RunAllOHConverterTests()
{
    size_t count = sizeof(g_converterTestCases) / sizeof(g_converterTestCases[0]);
    
    for (size_t idx = 0; idx < count; idx++) {
        const OHConverterTestInfo& info = g_converterTestCases[idx];
        EXPECT_TRUE(RunOHConverterTest(info));
    }
    return SUCCESS;
}

static int32_t RunAllOHConverterAlignTests()
{
    size_t count = sizeof(g_converterAlignTestCases) / sizeof(g_converterAlignTestCases[0]);
    
    for (size_t idx = 0; idx < count; idx++) {
        const OHConverterTestInfo& info = g_converterAlignTestCases[idx];
        EXPECT_TRUE(RunOHConverterTestWithAlignment(info));
    }
    return SUCCESS;
}

void AudioSuiteConverterTest::SetUpTestCase(void) { }

void AudioSuiteConverterTest::TearDownTestCase(void) { }

void AudioSuiteConverterTest::SetUp(void)
{
    if (!AudioSuite::AllNodeTypesSupported()) {
        GTEST_SKIP() << "not support all node types, skip this test";
    }
    g_requestDataCallCount = 0;
    g_callbackCallCount = 0;
    g_callbackDataSize = DEFAULT_BUFFER_SIZE;
    g_totalBytesRead = 0;
}

void AudioSuiteConverterTest::TearDown(void) { }

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_Compare_Pcm
 * @tc.desc  : Test conversion from real pcm file and save to compare pcm.
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_Process_Compare_Pcm, TestSize.Level0)
{
    g_outputCapacity = EXTRA_LARGE_BUFFER_SIZE;
    int32_t ret = RunAllOHConverterTests();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_GT(g_callbackCallCount, 0);
    EXPECT_GT(g_totalBytesRead, 0);
    EXPECT_GT(g_totalBytesWritten, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_Compare_Pcm_Equal
 * @tc.desc  : Test conversion from real pcm file and save to compare pcm.
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_Process_Compare_Pcm_Equal, TestSize.Level0)
{
    const OHConverterTestInfo& info = g_converterTestCasesEqual[0];
    auto it = AudioSuite::LAYOUT_TO_CHANNEL.find(info.outputChannelLayout);
    uint32_t channelCount =
        (it != AudioSuite::LAYOUT_TO_CHANNEL.end()) ? static_cast<uint32_t>(it->second) : DEFAULT_CHANNEL_COUNT;
    g_outputCapacity =
        AudioSuite::AudioSuiteUtil::GetSampleSize(info.outputSampleFormat) * channelCount;
    EXPECT_TRUE(RunOHConverterTest(info));
    EXPECT_GT(g_callbackCallCount, 0);
    EXPECT_GT(g_totalBytesRead, 0);
    EXPECT_GT(g_totalBytesWritten, 0);
}
 
/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_Compare_Pcm_Small
 * @tc.desc  : Test conversion from real pcm file and save to compare pcm.
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_Process_Compare_Pcm_Small, TestSize.Level0)
{
    const OHConverterTestInfo& info = g_converterTestCasesSmall[0];
    auto it = AudioSuite::LAYOUT_TO_CHANNEL.find(info.outputChannelLayout);
    uint32_t channelCount =
        (it != AudioSuite::LAYOUT_TO_CHANNEL.end()) ? static_cast<uint32_t>(it->second) : DEFAULT_CHANNEL_COUNT;
    g_outputCapacity =
        AudioSuite::AudioSuiteUtil::GetSampleSize(info.outputSampleFormat) * channelCount - 1;
    EXPECT_TRUE(RunOHConverterTest(info));
    EXPECT_EQ(g_callbackCallCount, 0);
    EXPECT_EQ(g_totalBytesRead, 0);
    EXPECT_EQ(g_totalBytesWritten, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process without set callback.
 * @tc.number: OH_AudioConverter_DonConvert_001
 * @tc.desc  : Test calling Process before set callback.
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_DonConvert_001, TestSize.Level0)
{
    AudioFormat inputFormat;
    inputFormat.audioChannelInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.audioChannelInfo.numChannels = 2;
    inputFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    inputFormat.rate = AudioSamplingRate::SAMPLE_RATE_44100;
    inputFormat.encodingType = AudioSuite::ENCODING_TYPE_RAW;
    AudioFormat outputFormat;
    outputFormat.audioChannelInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.audioChannelInfo.numChannels = 2;
    outputFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    outputFormat.rate = AudioSamplingRate::SAMPLE_RATE_48000;
    outputFormat.encodingType = AudioSuite::ENCODING_TYPE_RAW;
    auto converter = AudioSuite::AudioFormatConverter::Create(inputFormat, outputFormat);

    uint8_t outputBuffer[DEFAULT_BUFFER_SIZE] = {0};
    uint32_t outputSize = 0;
    
    // Calling Process directly without first calling Create should return an call back not find error.
    int32_t ret = converter->Process(outputBuffer, sizeof(outputBuffer), &outputSize);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_FOUND);
    converter->Destroy();
}

/**
 * @tc.name  : Test OH_AudioConverter_Process without callback.
 * @tc.number: OH_AudioConverter_DonConvert_002
 * @tc.desc  : Test calling Process without setting callback.
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_DonConvert_002, TestSize.Level0)
{
    AudioFormat inputFormat;
    inputFormat.audioChannelInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.audioChannelInfo.numChannels = 2;
    inputFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    inputFormat.rate = AudioSamplingRate::SAMPLE_RATE_44100;
    inputFormat.encodingType = AudioSuite::ENCODING_TYPE_RAW;
    AudioFormat outputFormat;
    outputFormat.audioChannelInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.audioChannelInfo.numChannels = 2;
    outputFormat.format = AudioSampleFormat::SAMPLE_S16LE;
    outputFormat.rate = AudioSamplingRate::SAMPLE_RATE_48000;
    outputFormat.encodingType = AudioSuite::ENCODING_TYPE_RAW;
    auto converter = AudioSuite::AudioFormatConverter::Create(inputFormat, outputFormat);
    
    // Calling Process directly without setting a callback should return a "callback not set" error.
    uint8_t outputBuffer[DEFAULT_BUFFER_SIZE] = {0};
    uint32_t outputSize = 0;
    int32_t ret = converter->Process(outputBuffer, sizeof(outputBuffer), &outputSize);
    EXPECT_EQ(ret, ERR_CALLBACK_NOT_FOUND);
    
    converter->Destroy();
}

// Multithreaded testing related
static std::atomic<int32_t> g_mtSuccessCount(0);
static std::atomic<int32_t> g_mtFailCount(0);

/**
 * @tc.name  : Test OH_AudioConverter multithread with real audio files.
 * @tc.number: OH_AudioConverter_Multithread_001
 * @tc.desc  : Test multiple threads performing actual audio file conversions.
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_Multithread_001, TestSize.Level0)
{
    g_mtSuccessCount = 0;
    g_mtFailCount = 0;

    const int numThreads = 10;

    std::vector<std::thread> threads;

    // Each thread handles different test cases.
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([i]() {
            size_t testCaseIdx = i % (sizeof(g_converterTestCases) / sizeof(g_converterTestCases[0]));
            const OHConverterTestInfo& info = g_converterTestCases[testCaseIdx];
            g_outputCapacity = EXTRA_LARGE_BUFFER_SIZE;
            bool success = RunOHConverterTest(info);

            if (success) {
                g_mtSuccessCount++;
            } else {
                g_mtFailCount++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(g_mtSuccessCount, numThreads);
    EXPECT_EQ(g_mtFailCount, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process with data alignment enabled.
 * @tc.number: OH_AudioConverter_Process_Data_Alignment_001
 * @tc.desc  : Test conversion from real pcm file with data alignment (remove delay and padding).
 */
HWTEST_F(AudioSuiteConverterTest, OH_AudioConverter_Process_Data_Alignment_001, TestSize.Level0)
{
    g_outputCapacity = EXTRA_LARGE_BUFFER_SIZE;
    int32_t ret = RunAllOHConverterAlignTests();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_GT(g_callbackCallCount, 0);
    EXPECT_GT(g_totalBytesRead, 0);
    EXPECT_GT(g_totalBytesWritten, 0);
}

}
}
