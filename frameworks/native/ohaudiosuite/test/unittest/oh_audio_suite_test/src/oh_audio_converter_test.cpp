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

#include "native_audio_suite_engine.h"
#include "oh_audio_converter_test.h"
#include "OHAudioSuiteFormatConverter.h"
#include "audio_suite_log.h"
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t DEFAULT_BUFFER_SIZE = 1024;
constexpr uint32_t DOUBLE_BUFFER_SIZE = 2048;
constexpr uint32_t FILE_BUFFER_SIZE = 7056; //*60
constexpr uint32_t LARGE_BUFFER_SIZE = 10240;
constexpr uint32_t EXTRA_LARGE_BUFFER_SIZE = 50000; //500000
constexpr uint32_t SMALL_BUFFER_SIZE = 3;
constexpr uint32_t CUSTOM_BUFFER_SIZE_1022 = 1022;
constexpr uint32_t CUSTOM_BUFFER_SIZE_19200 = 19200;
constexpr uint32_t CUSTOM_BUFFER_SIZE_9600 = 9600;
constexpr uint32_t CUSTOM_BUFFER_SIZE_1600 = 1600;
// constexpr uint32_t CUSTOM_BUFFER_BUFFER_SIZE = 10000;

constexpr int32_t LOOP_COUNT_2 = 2;
constexpr int32_t LOOP_COUNT_6 = 6;
constexpr int32_t LOOP_COUNT_10 = 10;

constexpr int32_t SIGNAL_AMPLITUDE_1000 = 1000;
constexpr int32_t SIGNAL_AMPLITUDE_5000 = 5000;
constexpr int32_t SIGNAL_AMPLITUDE_3000 = 3000;

static int32_t g_requestDataCallCount = 0;
// MD5 constants

constexpr uint32_t MD5_BLOCK_SIZE = 64;
constexpr uint32_t MD5_DIGEST_SIZE = 16;
constexpr uint32_t MD5_STATE_SIZE = 4;
constexpr uint32_t MD5_BITS_LEFT_BYTE = 3;
constexpr uint32_t MD5_WORD_COUNT = 16;
constexpr uint32_t MD5_BITS_PER_BYTE = 8;
constexpr uint32_t MD5_BITS_PER_WORD = 32;

// Padding byte
constexpr uint8_t MD5_PADDING_BYTE = 0x80;
constexpr size_t MD5_LENGTH_BYTES = 8;
std::vector<int> MD5_INDEX{0, 1, 2, 3, 4,5,6,7,8,9,10,11,12,13,14,15};

static int32_t RequestDataCallback(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    g_requestDataCallCount++;
    return DEFAULT_BUFFER_SIZE;
}

static int32_t RequestDataCallbackNoData(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    (void)outInputData;
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA;
    }
    return 0;
}

static int32_t RequestDataCallbackOutStatusFinish(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    (void)outInputData;
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
    }
    return 0;
}

static int32_t RequestDataCallbackHaveData(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    return DEFAULT_BUFFER_SIZE;
}

static int32_t g_callbackCallCount = 0;
static int32_t g_callbackDataSize = DEFAULT_BUFFER_SIZE;
static FILE* g_inputFile = nullptr;
static int32_t g_totalBytesRead = 0;

static int32_t RequestDataCallbackCustomSize(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DOUBLE_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
    }
    g_callbackCallCount++;
    return g_callbackDataSize;
}

static int32_t ReadFileCallback(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    // 使用 thread_local 替代 static，解决多线程竞争问题
    // 在单线程环境下行为与 static 完全一致
    thread_local uint8_t fileBuffer[FILE_BUFFER_SIZE];
    
    // 兼容性处理：如果提供了 userData，使用它；否则使用全局变量
    // 多线程测试通过 userData 传递线程专属的文件指针
    // 单线程测试可以继续使用全局 g_inputFile
    FILE* inputFile = (userData != nullptr) ?
                     static_cast<FILE*>(userData) : g_inputFile;
    
    size_t bytesRead = fread(fileBuffer, 1, sizeof(fileBuffer), inputFile);
    
    if (outInputData != nullptr) {
        *outInputData = fileBuffer;
    }
    if (outStatus != nullptr) {
        if (bytesRead < sizeof(fileBuffer)) {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        } else {
            *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
        }
    }
    
    g_callbackCallCount++;
    g_totalBytesRead += bytesRead;
    return static_cast<int32_t>(bytesRead);
}

// MD5 implementation
struct MD5Context {
    uint32_t state[MD5_STATE_SIZE];
    uint32_t count[2];
    uint8_t buffer[MD5_BLOCK_SIZE];
};

static void MD5Init(MD5Context* ctx);
static void MD5Update(MD5Context* ctx, const void* data, size_t len);
static void MD5Final(uint8_t digest[16], MD5Context* ctx);
static void MD5Transform(uint32_t state[4], const uint8_t block[64]);
static void MD5EncodeBits(uint8_t bits[8], const uint32_t count[2]);
static void MD5EncodeDigest(uint8_t digest[16], const uint32_t state[4]);
static void MD5ProcessBuffer(MD5Context* ctx, const uint8_t** data, size_t* len, uint32_t originalCount);
static void MD5DecodeBlock(uint32_t x[16], const uint8_t block[64]);
static void MD5Round1(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);
static void MD5Round2(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);
static void MD5Round3(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);
static void MD5Round4(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16]);

