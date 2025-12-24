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
#include "audio_secure_hash.h"
#include <cstring>
#include <cstdint>
#include <cerrno>   // for EINVAL
#include <cstdlib>  // for size_t
#include "audio_utils.h"
#include "audio_log.h"

// bit operations
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n)))) // rotate right
#define SHR(x, n) ((x) >> (n)) // shfit right

#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z))) // choose bits from y or z based on x
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z))) // majority of bits among x, y, z

#define Sigma0(x) (ROTR((x), 2) ^ ROTR((x), 13) ^ ROTR((x), 22)) // uppercase sigma0, rotates 2,13,22
#define Sigma1(x) (ROTR((x), 6) ^ ROTR((x), 11) ^ ROTR((x), 25)) // uppercase sigma1, rotates 6,11,25
#define sigma0(x) (ROTR((x), 7) ^ ROTR((x), 18) ^ SHR((x), 3)) // lowercase sigma0, rotates 7,18, shfit 3
#define sigma1(x) (ROTR((x), 17) ^ ROTR((x), 19) ^ SHR((x), 10)) // lowercase sigma0, rotates 17,19, shfit 10

namespace OHOS {
namespace AudioStandard {

namespace {
static constexpr size_t BLOCK_SIZE = 64;  // single msg length in bytes
static constexpr size_t DIGEST_SIZE = 32; // output hash length in bytes
static constexpr size_t PAD_LENGTH = 56;  // padding length before appending msg length
static constexpr size_t BYTES_PER_WORD = 4; // bytes per word, 32bit

// h0~h7 state registers
enum StateWord {
    STATE_WORD_H0 = 0,
    STATE_WORD_H1,
    STATE_WORD_H2,
    STATE_WORD_H3,
    STATE_WORD_H4,
    STATE_WORD_H5,
    STATE_WORD_H6,
    STATE_WORD_H7,
    STATE_WORDS_COUNT = 8
};

// hash constants K[t]
static const uint32_t K256[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};
}
// context
struct AlgoCTX {
    uint32_t h[STATE_WORDS_COUNT];  // hash state h0~h7
    unsigned char data[BLOCK_SIZE]; // msg block buffer
    uint32_t num;                   // bytes in data[]
    unsigned long long nbits;       // total bits processed
};

// internal functions
static void AlgoInit(AlgoCTX* c);
static bool AlgoUpdate(AlgoCTX* c, const unsigned char *data, size_t len);
static bool AlgoFinal(unsigned char *md, AlgoCTX* c);
static void AlgoTransform(AlgoCTX* ctx, const unsigned char *block);

// public interface
unsigned char* AudioSecureHash::AudioSecureHashAlgo(const unsigned char *d, size_t n, unsigned char *md)
{
    // illegal state, d == nullptr && n > 0
    CHECK_AND_RETURN_RET_LOG(d != nullptr || n == 0, nullptr, "illegal parameter");

    static unsigned char staticBuffer[SHA256_DIGEST_LENGTH];
    if (md == nullptr) {
        md = staticBuffer;
    }

    AlgoCTX ctx;
    AlgoInit(&ctx);

    CHECK_AND_RETURN_RET(AlgoUpdate(&ctx, d, n), nullptr);
    CHECK_AND_RETURN_RET(AlgoFinal(md, &ctx), nullptr);
    return md;
}

// context init
static void AlgoInit(AlgoCTX* c)
{
    // init state h[0~7]
    c->h[STATE_WORD_H0] = 0x6a09e667UL;
    c->h[STATE_WORD_H1] = 0xbb67ae85UL;
    c->h[STATE_WORD_H2] = 0x3c6ef372UL;
    c->h[STATE_WORD_H3] = 0xa54ff53aUL;
    c->h[STATE_WORD_H4] = 0x510e527fUL;
    c->h[STATE_WORD_H5] = 0x9b05688cUL;
    c->h[STATE_WORD_H6] = 0x1f83d9abUL;
    c->h[STATE_WORD_H7] = 0x5be0cd19UL;
    c->num = 0;
    c->nbits = 0;
}

// transform using full W[64] schedule (clear and explicit)
static void AlgoTransform(AlgoCTX* ctx, const unsigned char *block)
{
    uint32_t w[BLOCK_SIZE];
    // load big-endian
    for (size_t t = 0; t < 16; ++t) { // load first 16 works from input block, big-endian
        w[t] = (static_cast<uint32_t>(block[BYTES_PER_WORD * t]) << 24) |     // byte 0, shift 24, bits[24~31]
               (static_cast<uint32_t>(block[BYTES_PER_WORD * t + 1]) << 16) | // byte 1, shift 16, bits[16~23]
               (static_cast<uint32_t>(block[BYTES_PER_WORD * t + 2]) << 8) |  // byte 2, shift 8, bits[8~15]
               (static_cast<uint32_t>(block[BYTES_PER_WORD * t + 3]));        // byte 3, lowest byte, bits[0~7]
    }
    for (size_t t = 16; t < BLOCK_SIZE; ++t) { // extend W[16~63]
        w[t] = static_cast<uint32_t>(
            static_cast<uint64_t>(sigma1(w[t - 2])) + // t-2: second previous word, used in sigma1
                                  w[t - 7] +          // t-7: seventh previous word, add directly
                                  sigma0(w[t - 15]) + // t-15: fifteenth previous word, used in sigma0
                                  w[t - 16]           // t-16: sixteenth previous word, earliest word in schedule
        );
    }

    uint32_t a = ctx->h[STATE_WORD_H0];
    uint32_t b = ctx->h[STATE_WORD_H1];
    uint32_t c = ctx->h[STATE_WORD_H2];
    uint32_t d = ctx->h[STATE_WORD_H3];
    uint32_t e = ctx->h[STATE_WORD_H4];
    uint32_t f = ctx->h[STATE_WORD_H5];
    uint32_t g = ctx->h[STATE_WORD_H6];
    uint32_t h = ctx->h[STATE_WORD_H7];

    for (size_t t = 0; t < BLOCK_SIZE; ++t) {
        uint32_t t1 = static_cast<uint32_t>(static_cast<uint64_t>(h) + Sigma1(e) + Ch(e, f, g) + K256[t] + w[t]);
        uint32_t t2 = static_cast<uint32_t>(static_cast<uint64_t>(Sigma0(a)) + Maj(a, b, c));
        h = g;
        g = f;
        f = e;
        e = static_cast<uint32_t>(static_cast<uint64_t>(d) + t1);
        d = c;
        c = b;
        b = a;
        a = static_cast<uint32_t>(static_cast<uint64_t>(t1) + t2);
    }

    ctx->h[STATE_WORD_H0] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H0]) + a);
    ctx->h[STATE_WORD_H1] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H1]) + b);
    ctx->h[STATE_WORD_H2] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H2]) + c);
    ctx->h[STATE_WORD_H3] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H3]) + d);
    ctx->h[STATE_WORD_H4] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H4]) + e);
    ctx->h[STATE_WORD_H5] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H5]) + f);
    ctx->h[STATE_WORD_H6] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H6]) + g);
    ctx->h[STATE_WORD_H7] = static_cast<uint32_t>(static_cast<uint64_t>(ctx->h[STATE_WORD_H7]) + h);
}