static void MD5Init(MD5Context* ctx)
{
    ctx->count[0] = 0;
    ctx->count[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

static void MD5EncodeBits(uint8_t bits[MD5_LENGTH_BYTES], const uint32_t count[2])
{
    for (size_t i = 0; i < MD5_LENGTH_BYTES; i++) {
        bits[i] = static_cast<uint8_t>((count[i >> 2] >> ((i & 3) << 3)) & 0xFF);
    }
}

static void MD5EncodeDigest(uint8_t digest[MD5_DIGEST_SIZE], const uint32_t state[MD5_STATE_SIZE])
{
    for (size_t i = 0; i < MD5_DIGEST_SIZE; i++) {
        digest[i] = static_cast<uint8_t>((state[i >> 2] >> ((i & 3) << 3)) & 0xFF);
    }
}

static void MD5ProcessBuffer(MD5Context* ctx, const uint8_t** data, size_t* len, uint32_t originalCount)
{
    uint8_t* p = ctx->buffer + ((originalCount >> 3) & (MD5_BLOCK_SIZE - 1));
    size_t remaining = MD5_BLOCK_SIZE - ((originalCount >> 3) & (MD5_BLOCK_SIZE - 1));

    if (*len < remaining) {
        memcpy(p, *data, *len);
        return;
    }
    memcpy(p, *data, remaining);
    MD5Transform(ctx->state, ctx->buffer);
    *data += remaining;
    *len -= remaining;
}

static void MD5Update(MD5Context* ctx, const void* data, size_t len)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t i = ctx->count[0];

    if ((ctx->count[0] += static_cast<uint32_t>(len) << MD5_BITS_LEFT_BYTE) < i) {
        ctx->count[1]++;
    }
    ctx->count[1] += static_cast<uint32_t>(len >> (MD5_BITS_PER_WORD - MD5_BITS_LEFT_BYTE));

    if ((i >> 3) & (MD5_BLOCK_SIZE - 1)) {
        MD5ProcessBuffer(ctx, &bytes, &len, i);
    }

    while (len >= MD5_BLOCK_SIZE) {
        MD5Transform(ctx->state, bytes);
        bytes += MD5_BLOCK_SIZE;
        len -= MD5_BLOCK_SIZE;
    }

    memcpy(ctx->buffer, bytes, len);
}

static void MD5Final(uint8_t digest[MD5_DIGEST_SIZE], MD5Context* ctx)
{
    uint8_t bits[MD5_LENGTH_BYTES];
    size_t padLen;

    MD5EncodeBits(bits, ctx->count);

    padLen = MD5_BLOCK_SIZE - ((ctx->count[0] >> 3) & (MD5_BLOCK_SIZE - 1));
    if (padLen < MD5_LENGTH_BYTES) {
        padLen += MD5_BLOCK_SIZE;
    }

    static const uint8_t PADDING[MD5_BLOCK_SIZE] = { MD5_PADDING_BYTE };
    MD5Update(ctx, PADDING, padLen - MD5_LENGTH_BYTES);
    MD5Update(ctx, bits, MD5_LENGTH_BYTES);

    MD5EncodeDigest(digest, ctx->state);
    memset(ctx, 0, sizeof(*ctx));
}

// MD5 operation parameters structure
struct MD5OpParams {
    uint32_t x;
    int s;
    uint32_t ac;
};

// MD5 helper functions (replacing macros)
static inline uint32_t MD5_F(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | (~x & z);
}

static inline uint32_t MD5_G(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & z) | (y & ~z);
}

static inline uint32_t MD5_H(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

static inline uint32_t MD5_I(uint32_t x, uint32_t y, uint32_t z)
{
    return y ^ (x | ~z);
}

static inline uint32_t MD5_RotateLeft(uint32_t x, int n)
{
    return (x << n) | (x >> (MD5_BITS_PER_WORD - n));
}

static inline void MD5_FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += MD5_F(b, c, d) + params.x + params.ac;
    a = MD5_RotateLeft(a, params.s);
    a += b;
}

static inline void MD5_GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += MD5_G(b, c, d) + params.x + params.ac;
    a = MD5_RotateLeft(a, params.s);
    a += b;
}

static inline void MD5_HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += MD5_H(b, c, d) + params.x + params.ac;
    a = MD5_RotateLeft(a, params.s);
    a += b;
}

static inline void MD5_II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, const MD5OpParams& params)
{
    a += MD5_I(b, c, d) + params.x + params.ac;
    a = MD5_RotateLeft(a, params.s);
    a += b;
}


static void MD5DecodeBlock(uint32_t x[MD5_WORD_COUNT], const uint8_t block[MD5_BLOCK_SIZE])
{
    for (size_t i = 0; i < MD5_WORD_COUNT; i++) {
        x[i] = static_cast<uint32_t>(block[i * 4]) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << MD5_BITS_PER_BYTE) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << (MD5_BITS_PER_BYTE * 2)) |
               (static_cast<uint32_t>(block[i * 4 + 3]) << (MD5_BITS_PER_BYTE * 3));
    }
}

static void MD5Round1(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    MD5_FF(a, b, c, d, {x[0], 7, 0xD76AA478});
    MD5_FF(d, a, b, c, {x[1], 12, 0xE8C7B756});
    MD5_FF(c, d, a, b, {x[2], 17, 0x242070DB});
    MD5_FF(b, c, d, a, {x[3], 22, 0xC1BDCEEE});
    MD5_FF(a, b, c, d, {x[4], 7, 0xF57C0FAF});
    MD5_FF(d, a, b, c, {x[5], 12, 0x4787C62A});
    MD5_FF(c, d, a, b, {x[6], 17, 0xA8304613});
    MD5_FF(b, c, d, a, {x[7], 22, 0xFD469501});
    MD5_FF(a, b, c, d, {x[8], 7, 0x698098D8});
    MD5_FF(d, a, b, c, {x[9], 12, 0x8B44F7AF});
    MD5_FF(c, d, a, b, {x[10], 17, 0xFFFF5BB1});
    MD5_FF(b, c, d, a, {x[11], 22, 0x895CD7BE});
    MD5_FF(a, b, c, d, {x[12], 7, 0x6B901122});
    MD5_FF(d, a, b, c, {x[13], 12, 0xFD987193});
    MD5_FF(c, d, a, b, {x[14], 17, 0xA679438E});
    MD5_FF(b, c, d, a, {x[15], 22, 0x49B40821});
}

static void MD5Round2(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    MD5_GG(a, b, c, d, {x[1], 5, 0xF61E2562});
    MD5_GG(d, a, b, c, {x[6], 9, 0xC040B340});
    MD5_GG(c, d, a, b, {x[11], 14, 0x265E5A51});
    MD5_GG(b, c, d, a, {x[0], 20, 0xE9B6C7AA});
    MD5_GG(a, b, c, d, {x[5], 5, 0xD62F105D});
    MD5_GG(d, a, b, c, {x[10], 9, 0x02441453});
    MD5_GG(c, d, a, b, {x[15], 14, 0xD8A1E681});
    MD5_GG(b, c, d, a, {x[4], 20, 0xE7D3FBC8});
    MD5_GG(a, b, c, d, {x[9], 5, 0x21E1CDE6});
    MD5_GG(d, a, b, c, {x[14], 9, 0xC33707D6});
    MD5_GG(c, d, a, b, {x[3], 14, 0xF4D50D87});
    MD5_GG(b, c, d, a, {x[8], 20, 0x455A14ED});
    MD5_GG(a, b, c, d, {x[13], 5, 0xA9E3E905});
    MD5_GG(d, a, b, c, {x[2], 9, 0xFCEFA3F8});
    MD5_GG(c, d, a, b, {x[7], 14, 0x676F02D9});
    MD5_GG(b, c, d, a, {x[12], 20, 0x8D2A4C8A});
}

static void MD5Round3(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    MD5_HH(a, b, c, d, {x[5], 4, 0xFFFA3942});
    MD5_HH(d, a, b, c, {x[8], 11, 0x8771F681});
    MD5_HH(c, d, a, b, {x[11], 16, 0x6D9D6122});
    MD5_HH(b, c, d, a, {x[14], 23, 0xFDE5380C});
    MD5_HH(a, b, c, d, {x[1], 4, 0xA4BEEA44});
    MD5_HH(d, a, b, c, {x[4], 11, 0x4BDECFA9});
    MD5_HH(c, d, a, b, {x[7], 16, 0xF6BB4B60});
    MD5_HH(b, c, d, a, {x[10], 23, 0xBEBFBC70});
    MD5_HH(a, b, c, d, {x[13], 4, 0x289B7EC6});
    MD5_HH(d, a, b, c, {x[0], 11, 0xEAA127FA});
    MD5_HH(c, d, a, b, {x[3], 16, 0xD4EF3085});
    MD5_HH(b, c, d, a, {x[6], 23, 0x04881D05});
    MD5_HH(a, b, c, d, {x[9], 4, 0xD9D4D039});
    MD5_HH(d, a, b, c, {x[12], 11, 0xE6DB99E5});
    MD5_HH(c, d, a, b, {x[15], 16, 0x1FA27CF8});
    MD5_HH(b, c, d, a, {x[2], 23, 0xC4AC5665});
}

static void MD5Round4(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t x[16])
{
    MD5_II(a, b, c, d, {x[0], 6, 0xF4292244});
    MD5_II(d, a, b, c, {x[7], 10, 0x432AFF97});
    MD5_II(c, d, a, b, {x[14], 15, 0xAB9423A7});
    MD5_II(b, c, d, a, {x[5], 21, 0xFC93A039});
    MD5_II(a, b, c, d, {x[12], 6, 0x655B59C3});
    MD5_II(d, a, b, c, {x[3], 10, 0x8F0CCC92});
    MD5_II(c, d, a, b, {x[10], 15, 0xFFEFF47D});
    MD5_II(b, c, d, a, {x[1], 21, 0x85845DD1});
    MD5_II(a, b, c, d, {x[8], 6, 0x6FA87E4F});
    MD5_II(d, a, b, c, {x[15], 10, 0xFE2CE6E0});
    MD5_II(c, d, a, b, {x[6], 15, 0xA3014314});
    MD5_II(b, c, d, a, {x[13], 21, 0x4E0811A1});
    MD5_II(a, b, c, d, {x[4], 6, 0xF7537E82});
    MD5_II(d, a, b, c, {x[11], 10, 0xBD3AF235});
    MD5_II(c, d, a, b, {x[2], 15, 0x2AD7D2BB});
    MD5_II(b, c, d, a, {x[9], 21, 0xEB86D391});
}

static void MD5Transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t x[16];

    MD5DecodeBlock(x, block);
    MD5Round1(a, b, c, d, x);
    MD5Round2(a, b, c, d, x);
    MD5Round3(a, b, c, d, x);
    MD5Round4(a, b, c, d, x);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