// update
static bool AlgoUpdate(AlgoCTX* c, const unsigned char *data, size_t len)
{
    CHECK_AND_RETURN_RET(len != 0, true); // len == 0, return true directly
    // update bit count (message bits only)
    unsigned long long add = static_cast<unsigned long long>(len) * 8ULL; // 8, convert bytes to bits
    c->nbits += add;

    // fill existing buffer if any
    if (c->num) {
        size_t need = BLOCK_SIZE - c->num;
        if (len < need) {
            CHECK_AND_RETURN_RET(memcpy_s(c->data + c->num, sizeof(c->data) - c->num, data, len) == EOK, false);
            c->num += static_cast<uint32_t>(len);
            return true;
        } else {
            CHECK_AND_RETURN_RET(memcpy_s(c->data + c->num, sizeof(c->data) - c->num, data, need) == EOK, false);
            AlgoTransform(c, c->data);
            data += need;
            len -= need;
            c->num = 0;
        }
    }

    // process complete blocks directly from input
    while (len >= BLOCK_SIZE) {
        AlgoTransform(c, data);
        data += BLOCK_SIZE;
        len -= BLOCK_SIZE;
    }

    // store remainder
    if (len > 0) {
        CHECK_AND_RETURN_RET(memcpy_s(c->data, sizeof(c->data), data, len) == EOK, false);
        c->num = static_cast<uint32_t>(len);
    }
    return true;
}