static std::string CalculateMD5(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        AUDIO_ERR_LOG("Failed to open file for MD5 calculation: %{public}s", filePath.c_str());
        return "";
    }

    MD5Context ctx;
    MD5Init(&ctx);

    char buffer[8192];
    while (file) {
        file.read(buffer, sizeof(buffer));
        MD5Update(&ctx, buffer, file.gcount());
    }

    uint8_t digest[16];
    MD5Final(digest, &ctx);

    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }

    file.close();
    return ss.str();
}

static void CompareOutputWithReference(const std::string& outputFilePath, const std::string& compareFilePath)
{
    std::string outputMd5 = CalculateMD5(outputFilePath);
    // std::string compareMd5 = CalculateMD5(compareFilePath);

    // ASSERT_FALSE(outputMd5.empty()) << "Failed to calculate MD5 for output file: " << outputFilePath;
    // ASSERT_FALSE(compareMd5.empty()) << "Failed to calculate MD5 for compare file: " << compareFilePath;

    // AUDIO_INFO_LOG("Output file MD5: %{public}s", outputMd5.c_str());
    // AUDIO_INFO_LOG("Compare file MD5: %{public}s", compareMd5.c_str());
    // std::cout<<compareFilePath<<"Compare file MD5:"<<compareMd5.c_str()<<std::endl;

    EXPECT_EQ(outputMd5.empty(), false);
    EXPECT_EQ(outputMd5, compareFilePath);
}

struct OHConverterTestInfo {
    std::string inputFileName;
    std::string outputFileName;
    std::string compareFileMd5;
    OH_Audio_SampleRate inputSampleRate;
    OH_AudioChannelLayout inputChannelLayout;
    OH_Audio_SampleFormat inputSampleFormat;
    OH_Audio_SampleRate outputSampleRate;
    OH_AudioChannelLayout outputChannelLayout;
    OH_Audio_SampleFormat outputSampleFormat;
};

static const std::string g_CONVERTER_TEST_DIR = "/data/audiosuite/converter/";

static OHConverterTestInfo g_converterTestCases[] = {
    {"in_converter_44100_2_s16le.pcm", "converterOut1.pcm", "9f68a71725ac44ff52edf8c40babdf4f",
        OH_Audio_SampleRate::SAMPLE_RATE_44100, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_11025, OH_AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE},
    {"in_converter_44100_2_s16le.pcm", "converterOut2.pcm", "44e7e7fd85c146a4e9d215a8cbbed61c",
        OH_Audio_SampleRate::SAMPLE_RATE_44100, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_11025, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"in_converter_11025_1_s16.pcm", "converterOut3.pcm", "c274111c0facfae79fea46c3a7a6153b",
        OH_Audio_SampleRate::SAMPLE_RATE_11025, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_11025, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"in_converter_22050_2_s24le.pcm", "converterOut4.pcm", "9184b2f616a2371b7bd9537c367aacd7",
        OH_Audio_SampleRate::SAMPLE_RATE_22050, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE},
    {"in_converter_44100_2_s16le.pcm", "converterOut5.pcm", "36a049bfd4fb92cf5668c9a87dc4ed3a",
        OH_Audio_SampleRate::SAMPLE_RATE_44100, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_22050, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},

    {"ch_in_001.pcm", "ch_out_001.pcm", "7ad1dea7cfa24bc384abf10361f68589",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"ch_in_002.pcm", "ch_out_002.pcm", "8e7bf216800b562fdb3bdecf4d3d5e13",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"ch_in_003.pcm", "ch_out_003.pcm", "4f4965f5dc0fa47ddad4f6719acca5b1",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"ch_in_004.pcm", "ch_out_004.pcm", "60263be9555026e50a0900e937240f20",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_MONO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"ch_in_005.pcm", "ch_out_005.pcm", "0b4283429309cd3b6683ebb57aff2644",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_2POINT1,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_2POINT1,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"ch_in_006.pcm", "ch_out_006.pcm", "8ba80cd0123365f56a753d71b3001ae2",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_5POINT1,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_5POINT1,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE},
    {"ch_in_007.pcm", "ch_out_007.pcm", "e8c09270ba16186d9b7590db485c51b0",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_7POINT1,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_7POINT1,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_U8},
    {"ch_in_008.pcm", "ch_out_008.pcm", "819e720a1f9f128f98f8a61a4ce2a202",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE},
    {"ch_in_009.pcm", "ch_out_009.pcm", "8e7bf216800b562fdb3bdecf4d3d5e13",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE},
    {"ch_in_010.pcm", "ch_out_010.pcm", "4f5b26721600ce7cee23216790608948",
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_48000, OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE},
    {"ch_in_011.pcm", "ch_out_011.pcm", "b853cef4732fb12807f9eb35263a84d2",
        OH_Audio_SampleRate::SAMPLE_RATE_44100, OH_AudioChannelLayout::CH_LAYOUT_STEREO,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
        OH_Audio_SampleRate::SAMPLE_RATE_22050, OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
        OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE},
};

static void SetupInputFormat(OH_AudioConverter_Format& format, const OHConverterTestInfo& info)
{
    format.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    format.samplingRate = info.inputSampleRate;
    format.channelLayout = info.inputChannelLayout;
    format.sampleFormat = info.inputSampleFormat;
}

static void SetupOutputFormat(OH_AudioConverter_Format& format, const OHConverterTestInfo& info)
{
    format.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    format.samplingRate = info.outputSampleRate;
    format.channelLayout = info.outputChannelLayout;
    format.sampleFormat = info.outputSampleFormat;
}

static OH_AudioConverter* CreateConverter(const OHConverterTestInfo& info)
{
    OH_AudioConverter_Format inputFormat;
    SetupInputFormat(inputFormat, info);
    
    OH_AudioConverter_Format outputFormat;
    SetupOutputFormat(outputFormat, info);
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    if (ret != AUDIOCONVERTER_SUCCESS) {
        return nullptr;
    }
    
    return converter;
}

static FILE* OpenInputFile(const OHConverterTestInfo& info)
{
    std::string inputFilePath = g_CONVERTER_TEST_DIR + info.inputFileName;
    return fopen(inputFilePath.c_str(), "rb");
}

static FILE* OpenOutputFile(const OHConverterTestInfo& info)
{
    std::string outputFilePath = g_CONVERTER_TEST_DIR + info.outputFileName;
    return fopen(outputFilePath.c_str(), "wb");
}

static bool ProcessConversion(OH_AudioConverter* converter, FILE* outputFile)
{
    uint8_t outputBuffer[EXTRA_LARGE_BUFFER_SIZE] = {0};
    // std::vector<uint8_t>outputBuffer(EXTRA_LARGE_BUFFER_SIZE, 0);
    int32_t totalBytesWritten = 0;
    
    while (true) {
        int32_t outputSize = 0;
        OH_AudioConverter_Result ret = OH_AudioConverter_Process(converter, outputBuffer,
            sizeof(outputBuffer), &outputSize);
        if (ret != AUDIOCONVERTER_SUCCESS) {
            break;
        }
        
        if (outputSize == 0) {
            break;
        }
        
        size_t bytesWritten = fwrite(outputBuffer, 1, outputSize, outputFile);
        totalBytesWritten += bytesWritten;
    }
    
    return totalBytesWritten > 0;
}

static void VerifyResults(const OHConverterTestInfo& info, int32_t totalBytesWritten)
{
    std::string outputFilePath = g_CONVERTER_TEST_DIR + info.outputFileName;
    std::string compareFilePath = info.compareFileMd5;
    CompareOutputWithReference(outputFilePath, compareFilePath);
    
    EXPECT_GT(g_callbackCallCount, 0);
    EXPECT_GT(g_totalBytesRead, 0);
    EXPECT_GT(totalBytesWritten, 0);
}

static int32_t g_callbackCallCount_016 = 0;

static int32_t RequestDataCallback_016(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[CUSTOM_BUFFER_SIZE_19200] = {0};
    
    for (int32_t i = 0; i < CUSTOM_BUFFER_SIZE_19200 / LOOP_COUNT_2; i += LOOP_COUNT_2) {
        int16_t sample =
            static_cast<int16_t>(SIGNAL_AMPLITUDE_1000 * sin(2 * M_PI * i / 48000));
        testData[i] = sample & 0xFF;
        testData[i + 1] = (sample >> 8) & 0xFF;
    }
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    
    g_callbackCallCount_016++;
    return CUSTOM_BUFFER_SIZE_19200;
}

static int32_t g_callbackCallCount_017 = 0;

static int32_t RequestDataCallback_017(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[CUSTOM_BUFFER_SIZE_9600] = {0};
    
    for (int32_t i = 0; i < CUSTOM_BUFFER_SIZE_9600 / LOOP_COUNT_2; i += LOOP_COUNT_2) {
        int16_t sample =
            static_cast<int16_t>(SIGNAL_AMPLITUDE_5000 * sin(2 * M_PI * i / 48000));
        testData[i] = sample & 0xFF;
        testData[i + 1] = (sample >> 8) & 0xFF;
    }
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    
    g_callbackCallCount_017++;
    return CUSTOM_BUFFER_SIZE_9600;
}

static int32_t g_callbackCallCount_018 = 0;

static int32_t RequestDataCallback_018(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[CUSTOM_BUFFER_SIZE_1600] = {0};
    
    for (int32_t i = 0; i < CUSTOM_BUFFER_SIZE_1600 / LOOP_COUNT_2; i += LOOP_COUNT_2) {
        int16_t sample =
            static_cast<int16_t>(SIGNAL_AMPLITUDE_3000 * sin(2 * M_PI * i / 8000));
        testData[i] = sample & 0xFF;
        testData[i + 1] = (sample >> 8) & 0xFF;
    }
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    
    g_callbackCallCount_018++;
    return CUSTOM_BUFFER_SIZE_1600;
}

static int32_t g_callCount_024 = 0;

static int32_t RequestDataCallback_024(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        if (g_callCount_024 == 0) {
            *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
        } else {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        }
    }
    
    g_callCount_024++;
    return DEFAULT_BUFFER_SIZE;
}

static int32_t g_callCount_030 = 0;

static int32_t RequestDataCallback_030(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        if (g_callCount_030 < LOOP_COUNT_2 + 1) {
            *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
        } else {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        }
    }
    
    g_callCount_030++;
    return DEFAULT_BUFFER_SIZE;
}