// final
static bool AlgoFinal(unsigned char *md, AlgoCTX* c)
{
    unsigned char tmp[64];
    size_t len = c->num;

    // copy remaining data
    if (len > 0) {
        CHECK_AND_RETURN_RET(memcpy_s(tmp, sizeof(tmp), c->data, len) == EOK, false);
    }
    // append 0x80
    tmp[len++] = 0x80;

    if (len > PAD_LENGTH) {
        // pad to 64 and process
        CHECK_AND_RETURN_RET(memset_s(tmp + len, sizeof(tmp) - len, 0, BLOCK_SIZE - len) == EOK, false);
        AlgoTransform(c, tmp);
        // new block
        CHECK_AND_RETURN_RET(memset_s(tmp, sizeof(tmp), 0, PAD_LENGTH) == EOK, false);
    } else {
        // pad zeros until 56
        CHECK_AND_RETURN_RET(memset_s(tmp + len, sizeof(tmp) - len, 0, PAD_LENGTH - len) == EOK, false);
    }

    // append length in bits as big-endian 8-byte value
    unsigned long long bits = c->nbits;
    // define bytes 56~63 to store message length
    // 56 is first byte of the 8-byte length field, highest bytes, bits bits[56~63]
    tmp[56] = static_cast<unsigned char>((bits >> 56) & 0xFFULL);
    tmp[57] = static_cast<unsigned char>((bits >> 48) & 0xFFULL); // 57 is the second, bits[48~55]
    tmp[58] = static_cast<unsigned char>((bits >> 40) & 0xFFULL); // 58 is the third, bits[40~47]
    tmp[59] = static_cast<unsigned char>((bits >> 32) & 0xFFULL); // 59 is the fourth, bits[32~39]
    tmp[60] = static_cast<unsigned char>((bits >> 24) & 0xFFULL); // 60 is the fifth, bits[24~31]
    tmp[61] = static_cast<unsigned char>((bits >> 16) & 0xFFULL); // 61 is the sixth, bits[16~23]
    tmp[62] = static_cast<unsigned char>((bits >> 8) & 0xFFULL);  // 62 is the seventh, bits[8~15]
    tmp[63] = static_cast<unsigned char>(bits & 0xFFULL);         // 63 is the eighth, lowest byte, bits[0~7]

    // final transform for last block
    AlgoTransform(c, tmp);

    // produce digest (big-endian)
    for (size_t i = 0; i < STATE_WORDS_COUNT; ++i) {
        md[BYTES_PER_WORD * i    ] = static_cast<unsigned char>((c->h[i] >> 24) & 0xFFU); // byte 0, bits[24~31]
        md[BYTES_PER_WORD * i + 1] = static_cast<unsigned char>((c->h[i] >> 16) & 0xFFU); // byte 1, bits[16~23]
        md[BYTES_PER_WORD * i + 2] = static_cast<unsigned char>((c->h[i] >> 8) & 0xFFU);  // byte 2, bits[8~15]
        md[BYTES_PER_WORD * i + 3] = static_cast<unsigned char>(c->h[i] & 0xFFU);         // byte 3, bits[0~7]
    }

    // clear sensitive data
    memset_s(c, sizeof(*c), 0, sizeof(*c)); // ignore clearing failure for return value purposes
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