static bool RunOHConverterTest(const OHConverterTestInfo& info)
{
    OH_AudioConverter* converter = CreateConverter(info);
    if (converter == nullptr) {
        return false;
    }
    
    // 使用局部变量替代全局 g_inputFile，解决多线程竞争问题
    FILE* inputFile = OpenInputFile(info);
    if (inputFile == nullptr) {
        OH_AudioConverter_Destroy(converter);
        return false;
    }
    
    // 将文件指针作为 userData 传递给回调函数
    // 多线程环境下每个线程有独立的 inputFile，避免竞争
    OH_AudioConverter_Result ret = OH_AudioConverter_SetInputCallback(converter, ReadFileCallback, inputFile);
    if (ret != AUDIOCONVERTER_SUCCESS) {
        fclose(inputFile);
        OH_AudioConverter_Destroy(converter);
        return false;
    }
    
    FILE* outputFile = OpenOutputFile(info);
    if (outputFile == nullptr) {
        fclose(inputFile);
        OH_AudioConverter_Destroy(converter);
        return false;
    }
    
    int32_t totalBytesWritten = ProcessConversion(converter, outputFile);
    
    fclose(inputFile);
    fclose(outputFile);
    
    VerifyResults(info, totalBytesWritten);
    
    OH_AudioConverter_Destroy(converter);
    return true;
}

static int32_t RunAllOHConverterTests()
{
    size_t count = sizeof(g_converterTestCases) / sizeof(g_converterTestCases[0]);
    
    for (size_t idx = 0; idx < count; idx++) {
        const OHConverterTestInfo& info = g_converterTestCases[idx];
        AUDIO_INFO_LOG("Running test case [%{public}zu/%{public}zu]: %{public}s", 
                       idx + 1, count, info.inputFileName.c_str());
        
        RunOHConverterTest(info);
    }
    return SUCCESS;
}

void OHAudioConverterTest::SetUpTestCase(void) { }

void OHAudioConverterTest::TearDownTestCase(void) { }

void OHAudioConverterTest::SetUp(void)
{
    g_requestDataCallCount = 0;
    g_callbackCallCount = 0;
    g_callbackDataSize = 1024;
    g_totalBytesRead = 0;
}

void OHAudioConverterTest::TearDown(void) { }

/**
 * @tc.name  : Test OH_AudioConverter_Create.
 * @tc.number: OH_AudioConverter_Create_001
 * @tc.desc  : Test nullptr parameters.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Create_001, TestSize.Level0)
{
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    ret = OH_AudioConverter_Create(&inputFormat, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    ret = OH_AudioConverter_Create(nullptr, &outputFormat, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    ret = OH_AudioConverter_Create(&inputFormat, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    ret = OH_AudioConverter_Create(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioConverter_Create.
 * @tc.number: OH_AudioConverter_Create_002
 * @tc.desc  : Test success.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Create_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_NE(converter, nullptr);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Create.
 * @tc.number: OH_AudioConverter_Create_003
 * @tc.desc  : Test create with different sample rates.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Create_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_16000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_NE(converter, nullptr);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Destroy.
 * @tc.number: OH_AudioConverter_Destroy_001
 * @tc.desc  : Test success.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Destroy_001, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(nullptr);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_001
 * @tc.desc  : Test nullptr converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_001, TestSize.Level0)
{
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_002
 * @tc.desc  : Test nullptr callback.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_CALLBACK_INVALID);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_003
 * @tc.desc  : Test success.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_004
 * @tc.desc  : Test set callback multiple times.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_004, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_001
 * @tc.desc  : Test nullptr converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_001, TestSize.Level0)
{
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    OH_AudioConverter_Result ret = OH_AudioConverter_Process(nullptr, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_002
 * @tc.desc  : Test nullptr outputData.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, nullptr, LARGE_BUFFER_SIZE, &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_003
 * @tc.desc  : Test nullptr outputSize.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_004
 * @tc.desc  : Test callback not set.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_004, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_005
 * @tc.desc  : Test success with callback returning finished.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_005, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;

    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(g_requestDataCallCount, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_006
 * @tc.desc  : Test callback returning no available data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_006, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackNoData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(outputSize, 0);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackOutStatusFinish, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_007
 * @tc.desc  : Test callback returning have data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_007, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackHaveData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_008
 * @tc.desc  : Test process after input finished.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_008, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_009
 * @tc.desc  : Test mono to stereo conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_009, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_010
 * @tc.desc  : Test sample rate conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_010, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_011
 * @tc.desc  : Test cache accumulation - data not aligned.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_011, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackCustomSize, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    g_callbackDataSize = CUSTOM_BUFFER_SIZE_1022;
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_012
 * @tc.desc  : Test cache satisfied after accumulation.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_012, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackCustomSize, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    g_callbackDataSize = CUSTOM_BUFFER_SIZE_1022;
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);

    g_callbackDataSize = LOOP_COUNT_2;
    outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_013。
 * @tc.desc  : Test output buffer too small.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_013, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_22050;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[SMALL_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_014
 * @tc.desc  : Test multiple Process calls.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_014, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_015
 * @tc.desc  : Test empty input data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_015, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackNoData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_016
 * @tc.desc  : Test functional format conversion with real audio data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_016, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callbackCallCount_016 = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_016, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[EXTRA_LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    EXPECT_GT(g_callbackCallCount_016, 0);
    
    bool hasNonZeroData = false;
    for (int32_t i = 0; i < outputSize; i++) {
        if (outputData[i] != 0) {
            hasNonZeroData = true;
            break;
        }
    }
    EXPECT_TRUE(hasNonZeroData);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_017
 * @tc.desc  : Test mono to stereo conversion with real audio data.
 *
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_017, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    int32_t inputBytes = CUSTOM_BUFFER_SIZE_9600;
    g_callbackCallCount_017 = 0;
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_017, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[19200] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    EXPECT_GT(g_callbackCallCount_017, 0);
    
    EXPECT_EQ(outputSize, inputBytes * LOOP_COUNT_2);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_018
 * @tc.desc  : Test sample rate conversion with real audio data.
 *
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_018, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callbackCallCount_018 = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_018, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[9600] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    EXPECT_GT(g_callbackCallCount_018, 0);
    
    EXPECT_EQ(outputSize, CUSTOM_BUFFER_SIZE_1600 * LOOP_COUNT_6);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_019
 * @tc.desc  : Test conversion from real pcm file and save to compare pcm.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Compare_Pcm, TestSize.Level0)
{
    int32_t ret = RunAllOHConverterTests();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioConverter_Reset.
 * @tc.number: OH_AudioConverter_Reser_001
 * @tc.desc  : Test Rest Method.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Reset_001, TestSize.Level0)
{
    OHAudioConverter converter;
    converter.inputFinished_ = true;
    converter.formatConversion_ = nullptr;
    converter.Reset();
    EXPECT_EQ(converter.inputFinished_, false);

    converter.formatConversion_ = std::make_unique<AudioSuite::AudioSuiteFormatConversion>();
    converter.Reset();
    EXPECT_EQ(converter.formatConversion_->resampleCfg_.inRate, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_024
 * @tc.desc  : Test callback returning DATA_FINISHED status.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_024, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callCount_024 = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_024, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_025
 * @tc.desc  : Test padding logic when input finished.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_025, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
      
    auto callback = [](void* userData, const void** outInputData,
        OH_AudioConverter_InputStatus* outStatus) -> int32_t {
        (void)userData;
        static uint8_t testData[CUSTOM_BUFFER_SIZE_1022] = {0};
        
        if (outInputData != nullptr) {
            *outInputData = testData;
        }
        if (outStatus != nullptr) {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        }
        return CUSTOM_BUFFER_SIZE_1022;
    };
    
    ret = OH_AudioConverter_SetInputCallback(converter, callback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_026
 * @tc.desc  : Test invalid input format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_026, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = static_cast<OH_Audio_SampleRate>(0);
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_027
 * @tc.desc  : Test invalid output format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_027, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = static_cast<OH_Audio_SampleRate>(0);
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_028
 * @tc.desc  : Test 11025Hz special handling.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_028, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_11025;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[SMALL_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_029
 * @tc.desc  : Test output buffer exactly enough.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_029, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[DEFAULT_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_030
 * @tc.desc  : Test multiple Process calls until completion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_030, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callCount_030 = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_030, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t totalOutputSize = 0;
    int32_t processCount = 0;
    
    while (processCount < LOOP_COUNT_10) {
        int32_t outputSize = 0;
        ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
        EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
        
        totalOutputSize += outputSize;
        processCount++;
        
        if (outputSize == 0) {
            break;
        }
    }
    
    EXPECT_GT(totalOutputSize, 0);
    EXPECT_LE(processCount, LOOP_COUNT_10);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_031
 * @tc.desc  : Test stereo to mono conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_031, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_032
 * @tc.desc  : Test 16000Hz sample rate conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_032, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_16000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_DonConvert_001
 * @tc.desc  : Test invalid input format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_DonConvert_001, TestSize.Level0)
{
    OHAudioConverter converter;
    converter.formatConversion_ = nullptr;
    AudioSuite::AudioSuitePcmBuffer* inputBuffer = nullptr;
    AudioSuite::AudioSuitePcmBuffer* ret = converter.DoConvert(inputBuffer);
    EXPECT_EQ(ret, nullptr);
}

// 400KB = 400 * 1024 = 1048576 bytes
constexpr uint32_t INPUT_DATA_SIZE = 1024 * 400;
constexpr uint32_t INPUT_DATA_SIZE_PLUS = 2 * 400 * 1024;

static int32_t g_callbackCallCount_034 = 0;

/**
 * @brief Callback function that returns data larger than 400KB
 */
static int32_t RequestDataCallback_LargerThanOneMB(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[INPUT_DATA_SIZE_PLUS] = {0};

    *outInputData = testData;
    *outStatus = OH_AudioConverter_InputStatus::AUDIOCONVERTER_INPUT_HAVE_DATA;
    g_callbackCallCount_034++;

    return INPUT_DATA_SIZE_PLUS;
}

/**
 * @brief Callback function that returns data exactly equal to 400KB
 */
static int32_t RequestDataCallback_EqualsOneMB(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    // static uint8_t testData[INPUT_DATA_SIZE] = {0};
    std::vector<uint8_t>testData(INPUT_DATA_SIZE, 0);
    if (outInputData != nullptr) {
        *outInputData = testData.data();
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
    }
    g_callbackCallCount_034++;
    return testData.capacity();
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_034
 * @tc.desc  : Test callback returns data larger than 400KB.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_034, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_LargerThanOneMB, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // Allocate buffer larger than 400KB to hold the output
    std::vector<uint8_t> outputData(INPUT_DATA_SIZE_PLUS, 0);
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData.data(), INPUT_DATA_SIZE_PLUS * 2, &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_SYSTEM);
    EXPECT_EQ(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_035
 * @tc.desc  : Test callback returns data exactly equal to 400KB.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_035, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_EqualsOneMB, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // Allocate buffer larger than 400KB to hold the output
    std::vector<uint8_t> outputData(INPUT_DATA_SIZE, 0);
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData.data(), INPUT_DATA_SIZE * 2, &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_036
 * @tc.desc  : Test invalid input format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_036, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_22POINT2;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    OH_AudioConverter_Destroy(converter);
}


// 多线程测试相关
static std::atomic<int32_t> g_mtCallbackCallCount(0);
static std::atomic<int32_t> g_mtSuccessCount(0);
static std::atomic<int32_t> g_mtFailCount(0);

// 多线程测试回调函数
static int32_t RequestDataCallbackMultithread(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static thread_local uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    g_mtCallbackCallCount++;
    return DEFAULT_BUFFER_SIZE;
}

// 单个线程的测试任务
static void ThreadTestTask(int threadId, int iterations)
{
    for (int i = 0; i < iterations; i++) {
        OH_AudioConverter_Format inputFormat;
        inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
        inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
        inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
        inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

        OH_AudioConverter_Format outputFormat;
        outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
        outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
        outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
        outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;

        OH_AudioConverter* converter = nullptr;
        OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);

        if (ret != AUDIOCONVERTER_SUCCESS || converter == nullptr) {
            g_mtFailCount++;
            continue;
        }

        ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackMultithread, nullptr);
        if (ret != AUDIOCONVERTER_SUCCESS) {
            g_mtFailCount++;
            OH_AudioConverter_Destroy(converter);
            continue;
        }

        uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
        int32_t outputSize = 0;
        ret = OH_AudioConverter_Process(converter, outputData, LARGE_BUFFER_SIZE, &outputSize);

        if (ret != AUDIOCONVERTER_SUCCESS) {
            g_mtFailCount++;
        } else {
            g_mtSuccessCount++;
        }

        OH_AudioConverter_Destroy(converter);
    }
}

/**
 * @tc.name  : Test OH_AudioConverter multithread independent converters.
 * @tc.number: OH_AudioConverter_Multithread_001
 * @tc.desc  : Test multiple threads with independent converters.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Multithread_001, TestSize.Level0)
{
    g_mtSuccessCount = 0;
    g_mtFailCount = 0;

    const int numThreads = 4;
    const int iterations = 10;

    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(ThreadTestTask, i, iterations);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(g_mtSuccessCount, numThreads * iterations);
    EXPECT_EQ(g_mtFailCount, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter concurrent create/destroy.
 * @tc.number: OH_AudioConverter_Multithread_002
 * @tc.desc  : Test concurrent create and destroy operations.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Multithread_002, TestSize.Level0)
{
    g_mtSuccessCount = 0;
    g_mtFailCount = 0;

    const int numThreads = 8;
    const int iterations = 20;

    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([]() {
            for (int j = 0; j < iterations; j++) {
                OH_AudioConverter_Format inputFormat;
                inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
                inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
                inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
                inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

                OH_AudioConverter_Format outputFormat;
                outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
                outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
                outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
                outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;

                OH_AudioConverter* converter = nullptr;
                OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);

                if (ret == AUDIOCONVERTER_SUCCESS && converter != nullptr) {
                    OH_AudioConverter_Destroy(converter);
                    g_mtSuccessCount++;
                } else {
                    g_mtFailCount++;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(g_mtSuccessCount, numThreads * iterations);
    EXPECT_EQ(g_mtFailCount, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter multithread with real audio files.
 * @tc.number: OH_AudioConverter_Multithread_003
 * @tc.desc  : Test multiple threads performing actual audio file conversions.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Multithread_003, TestSize.Level0)
{
    g_mtSuccessCount = 0;
    g_mtFailCount = 0;

    const int numThreads = 10;

    std::vector<std::thread> threads;

    // 每个线程处理不同的测试用例
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([i]() {
            // 确保每个线程使用不同的测试用例索引
            size_t testCaseIdx = i % (sizeof(g_converterTestCases) / sizeof(g_converterTestCases[0]));
            const OHConverterTestInfo& info = g_converterTestCases[testCaseIdx];

            // 执行实际的音频文件转换测试
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
 * @tc.name  : Test OH_AudioConverter_Process with full cache consumption
 * @tc.number: OH_AudioConverter_Process_Cached_001
 * @tc.desc  : Test Process when all cached data is consumed (branch 5)
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Cached_001, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // 使用小数据量的回调，确保缓存能被完全消耗
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackCustomSize, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    g_callbackDataSize = 100;  // 设置较小的数据量

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // 继续调用直到没有更多数据
    int32_t totalOutput = outputSize;
    int32_t maxIterations = 10;
    int32_t iterations = 0;
    while (outputSize > 0 && iterations < maxIterations) {
        outputSize = 0;
        ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
        if (ret == AUDIOCONVERTER_SUCCESS) {
            totalOutput += outputSize;
        }
        iterations++;
    }

    EXPECT_GT(totalOutput, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process cache alignment
 * @tc.number: OH_AudioConverter_Process_Cached_002
 * @tc.desc  : Test Process with non-frame-aligned output capacity
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Cached_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackHaveData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // 使用非帧对齐的输出容量 (STEREO + S16LE = 4 bytes per frame, 1022 is not divisible by 4)
    uint8_t outputData[CUSTOM_BUFFER_SIZE_1022] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // 输出大小应该是帧对齐的 (4的倍数)
    EXPECT_EQ(outputSize % 4, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process with partial cache consumption
 * @tc.number: OH_AudioConverter_Process_Cached_003
 * @tc.desc  : Test Process when only part of cached data is consumed (branch 4)
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Cached_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // 第一次调用产生缓存数据
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_016, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[DEFAULT_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // 第二次调用，可能使用剩余的缓存数据（部分复制分支）
    outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

}
}
